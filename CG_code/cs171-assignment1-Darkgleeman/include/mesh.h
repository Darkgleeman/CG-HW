#ifndef _MESH_H_
#define _MESH_H_
#include "defines.h"
#include <vector>

struct Vertex {
  vec3 position;
  vec3 normal;
};

class Mesh {
 public:
  /**
   * construct a mesh from a obj file
   */
  Mesh(const std::string &path);
  /**
   * draw mesh objects
   */
  void draw() const;
  void MeshVAO_set();

 private:
  //  mesh data
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  unsigned int VAO, VBO, EBO;
  void loadDataFromFile(const std::string &path);
};
#endif