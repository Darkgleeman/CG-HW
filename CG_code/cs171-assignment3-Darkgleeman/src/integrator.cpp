#include <integrator.h>
#include <material.h>
#include <light.h>

/**
 * Integrator class
 */
Integrator::Integrator(std::shared_ptr<Camera> camera) : camera(camera) {}

/**
 * PhongLightingIntegrator class
 */
PhongLightingIntegrator::PhongLightingIntegrator(std::shared_ptr<Camera> camera)
    : Integrator(camera) {}

/**
 * render a scene
 * @param[in] the given scene
 */

float grid_pos[][2] = {
    0.06388766,  -0.4472136,  0.31943828,  -0.31943828, -0.31943828,
    -0.31943828, -0.06388766, -0.19166297, 0.19166297,  -0.06388766,
    0.4472136,   0.06388766,  -0.4472136,  -0.06388766, -0.19166297,
    0.06388766,  0.06388766,  0.19166297,  0.31943828,  0.31943828,
    -0.31943828, 0.31943828,  -0.06388766, 0.4472136,
};

void PhongLightingIntegrator::render(Scene &scene) {
  int now = 0;
#//pragma omp parallel for schedule(guided, 2) default(none) shared(now)
  for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx) {
//#pragma omp atomic
    ++now;
    printf("\r%.02f%%", now * 100.0 / camera->getFilm().resolution.x());
    for (int dy = 0; dy < camera->getFilm().resolution.y(); ++dy) {
      vec3 L(0, 0, 0);
      float center_dx = dx+0.5;
      float center_dy = dy+0.5;
      vec3 sum(0, 0, 0);
      for (int i = 0; i < 12; i++) {
        /*float sample_dx = center_dx + grid_pos[i][0];
        float sample_dy = center_dy + grid_pos[i][1];*/
        float sample_dx =
            (center_dx+ grid_pos[i][0]) / camera->getFilm().resolution.x() ;
        ;
        float sample_dy =
            (center_dy+ grid_pos[i][1]) / camera->getFilm().resolution.y() ;

        Ray temp_ray = camera->generateRay(sample_dx, sample_dy);

        Interaction temp_interaction;
        if (scene.intersect(temp_ray, temp_interaction)) {
          sum +=radiance(scene, temp_interaction, temp_ray);
        } 
      }
      L = sum / 12;
      // TODO: generate camera ray & calculate radiance
      camera->setPixel(dx, dy, L);
    }
  }
}

/**
 * according to phong lighting model
 * calculate the radiance with given scene, ray and interaction
 * @param[in] scene the given scene
 * @param[in] interaction the given interaction
 * @param[in] the given ray
 */
vec3 PhongLightingIntegrator::radiance(Scene &scene,
                                       const Interaction &interaction,
                                       const Ray &ray) const {
    if (interaction.type == Interaction::Type::LIGHT)
    {
        return scene.getLight()->getColor();
     }
    else if (interaction.type == Interaction::Type::GEOMETRY) {
      std::shared_ptr<Light> scene_light = scene.getLight();
      vec3 ambient_light = scene.getAmbientLight();
      std::vector<LightSamplePair> sample_lights = scene_light->samples();
      vec3 diffuse(0, 0, 0);
      vec3 specular(0, 0, 0);
      vec3 ambient(0, 0, 0);

      for (int i = 0; i < sample_lights.size(); i++) {
        vec3 interaction_light_dir =
            (sample_lights[i].first - interaction.entryPoint).normalized();
        Ray temp_ray(interaction.entryPoint, interaction_light_dir, 0.0001,
                     999999);
        bool vis = !scene.isShadowed(temp_ray);
        //std::cout << vis << std::endl;
        diffuse +=
            vis *interaction.lightingModel.diffusion.cwiseProduct(sample_lights[i].second) *std::max(0.0,double(interaction.normal.dot(interaction_light_dir)));
        vec3 reflect_dir = 2 * interaction_light_dir.dot(interaction.normal) *interaction.normal -interaction_light_dir;
        Float shininess = interaction.lightingModel.shininess;
        specular +=
            vis *
            interaction.lightingModel.specular.cwiseProduct(
                sample_lights[i].second) *
            std::pow(std::max(0.0f, reflect_dir.dot(-ray.direction)),
                              shininess);
        ambient += interaction.lightingModel.ambient.cwiseProduct(
            vis ? sample_lights[i].second : ambient_light);
      }
      return (diffuse + specular + ambient)/ sample_lights.size();
    }

    else return vec3::Zero();
}

std::shared_ptr<Integrator> makePhongLightingIntegrator(
    std::shared_ptr<Camera> camera) {
  return std::make_shared<PhongLightingIntegrator>(camera);
}