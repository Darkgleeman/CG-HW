#include <object.h>
#include <shader.h>
#include <utils.h>
#include <fstream>
#include <vector>

Object::Object() {}
Object::~Object() {}

/**
 * TODO: initialize VAO, VBO, VEO and set the related variables
 */
void Object::init() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	if (indices.size() != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}
	// vertex pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(0);
}

/**
 * TODO: draw object with VAO and VBO
 * You can choose to implement either one or both of the following functions.
 */

/* Implement this one if you do not use a shader */
void Object::drawArrays() const {
	glBindVertexArray(VAO);
	glDrawArrays(draw_mode.primitive_mode,0,vertices.size());
	glBindVertexArray(0);
}

/* Implement this one if you do use a shader */
void Object::drawArrays(const Shader& shader) const {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
  UNIMPLEMENTED;
}

/**
 * TODO: draw object with VAO, VBO, and VEO
 * You can choose to implement either one or both of the following functions.
 */

/* Implement this one if you do not use a shader */
void Object::drawElements() const {
	glBindVertexArray(VAO);
	glDrawElements(draw_mode.primitive_mode, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

/* Implement this one if you do use a shader */
void Object::drawElements(const Shader& shader) const {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
  UNIMPLEMENTED;
}