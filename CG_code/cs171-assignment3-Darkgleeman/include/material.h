#ifndef CS171_HW3_INCLUDE_MATERIAL_H_
#define CS171_HW3_INCLUDE_MATERIAL_H_
#include <interaction.h>
#include "stb_image.h"
/**
 * Base class of materials
 */
class Material {
 public:
  Material() = default;
  virtual ~Material() = default;
  /**
   * evaluate phong model
   */
  [[nodiscard]] virtual InteractionPhongModel evaluate(
      const Interaction &in) const = 0;
};

/**
 * Constant color-based materials
 */
class ConstantMaterial : public Material {
 protected:
  vec3 diffuse;
  vec3 specular;
  vec3 ambient;
  Float shininess;

 public:
  ConstantMaterial();
  ConstantMaterial(vec3 diff, vec3 spec, vec3 amb, Float sh = 16.0f);
  ConstantMaterial(vec3 rgb, Float shininess);
  [[nodiscard]] InteractionPhongModel evaluate(
      const Interaction &in) const override;
};

// optional
class TextureMaterial : public Material {
  // optional TODO: finish this
    public:
    int bytes_per_pixel = 3;
    TextureMaterial(const char* filename) {
      int components_per_pixel = 0;
      //std::cout << "YES" << std::endl;
      data = stbi_load(filename, &width, &height, &components_per_pixel,
                       components_per_pixel);
      //std::cout << data[0] << std::endl;
      //std::cout << components_per_pixel << std::endl;
      if (!data) {
        std::cerr << "ERROR: Could not load texture image file '" << filename
                  << "'.\n";
        width = height = 0;
      }
      

     }
    ~TextureMaterial() { delete data;}
     [[nodiscard]] InteractionPhongModel evaluate(
         const Interaction &in) const override;


    private:
    unsigned char *data;
     int width, height;
    int bytes_per_scanline;
};

std::shared_ptr<ConstantMaterial> makeConstantMaterial(const vec3 &rgb,
                                                       Float shininess);
std::shared_ptr<TextureMaterial> makeTextureMaterial(const char *filename);
#endif  // CS171_HW3_INCLUDE_MATERIAL_H_
