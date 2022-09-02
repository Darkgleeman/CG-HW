#include <light.h>

/**
 * Light class
 */
Light::Light(const vec3 &position, const vec3 &color)
    : position(position), color(color) {}

vec3 Light::getPosition() const { return position; }

vec3 Light::getColor() const { return color; }

/**
 * AreaLight class
 */
AreaLight::AreaLight(const vec3 &position, const vec3 &color, const vec2 &size)
    : Light(position, color),
      areaSize(size),
      geoms(makeParallelogram(position - vec3(size[0], 0, size[1]) / 2,
                              vec3(size[0], 0, 0), vec3(0, 0, size[1]),
                              vec3(0, -1, 0), nullptr)) {}

std::vector<LightSamplePair> AreaLight::samples() const {
  std::vector<LightSamplePair> ret;

  // TODO: generate rectangle light samples
  float m = 5;
  float n = 5;
  vec3 left_corner = position - vec3(areaSize[0], 0, areaSize[1]) / 2;
  vec3 sample_color = color ;
  for (int i = 0;i<=m;i++) 
  { 
    for (int j=0;j<=n;j++) 
    {
      vec3 sample_pos = left_corner + vec3(areaSize[0], 0, 0) * i / m + vec3(0, 0, areaSize[1]) * j / n;
      ret.push_back(std::pair(sample_pos, sample_color));
    }
  }
  return ret;
}

bool AreaLight::intersect(Interaction &interaction,
                                const Ray &ray) const {
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