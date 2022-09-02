#include <mesh.h>
#include <utils.h>
#include <fstream>
#include <vector>

Mesh::Mesh(const std::string &path) { loadDataFromFile(getPath(path)); }

/**
 * TODO: implement load mesh data from file here
 * you need to open file with [path]
 *
 * File format
 * First line contains 3 integers: vertex count, normal count, face count
 *
 * For the next [vertex count] lines, each line contains 3 floats, which
 * represents the position of a vertex
 *
 * For the next [normal count] lines, each line contains 3 floats, which
 * represents the normal of a vertex
 *
 * For the next [face count] lines, each line contains 6 integers
 * [v0, n0, v1, n1, v2, n2], which represents the triangle face
 * v0, n0 means the vertex index and normal index of the first vertex
 * v1, n1 means the vertex index and normal index of the second vertex
 * v2, n2 means the vertex index and normal index of the third vertex
 */
void Mesh::loadDataFromFile(const std::string &path) {
	std::ifstream fin;
	fin.open(path);
	if (fin.is_open())
	{
		int vertex_count, normal_count, face_count;
		fin >> vertex_count >> normal_count >> face_count;
		//printf("%d\n", vertex_count);
		//printf("%d\n", normal_count);
		//printf("%d\n", face_count);
		for (int i = 0; i < vertex_count; i++)
		{	
			struct Vertex temp;
			fin >> temp.position.x >> temp.position.y >> temp.position.z;
			//printf("i:%i,x:%f,y:%f,z:%f\n", i, temp.position.x, temp.position.y, temp.position.z);
			vertices.push_back(temp);
		}
		GLfloat normal_tepo[3800][3] = { 0 };
		for (int i = 0; i < normal_count; i++)
		{	
			fin >> normal_tepo[i][0] >> normal_tepo[i][1] >> normal_tepo[i][2];
			//printf("i:%d,normal_tepo[i][0]:%f,normal_tepo[i][1]:%f,normal_tepo[i][2]:%f\n", i, normal_tepo[i][0], normal_tepo[i][1], normal_tepo[i][2]);
		}
		for (int i = 0; i < face_count; i++)
		{
			int v0, n0, v1, n1, v2, n2;
			fin >> v0 >> n0 >> v1 >> n1 >> v2 >> n2;
			//printf("v0:%d,n0:%d,v1:%d,n1:%d,v2:%d,v2:%d\n", v0, n0, v1, n1, v2, n2);
			vertices[v0].normal.x = normal_tepo[n0][0];
			vertices[v0].normal.y = normal_tepo[n0][1];
			vertices[v0].normal.z = normal_tepo[n0][2];

			vertices[v1].normal.x = normal_tepo[n1][0];
			vertices[v1].normal.y = normal_tepo[n1][1];
			vertices[v1].normal.z = normal_tepo[n1][2];

			vertices[v2].normal.x = normal_tepo[n2][0];
			vertices[v2].normal.y = normal_tepo[n2][1];
			vertices[v2].normal.z = normal_tepo[n2][2];
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(v2);
		}
	}
}

void Mesh::MeshVAO_set()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),indices.data(), GL_STATIC_DRAW);

	// vertex pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(0);
}

/**
 * TODO: implement your draw code here
 */
void Mesh::draw() const {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}