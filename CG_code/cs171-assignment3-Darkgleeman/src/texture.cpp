#include <texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void loadTextureFromFile(std::vector<std::uint8_t> &data,
                         const std::string &path) {
  int w, h, ch;
  stbi_uc *img = stbi_load(path.c_str(), &w, &h, &ch, 0);
  data.assign(img, img + w * h * ch);
  stbi_image_free(img);
}