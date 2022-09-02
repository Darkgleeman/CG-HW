#include <light.h>
#include <utils.h>

/**
 * Light class
 */
Light::Light(const vec3 &position, const vec3 &radiance)
    : position(position), radiance(radiance) {}

vec3 Light::getPosition() const { return position; }

vec3 Light::getRadiance() const { return radiance; }

/**
 * AreaLight class
 */
AreaLight::AreaLight(const vec3 &position, const vec3 &color, const vec2 &size)
    : Light(position, color),
      areaSize(size),
      geoms(makeParallelogram(position - vec3(size[0], 0, size[1]) / 2,
                              vec3(size[0], 0, 0), vec3(0, 0, size[1]),
                              vec3(0, -1, 0), nullptr)) {}

vec3 AreaLight::emission(vec3 pos, vec3 dir) {
  vec3 emission_radiance = std::max(dir.dot(vec3(0, -1, 0)), 0.0f) * getRadiance();
  return emission_radiance;
}

//Here pdf means the pdf of the area light ,not the pdf of the solid angle
Float AreaLight::pdf(const Interaction &ref_it, vec3 pos) {
  if(areaSize[0] * areaSize[1] !=0)
    return 1 / (areaSize[0] * areaSize[1]);
  return 0;
}

vec3 AreaLight::sample(Interaction &refIt, Float *pdf) {
    float eta1 = unif(0.0001, 0.9999, 1)[0];
    float eta2 = unif(0.0001, 0.9999, 1)[0];
    vec3 u = vec3(areaSize[0], 0, 0);
    vec3 v = vec3(0, 0, areaSize[1]);
    vec3 pos = position- vec3(areaSize[0], 0, areaSize[1]) / 2 + eta1 * u + eta2 * v;
    refIt.wo = (pos - refIt.entryPoint).normalized();
    *pdf = this->pdf(refIt, pos);
    return pos;

}

bool AreaLight::intersect(Interaction &interaction, const Ray &ray) const {
  bool intersection = false;
  for (auto &i : geoms)
    intersection = intersection || i->intersect(interaction, ray);
  interaction.type = Interaction::Type::LIGHT;
  return intersection;
}

std::shared_ptr<Light> makeAreaLight(const vec3 &position, const vec3 &color,
                                     const vec2 &size) {
  return std::make_shared<AreaLight>(position, color, size);
}