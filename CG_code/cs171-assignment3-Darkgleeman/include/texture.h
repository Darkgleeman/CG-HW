#ifndef CS171_HW3_INCLUDE_TEXTURE_H_
#define CS171_HW3_INCLUDE_TEXTURE_H_
#include <vector>
#include <cstdint>
#include <string>

// optional
// you can change the structure if you want
class Texture {
 private:
  std::vector<std::uint8_t> data;
};

void loadTextureFromFile(std::vector<std::uint8_t> &data,
                         const std::string &path);
#endif