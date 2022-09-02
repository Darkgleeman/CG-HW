#include <integrator.h>
#include <brdf.h>
#include <light.h>

/**
 * Integrator class
 */
Integrator::Integrator(std::shared_ptr<Camera> camera) : camera(camera) {}

/**
 * PhongLightingIntegrator class
 */
PathIntegrator::PathIntegrator(std::shared_ptr<Camera> camera)
    : Integrator(camera) {}

/**
 * render a scene
 * @param[in] the given scene
 */



void PathIntegrator::render(Scene &scene) {
  int now = 0;
  float sample_row = 15;
  float delta = 1.0 / sample_row;
#pragma omp parallel for schedule(dynamic, 8) default(none) shared(now)
  for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx) {
#pragma omp atomic
//for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx) {
    ++now;
    printf("\r%.02f%%", now * 100.0 / camera->getFilm().resolution.x());
    for (int dy = 0; dy < camera->getFilm().resolution.y(); ++dy) {
      vec3 L(0, 0, 0);
      for (float i = 0; i <= 1; i = i + delta)
      {
          for (float j = 0; j <= 1; j = j + delta)
          {
              float sample_dx =(dx + i);
              float sample_dy = (dy + j);
              Ray temp_ray = camera->generateRay(sample_dx, sample_dy);
              L += radiance(scene, temp_ray);
          }
      }
      L = L / ((sample_row + 1) * (sample_row + 1));
      camera->setPixel(dx, dy, L);
    }
  }
}

/**
 * calculate the radiance with given scene, ray and interaction
 * @param[in] scene the given scene
 * @param[in] interaction the given interaction
 * @param[in] the given ray
 */
vec3 PathIntegrator::radiance(Scene &scene, const Ray &ray) const {
    int depth = 5;
    vec3 L(0, 0, 0);
    vec3 beta(1.0, 1.0, 1.0);
    Ray shoot_ray = ray;
    bool prev_spec = false;
    for (int i = 0; i < depth; i++)
    {
        Interaction interaction;
        scene.intersect(shoot_ray, interaction);
        if (interaction.type == Interaction::Type::LIGHT)
        {   
            if (i == 0 ||(i == 1 && prev_spec)) //if (i == 0 ||prev_spec)
            {   
                L = scene.getLight()->getRadiance();
                return L;
            }
            else break;
        }
        if (interaction.type == Interaction::Type::NONE) break;
        vec3 dir_samplelight(0, 0, 0);
        vec3 dir_brdf(0, 0, 0);
        vec3 dir_light(0, 0, 0);
        float A_pdf = 0;
        float B_pdf = 0;
        float wi_pdf = 0;
        float weight_light = 1;
        float weight_brdf = 1;
        //sample light
        vec3 light_pos = scene.getLight()->sample(interaction, &A_pdf);
        std::shared_ptr<BRDF> brdf = interaction.brdf;
        vec3 f = brdf->eval(interaction);
        prev_spec = brdf->isDelta();
        Ray temp_ray = Ray(interaction.entryPoint, interaction.wo);
        if (!scene.isShadowed(temp_ray)&& !brdf->isDelta()) //Attention:here we use the temp_ray,which means the interaction to the light as the ray to be judged, not the shoot_ray which is first hit by the camera to the interaction
        {
            vec3 L_i = scene.getLight()->emission(light_pos, -interaction.wo);
            float r = (light_pos - interaction.entryPoint).norm();
            dir_samplelight = f.cwiseProduct(L_i) * abs(interaction.normal.dot(interaction.wo)) / (r * r) / A_pdf;
            wi_pdf = 1 / (r * r * A_pdf);
        }
        B_pdf = brdf->sample(interaction);//always >0,so don't need to judge
        weight_light = wi_pdf * wi_pdf / (wi_pdf * wi_pdf + B_pdf * B_pdf);

        //sample brdf
        temp_ray = Ray(interaction.entryPoint, interaction.wo);
        if (!scene.isShadowed(temp_ray)&& !brdf->isDelta()) //Attention:here we use the temp_ray,which means the interaction to the light as the ray to be judged, not the shoot_ray which is first hit by the camera to the interaction
        {
            vec3 L_i = scene.getLight()->emission(light_pos, -interaction.wo);
            dir_brdf = f.cwiseProduct(L_i) * abs(interaction.normal.dot(interaction.wo)) / B_pdf;
        }
        weight_brdf = 1 - weight_light;
        dir_light = weight_light * dir_samplelight + weight_brdf * dir_brdf;
        //remember,here interaction.wo has been updated from shooting to the light to shooting another surface
        L += beta.cwiseProduct(dir_light);
        float cos_theta_surface = abs(interaction.normal.dot(interaction.wo));
        beta = beta.cwiseProduct(f * cos_theta_surface / B_pdf); 
        shoot_ray = Ray(interaction.entryPoint + 0.001 * interaction.wo, interaction.wo);
 
    }
    //std::cout << "L:" << L[0] << " " << L[1] << " " << L[2] << std::endl;
    return L;

}


std::shared_ptr<Integrator> makePathIntegrator(std::shared_ptr<Camera> camera) {
  return std::make_shared<PathIntegrator>(camera);
}
