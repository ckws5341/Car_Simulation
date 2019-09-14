#include "treeModel0.h"
#include "GL/glew.h"
#include "glm/glm.hpp"
#include <iostream>
#include "../BaseCodes/Mesh.h"
#include "../BaseCodes/BasicShapeObjs.h"
#include <string>
#include <stack>
#include <math.h>

///////////////////////////////////////////////
// Tree

static GLuint g_tree_vao[6];
static GLuint g_tree_vbo_pos[6];
static GLuint g_tree_vbo_color[6];
static GLuint g_tree_vbo_normal[6];
static GLuint g_tree_index_buf[6];
static Mesh g_tree_mesh[6];
static Mesh g_flower_mesh;
static Mesh g_flowerseed_mesh;
Mesh trunk;
Mesh leaf;
Mesh g_leaf_mesh;
float age = 1.f;
void InitTreeModel()
{
	// Element 1, ³ª¹«±âµÕ
	trunk = glm::scale(glm::vec3(0.013f, 0.50f, 0.013f))
		* glm::translate(glm::vec3(0.f, 0.5f, 0.f))
		* g_cylinder_mesh;
	trunk.SetColor(0.98f, 0.5f, 0.3f, 1.f);

	// Element 2, ³ª¹µÀÙ
	leaf = glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0.f, 1.f, 0.f))
		* glm::rotate(-glm::pi<float>() / 6.f, glm::vec3(1.f, 0.f, 0.f))
		* glm::scale(glm::vec3(0.015f, 0.001f, 0.045f))
		* glm::translate(glm::vec3(1.f, 0.2f, 1.f))
		* g_cylinder_mesh;
	
	glm::mat4 M(1.0);
	M *= glm::scale(glm::vec3(2.f, 2.f, 2.f));
	for (int a = 0; a < 3; a++)
	{
		M *= glm::translate(glm::vec3(0.f, 0.04f, 0.f));
		for (int i = 0; i < 4; i++)
		{
			M *= glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0.f, 1.f, 0.f)) 
				*glm::translate(glm::vec3(0.f, 0.01f, 0.f));
			g_leaf_mesh += M * leaf;
		}
	}
	g_leaf_mesh.SetColor(0.f, 1.f, 0.f, 1.f);


	//Element 3, ²É(ÀÙ6Àå + ²É¼ú)
	g_flowerseed_mesh = glm::scale(glm::vec3(0.02f, 0.005f, 0.02f))
		* glm::translate(glm::vec3(0.f, 0.05f, 0.f))
		* g_hemisphere_mesh;
	g_flowerseed_mesh.SetColor(1.0f, 0.85f, 0.f);
	glm::mat4 F(1.0);

	g_flower_mesh = leaf;
	for (int i = 0; i < 6; i++)
	{
		F *= glm::rotate(glm::pi<float>() / 3.f, glm::vec3(0.f, 1.f, 0.f));
		g_flower_mesh += F * leaf;
	}
	g_flower_mesh.SetColor(1.f, 0.0f, 0.f);
	g_flower_mesh += g_flowerseed_mesh;
	g_flower_mesh = glm::rotate(-glm::pi<float>() / 2.f, glm::vec3(1.f, 0.f, 0.f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f))* g_flower_mesh;

	
	for (int tree_age = 0; tree_age < 6; tree_age++)
	{
		
		// Tree, ³ª¹«±âµÕ+³ª¹µÀÓ+³ª¹«±âµÕ
		glm::mat4 T(1.0);
		std::stack<glm::mat4> stack_T;
		static float g_delta_degree = glm::pi<float>() / 6.f;
		std::string add_struct_X = "FL----[&X]+++[^X]---[^^&X]+++---&^---Fh+++X---";
		std::string add_struct_F = "FF";
		std::string LSystem_struct[6] = {"X","X","X","X","X","X"};
		for (unsigned int i = 0; i < tree_age; i++)
		{
			std::string Res_struct;
			for (unsigned int j = 0; j < LSystem_struct[i].size(); j++)
			{
				if (LSystem_struct[i][j] == 'X')
					Res_struct = Res_struct + std::to_string(i + 1) + add_struct_X + std::to_string(i);
				else if (LSystem_struct[i][j] == 'F')
					Res_struct = Res_struct + add_struct_F;
				else
					Res_struct = Res_struct + LSystem_struct[i][j];

			}
			LSystem_struct[i] = Res_struct;
			if(i<5)
				LSystem_struct[i+1] = LSystem_struct[i];
			std::cout << i <<"        " << LSystem_struct[i]<< std::endl << std::endl;
		}
		int age = 1;
		int trunk_num = 0;
		for (unsigned int i = 0; i < LSystem_struct[tree_age].size(); i++)
		{
			glm::vec3 smaller_becauseof_age((1.0 / tree_age)*(tree_age - (age-1)), (1.0 / tree_age)*(tree_age - (age - 1)), (1.0 / tree_age)*(tree_age - (age - 1)));
			switch (LSystem_struct[tree_age][i])
			{
			case 'F':
				g_tree_mesh[tree_age] += T * glm::scale(smaller_becauseof_age) * trunk;
				T = T * glm::translate(glm::vec3(0.f, 0.07f, 0.f));
				break;
			case 'L':
				if(tree_age != 1)
					g_tree_mesh[tree_age] += T * glm::scale(smaller_becauseof_age) * g_leaf_mesh;
				break;
			case 'h':
				g_tree_mesh[tree_age] += T * glm::translate(glm::vec3(0.f, 0.f, -0.02f+(0.005f*age))) * glm::scale(smaller_becauseof_age) * g_flower_mesh;
				break;
			case '+':
				glRotated(g_delta_degree, 0, 1, 0);
				T = T * glm::rotate(g_delta_degree, glm::vec3(0, 1, 0));
				break;
			case '-':
				glRotated(-g_delta_degree, 0, 1, 0);
				T = T * glm::rotate(-g_delta_degree, glm::vec3(0, 1, 0));
				break;
			case '^':
				glRotated(g_delta_degree, 1, 0, 0);
				T = T * glm::rotate(g_delta_degree, glm::vec3(1, 0, 0));
				break;
			case '&':
				glRotated(-g_delta_degree, 1, 0, 0);
				T = T * glm::rotate(-g_delta_degree, glm::vec3(1, 0, 0));
				break;
			case '|':
				glRotated(g_delta_degree, 0, 0, 1);
				T = T * glm::rotate(g_delta_degree, glm::vec3(0, 0, 1));
				break;
			case '/':
				glRotated(-g_delta_degree, 0, 0, 1);
				T = T * glm::rotate(-g_delta_degree, glm::vec3(0, 0, 1));
				break;
			case '[':
				stack_T.push(T);
				break;
			case ']':
				T = stack_T.top();
				stack_T.pop();
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
				age = LSystem_struct[tree_age][i] - '0';
			}
		}
		// Vertex Array Object
		glGenVertexArrays(1, &g_tree_vao[tree_age]);
		glBindVertexArray(g_tree_vao[tree_age]);


		// Position VBO
		glGenBuffers(1, &g_tree_vbo_pos[tree_age]);
		glBindBuffer(GL_ARRAY_BUFFER, g_tree_vbo_pos[tree_age]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * g_tree_mesh[tree_age].num_points(), g_tree_mesh[tree_age].points(), GL_STATIC_DRAW);

		// Color VBO
		glGenBuffers(1, &g_tree_vbo_color[tree_age]);
		glBindBuffer(GL_ARRAY_BUFFER, g_tree_vbo_color[tree_age]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * g_tree_mesh[tree_age].num_points(), g_tree_mesh[tree_age].colors(), GL_STATIC_DRAW);

		// Normal VBO
		glGenBuffers(1, &g_tree_vbo_normal[tree_age]);
		glBindBuffer(GL_ARRAY_BUFFER, g_tree_vbo_normal[tree_age]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * g_tree_mesh[tree_age].num_points(), g_tree_mesh[tree_age].normals(), GL_STATIC_DRAW);


		// Index Buffer Object
		glGenBuffers(1, &g_tree_index_buf[tree_age]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_tree_index_buf[tree_age]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * g_tree_mesh[tree_age].num_triangles(), g_tree_mesh[tree_age].triangle_ids(), GL_STATIC_DRAW);


		// Vertex Attribute
		glBindBuffer(GL_ARRAY_BUFFER, g_tree_vbo_pos[tree_age]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, g_tree_vbo_color[tree_age]);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, g_tree_vbo_normal[tree_age]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
}


void DrawTreeModel(int tree_age)
{
		glBindVertexArray(g_tree_vao[tree_age]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_tree_index_buf[tree_age]);
		glDrawElements(GL_TRIANGLES, g_tree_mesh[tree_age].num_triangles() * 3, GL_UNSIGNED_INT, NULL);
}


	void DeleteTreeModel()
	{
		for (int tree_age = 0; tree_age < 6; tree_age++)
		{
			glDeleteBuffers(1, &g_tree_vbo_pos[tree_age]);
			glDeleteBuffers(1, &g_tree_vbo_color[tree_age]);
			glDeleteBuffers(1, &g_tree_vbo_normal[tree_age]);
			glDeleteBuffers(1, &g_tree_index_buf[tree_age]);
			glDeleteVertexArrays(1, &g_tree_vao[tree_age]);
		}
	}



