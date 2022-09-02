#include <material.h>
#include <interaction.h>

/**
 * ColorMaterial class
 */

ConstantMaterial::ConstantMaterial()
    : diffuse(vec3(1.0, 1.0, 1.0)), specular(vec3(0, 0, 0)), shininess(0.0) {}

ConstantMaterial::ConstantMaterial(vec3 diff, vec3 spec, vec3 amb, Float sh)
    : diffuse(diff), specular(spec), ambient(amb), shininess(sh) {}

ConstantMaterial::ConstantMaterial(vec3 rgb, Float sh = 16.0f)
    : diffuse(rgb), specular(rgb), ambient(rgb / 10), shininess(sh) {}

InteractionPhongModel ConstantMaterial::evaluate(const Interaction &in) const {
  InteractionPhongModel m;
  m.diffusion = diffuse;
  m.specular = specular;
  m.ambient = ambient;
  m.shininess = shininess;

  return m;
}

std::shared_ptr<ConstantMaterial> makeConstantMaterial(const vec3 &rgb,
                                                       Float shininess) {
  return std::make_shared<ConstantMaterial>(rgb, shininess);
}

InteractionPhongModel TextureMaterial::evaluate(const Interaction &in) const {
  double u = in.uv[0];
  double v = in.uv[1];
  u = std::clamp(u, 0.0, 1.0);
  v = 1.0 - std::clamp(v, 0.0, 1.0); 
  int i = u * width;
  int j = v * height;
  if (i >= width) i = width - 1;
  if (j >= height) j = height - 1;
  float color_scale = 1.0 / 255.0;
  //std::cout << data << std::endl;
  auto pixel = data + j * bytes_per_pixel * width + i * bytes_per_pixel;
  //auto pixel = data;
  vec3 color = vec3(color_scale * pixel[0], color_scale * pixel[1],color_scale * pixel[2]);
  InteractionPhongModel m;
  m.diffusion = color;
  m.specular = color;
  m.ambient = vec3(0,0,0);
  m.shininess = 16.0;

  return m;
}

std::shared_ptr<TextureMaterial> makeTextureMaterial(const char *filename) {
  return std::make_shared<TextureMaterial>(filename);
}
