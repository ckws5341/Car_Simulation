#include "DropFlowerModel.h"
#include "GL/glew.h"
#include "glm/glm.hpp"
#include <iostream>
#include "../BaseCodes/Mesh.h"
#include "../BaseCodes/BasicShapeObjs.h"

extern int g_shader_id;

GLuint g_dropflower_vao;
GLuint g_dropflower_vbo_pos;
GLuint g_dropflower_vbo_color;
GLuint g_dropflower_vbo_normal;
GLuint g_dropflower_index_buf;

Mesh g_dropflower_mesh;
Mesh flowerleaf;
Mesh g_flower_mesh;
Mesh g_flowerseed_mesh;
void InitDropFlowerModel()
{
	// Element 1. 采蕾
	flowerleaf = glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0.f, 1.f, 0.f))
		* glm::rotate(-glm::pi<float>() / 6.f, glm::vec3(1.f, 0.f, 0.f))
		* glm::scale(glm::vec3(0.015f, 0.001f, 0.045f))
		* glm::translate(glm::vec3(1.f, 0.2f, 1.f))
		* g_cylinder_mesh;
	

	//Element 2. 采贱
	g_flowerseed_mesh = glm::scale(glm::vec3(0.02f, 0.005f, 0.02f))
		* glm::translate(glm::vec3(0.f, 0.05f, 0.f))
		* g_hemisphere_mesh;
	g_flowerseed_mesh.SetColor(1.0f, 0.85f, 0.f,1.f);
	glm::mat4 F(1.0);

	//Element 3. 采(采蕾6厘 + 采贱)
	g_flower_mesh = flowerleaf;
	for (int i = 0; i < 6; i++)
	{
		F *= glm::rotate(glm::pi<float>() / 3.f, glm::vec3(0.f, 1.f, 0.f));
		g_flower_mesh += F * flowerleaf;
	}
	g_flower_mesh.SetColor(0.9f, 0.1f, 0.f, 1.f);
	g_flower_mesh += g_flowerseed_mesh;
	g_dropflower_mesh = glm::rotate(-glm::pi<float>() / 2.f, glm::vec3(1.f, 0.f, 0.f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f))* g_flower_mesh;

	// Vertex Array Object
	glGenVertexArrays(1, &g_dropflower_vao);
	glBindVertexArray(g_dropflower_vao);

	
	// Position VBO
	glGenBuffers(1, &g_dropflower_vbo_pos);
	glBindBuffer(GL_ARRAY_BUFFER, g_dropflower_vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * g_dropflower_mesh.num_points(), g_dropflower_mesh.points(), GL_STATIC_DRAW);

	// Color VBO
	glGenBuffers(1, &g_dropflower_vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, g_dropflower_vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * g_dropflower_mesh.num_points(), g_dropflower_mesh.colors(), GL_STATIC_DRAW);

	// Normal VBO
	glGenBuffers(1, &g_dropflower_vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, g_dropflower_vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * g_dropflower_mesh.num_points(), g_dropflower_mesh.normals(), GL_STATIC_DRAW);


	// Index Buffer Object
	glGenBuffers(1, &g_dropflower_index_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_dropflower_index_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * g_dropflower_mesh.num_triangles(), g_dropflower_mesh.triangle_ids(), GL_STATIC_DRAW);


	// Vertex Attribute
	glBindBuffer(GL_ARRAY_BUFFER, g_dropflower_vbo_pos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, g_dropflower_vbo_color);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, g_dropflower_vbo_normal);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	

}


void DrawDropFlowerModel()
{
	glBindVertexArray(g_dropflower_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_dropflower_index_buf);
	glDrawElements(GL_TRIANGLES, g_dropflower_mesh.num_triangles() * 3, GL_UNSIGNED_INT, NULL);
}


void DeleteDropFlowerModel()
{
	glDeleteBuffers(1, &g_dropflower_vbo_pos);
	glDeleteBuffers(1, &g_dropflower_vbo_color);
	glDeleteBuffers(1, &g_dropflower_vbo_normal);
	glDeleteBuffers(1, &g_dropflower_index_buf);
	glDeleteVertexArrays(1, &g_dropflower_vao);

}
