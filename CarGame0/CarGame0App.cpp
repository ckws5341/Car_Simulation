


#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "CarGame0App.h"
#include "../BaseCodes/Camera.h"
#include "../BaseCodes/GroundObj2.h"
#include "../BaseCodes/InitShader.h"
#include "../BaseCodes/BasicShapeObjs.h"
#include "CarModel.h"
#include "TreeModel0.h"
#include <iostream>
#include <cstring>
#include <math.h>
#include "DropFlowerModel.h"

// Window and User Interface
static bool g_left_button_pushed;
static bool g_right_button_pushed;
static int g_last_mouse_x;
static int g_last_mouse_y;

extern GLuint g_window_w;
extern GLuint g_window_h;

//////////////////////////////////////////////////////////////////////
// Camera 
//////////////////////////////////////////////////////////////////////
static Camera g_camera;
static int g_camera_mode = 0;
static float smooth_timer = 0.f;
glm::vec3 g_camera_position;
glm::vec3 g_ex_camera_position;
glm::vec3 g_driver_eye_position;
//////////////////////////////////////////////////////////////////////
//// Define Shader Programs
//////////////////////////////////////////////////////////////////////
GLuint s_program_id;



//////////////////////////////////////////////////////////////////////
//// Animation Parameters
//////////////////////////////////////////////////////////////////////
float g_elaped_time_s = 0.f;	// 


//////////////////////////////////////////////////////////////////////
//// Car Position, Rotation, Velocity
//// �ڵ��� ���� ������.
//////////////////////////////////////////////////////////////////////
glm::vec3 g_car_position(0.f, 0.f, 0.f); //��ġ
float g_car_speed = 0;			          // �ӵ� (�ʴ� �̵� �Ÿ�)
float g_car_rotation_y = 0;		          // ���� ���� (y�� ȸ��)
float g_car_angular_speed = 0;	          // ȸ�� �ӵ� (���ӵ� - �ʴ� ȸ�� ��)

bool n_mode = false; // ��&�� ��ȯ ����

std::string s_Type;
std::string s_Intensity;
std::string s_Position;
std::string s = "lights[";
const char * c;

int tree_arr[5][5] = { {2, 4, 5, 1, 3}, { 3, 5, 1, 2, 4}, {4, 2, 3, 1, 5} ,{1, 4, 3, 5, 2}, {5, 1, 4, 2, 3} };// ���� �迭

float ex_falling_Y;
/**
InitOpenGL: ���α׷� �ʱ� �� ������ ���� ���� �� �� ȣ��Ǵ� �Լ�. (main �Լ� ����)
OpenGL�� ������ �ʱ� ���� ���α׷��� �ʿ��� �ٸ� �ʱ� ���� �����Ѵ�.
�������, VAO�� VBO�� ���⼭ ������ �� �ִ�.
*/
void InitOpenGL()
{
	//////////////////////////////////////////////////////////////////////////////////////
	//// 3. Shader Programs ���
	////    Ref: https://www.khronos.org/opengl/wiki/Shader_Compilation
	//////////////////////////////////////////////////////////////////////////////////////
	s_program_id = CreateFromFiles("../Shaders/v_shader.glsl", "../Shaders/f_shader.glsl");
	glUseProgram(s_program_id);



	////////////////////////////////////////////////////////////////////////////////////
	//// 4. OpenGL ����
	//////////////////////////////////////////////////////////////////////////////////////
	glViewport(0, 0, (GLsizei)g_window_w, (GLsizei)g_window_h);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// Initial State of Camera
	// ī�޶� �ʱ� ��ġ �����Ѵ�.
	g_camera.lookAt(glm::vec3(3.f, 2.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	////////////////////////////////////////////////////////////////////////////////////
	//// 5. VAO, VBO ����
	////    Ref: https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object
	///////////////////////////////////////////////////////////////////////////////////
	
	// basic meshes
	InitBasicShapeObjs();

	// Tree
	InitTreeModel();
	InitDropFlowerModel();
	// Car
	InitCarModel();

	// �ٴ� ���� VAO ����
	InitGround2();
}







/**
ClearOpenGLResource: ���α׷��� ������ �޸� ������ ���� �� �� ȣ��Ǵ� �Լ�. (main �Լ� ����)
���α׷����� ����� �޸𸮸� ���⿡�� ������ �� �ִ�.
�������, VAO�� VBO�� ���⼭ ���� �� �ִ�.
*/
void ClearOpenGLResource()
{
	// Delete (VAO, VBO)
	DeleteBasicShapeObjs();
	DeleteTreeModel();
	DeleteCarModel();
	DeleteGround2();
}





/**
Display: ������ ȭ���� ������Ʈ �� �ʿ䰡 ���� �� ȣ��Ǵ� callback �Լ�.

������ �� ���� ����� ������ �ϴ� �ڵ�� �� �Լ� ���� �����ؾ��Ѵ�.
�����찡 ó�� ���� ��, ������ ũ�Ⱑ �ٲ� ��, �ٸ� �����쿡 ���� ȭ���� �Ϻ�
�Ǵ� ��ü�� �������ٰ� �ٽ� ��Ÿ�� �� �� �ý����� �ش� ������ ���� �׸��� ����
������Ʈ�� �ʿ��ϴٰ� �Ǵ��ϴ� ��� �ڵ����� ȣ��ȴ�.
���� ȣ���� �ʿ��� ��쿡�� glutPostRedisplay() �Լ��� ȣ���ϸ�ȴ�.

�� �Լ��� �ҽÿ� ����ϰ� ȣ��ȴٴ� ���� ����ϰ�, ������ ���� ��ȭ�� ������
1ȸ�� �ڵ�� �������� �� �Լ� �ۿ� �����ؾ��Ѵ�. Ư�� �޸� �Ҵ�, VAO, VBO ����
���� �ϵ���� ������ �õ��ϴ� �ڵ�� Ư���� ������ ���ٸ� ���� �� �Լ��� ���Խ�Ű��
�ȵȴ�. ���� ���, �޽� ���� �����ϰ� VAO, VBO�� �����ϴ� �κ��� ���� 1ȸ��
�����ϸ�ǹǷ� main() �Լ� �� �ܺο� �����ؾ��Ѵ�. ���ǵ� �޽� ���� ������ ���ۿ�
�׸����� �����ϴ� �ڵ常 �� �Լ��� �����ϸ� �ȴ�.

����, �� �Լ� ������ ���� �޸� �Ҵ��� �ؾ��ϴ� ��찡 �ִٸ� �ش� �޸𸮴� �ݵ��
�� �Լ��� ������ ���� ���� �ؾ��Ѵ�.

ref: https://www.opengl.org/resources/libraries/glut/spec3/node46.html#SECTION00081000000000000000
*/
void Display()
{
	// ��ü ȭ���� �����.
	// glClear�� Display �Լ� ���� �� �κп��� �� ���� ȣ��Ǿ���Ѵ�.
	if(n_mode)
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	else
		glClearColor(1.f, 1.f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	// Vertex shader �� matrix �������� location�� �޾ƿ´�.
	int m_proj_loc = glGetUniformLocation(s_program_id, "proj_matrix");
	int m_view_loc = glGetUniformLocation(s_program_id, "view_matrix");
	int m_model_loc = glGetUniformLocation(s_program_id, "model_matrix");

	glm::mat4 view_matrix = g_camera.GetGLViewMatrix();
	glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

	int num_of_lights_loc = glGetUniformLocation(s_program_id, "num_of_lights");
	glUniform1i(num_of_lights_loc, 100);


	if ( g_camera_mode == 1 )// Driver view
	{
		// Projection Transform Matrix ����.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(90.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		// Camera Transform Matrix ����.
		
		g_camera_position = { 0.f, 0.6f, -0.05f };
		g_driver_eye_position = glm::rotateY(g_camera_position + glm::vec3(0.f, 0.f, 3.f), g_car_rotation_y) + g_car_position;
		g_camera_position = glm::rotateY(g_camera_position, g_car_rotation_y) + g_car_position;
		view_matrix = glm::lookAt(g_camera_position, g_driver_eye_position, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else if (g_camera_mode == 2)// Birdeye view
	{
		// Projection Transform Matrix ����.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix ����.
		g_camera_position = { 0.f, 3.5f, -4.f };
		g_camera_position = glm::rotateY(g_camera_position, g_car_rotation_y)+ g_car_position;

		view_matrix = glm::lookAt(g_camera_position, g_car_position, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else if (g_camera_mode == 3)// Smooth Driver view
	{
		// Projection Transform Matrix ����.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.f) + glm::radians(45.f * smooth_timer / 60) , (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix ����.
		g_camera_position = { 0.f, 0.6f, -0.05f };
		g_driver_eye_position = glm::rotateY(g_camera_position + glm::vec3(0.f, 0.f, 3.f), g_car_rotation_y) + g_car_position;
		g_camera_position = glm::rotateY(g_camera_position, g_car_rotation_y) + g_car_position;
		g_camera_position = g_ex_camera_position + (g_camera_position - g_ex_camera_position)*smooth_timer / 60;
		view_matrix = glm::lookAt(g_camera_position, g_car_position - (g_car_position - g_driver_eye_position)*smooth_timer / 60, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

		if (smooth_timer >= 60.f)
		{
			smooth_timer = 0.f;
			g_camera_mode = 1;
		}
		else
			smooth_timer += 0.75f;
	}
	else if (g_camera_mode == 4)// Smooth Birdeye view
	{
		// Projection Transform Matrix ����.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(90.f) - glm::radians(45.f * smooth_timer / 60), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix ����.
		
		g_camera_position = { 0.f, 3.5f, -4.f };
		g_camera_position = glm::rotateY(g_camera_position, g_car_rotation_y) + g_car_position;
		g_camera_position = g_ex_camera_position + (g_camera_position - g_ex_camera_position)*smooth_timer / 60;
		view_matrix = glm::lookAt(g_camera_position, g_driver_eye_position - (g_driver_eye_position - g_car_position)*smooth_timer / 60, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
		if (smooth_timer >= 60.f)
		{
			smooth_timer = 0.f;
			g_camera_mode = 2;
		}
		else
			smooth_timer += 0.75f;
	}
	else
	{
		// Projection Transform Matrix ����.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix ����.
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(g_camera.GetGLViewMatrix()));
	}
	if (n_mode)
	{
		{
			// Directional Light ����.
				// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl ����.
			int type_loc = glGetUniformLocation(s_program_id, "lights[0].type");
			glUniform1i(type_loc, 0);

			// ���� ���� ����.
			glm::vec3 dir(-1.f, -1.f, 0.f);
			dir = glm::normalize(dir);

			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  dir�� ������ ��Ÿ���� �����̹Ƿ� �̵�(Translation)��ȯ�� ���õǵ��� �Ѵ�. (�� ��° ��� 0.f���� ����)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[0].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// ���� ���� ����.
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[0].intensity");
			glUniform3f(intensity_loc, 0.1f, 0.1f, 0.1f);
		}
		// ���� Headlight
		{
			// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl ����.
			int type_loc = glGetUniformLocation(s_program_id, "lights[1].type");
			glUniform1i(type_loc, 2);

			// ���� ����ϴ� ��ġ(����) ����.
			glm::vec3 pos = g_car_position + glm::rotateY(glm::vec3(0.17f, 0.4f, 0.36f), g_car_rotation_y);
			// Apply Camera Matrices
			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  �̶� pos�� ��ġ�� ��Ÿ���� ����Ʈ�̹Ƿ� �̵�(Translation)��ȯ�� ����ǵ��� �Ѵ�. (�� ��° ��� 1.f���� ����)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[1].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);


			// ���� ���� ����.
			glm::vec3 dir(0.f, -0.3f, 1.f);
			dir = glm::rotateY(dir, g_car_rotation_y);
			dir = glm::normalize(dir);

			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  dir�� ������ ��Ÿ���� �����̹Ƿ� �̵�(Translation)��ȯ�� ���õǵ��� �Ѵ�. (�� ��° ��� 0.f���� ����)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[1].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);



			// ���� ���� ����
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[1].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 1.f);

			// ���� ���� ���� ����.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[1].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(30.f / 180.f * glm::pi<float>()));
		}
		// ������ Headlight
		{
			// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl ����.
			int type_loc = glGetUniformLocation(s_program_id, "lights[2].type");
			glUniform1i(type_loc, 2);

			// ���� ����ϴ� ��ġ(����) ����.
			glm::vec3 pos = g_car_position + glm::rotateY(glm::vec3(-0.17f, 0.4f, 0.36f), g_car_rotation_y);
			// Apply Camera Matrices
			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  �̶� pos�� ��ġ�� ��Ÿ���� ����Ʈ�̹Ƿ� �̵�(Translation)��ȯ�� ����ǵ��� �Ѵ�. (�� ��° ��� 1.f���� ����)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[2].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);


			// ���� ���� ����.
			glm::vec3 dir(0.f, -0.3f, 1.f);
			dir = glm::rotateY(dir, g_car_rotation_y);
			dir = glm::normalize(dir);

			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  dir�� ������ ��Ÿ���� �����̹Ƿ� �̵�(Translation)��ȯ�� ���õǵ��� �Ѵ�. (�� ��° ��� 0.f���� ����)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[2].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);



			// ���� ���� ����
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[2].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 1.f);

			// ���� ���� ���� ����.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[2].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(30.f / 180.f * glm::pi<float>()));
		}
		// Point Light ����
		for (int j = 0; j < 6; j++)
		{
			for (int i = 3; i < 15-(2*j); i++)
			{
				
				s_Type = s + std::to_string(i + ((15 - 2 * j)*j)) + "].type";
				c = s_Type.c_str();
				glUniform1f(glGetUniformLocation(s_program_id, "emit"), true);
				// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light)
				int type_loc = glGetUniformLocation(s_program_id, c);
				glUniform1i(type_loc, 1);

				// ���� ����ϴ� ��ġ(����) ����.
				// �ð��� ���� ��ġ�� ���ϵ��� ��.
				glm::vec3 pos((2.f-(j*0.4f)) * cos(g_elaped_time_s - (1200/(12-2*j))*i), 0.5f + (j*0.5f), (2.f - (j*0.4f))* sin(g_elaped_time_s - (1200 / (12 - 2 * j))*i)) ;
				pos = pos + +g_car_position;

				// Apply Camera Matrices
				////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
				//  �̶� pos�� ��ġ�� ��Ÿ���� ����Ʈ�̹Ƿ� �̵�(Translation)��ȯ�� ����ǵ��� �Ѵ�. (�� ��° ��� 1.f���� ����)
				glm::mat4 modelview_T = glm::translate(pos) * glm::scale(glm::vec3(0.03f, 0.03f, 0.03f));
				glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(modelview_T));

				glUniform3f(glGetUniformLocation(s_program_id, "K_s"), glm::abs(sin((g_elaped_time_s - (100.f*i)) + glm::pi<float>() / 2)), glm::abs(sin((g_elaped_time_s - (100.f*i)))), glm::abs(cos(g_elaped_time_s)));
				glVertexAttrib4f(1, 0.3f, 0.6f, 0.9f, 1.f);
				DrawSphere();
				pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));
				s_Position = s + std::to_string(i + ((15-2*j)*j)) + "].position";
				c = s_Position.c_str();
				int pos_loc = glGetUniformLocation(s_program_id, c);
				glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

				// ���� ���� ����.
				s_Intensity = s + std::to_string(i + ((15 - 2 * j)*j)) + "].intensity";
				c = s_Intensity.c_str();
				int intensity_loc = glGetUniformLocation(s_program_id, c);
				glUniform3f(intensity_loc, glm::abs(sin((g_elaped_time_s-(100.f*i)) + glm::pi<float>() / 2 )), glm::abs(sin((g_elaped_time_s - (100.f*i)))), glm::abs(cos(g_elaped_time_s)));

				glUniform1f(glGetUniformLocation(s_program_id, "emit"), false);
				
				
			}
		}
	}
	
	// �ٴ� ����
	// Ground�� ���� Phong Shading ���� ���� ���� �����Ѵ�.
	int shininess_loc = glGetUniformLocation(s_program_id, "shininess_n");
	glUniform1f(shininess_loc, 50.f);

	int K_s_loc = glGetUniformLocation(s_program_id, "K_s");
	glUniform3f(K_s_loc, 0.3f, 0.3f, 0.3f);

	glm::mat4 T0(1.f); // ���� ���
	glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(T0));
	DrawGround2();

	

	// Moving Car
	{
		int shininess_loc = glGetUniformLocation(s_program_id, "shininess_n");
		glUniform1f(shininess_loc, 100.f);

		int K_s_loc = glGetUniformLocation(s_program_id, "K_s");
		glUniform3f(K_s_loc, 0.7f, 0.7f, 0.7f);

		glm::mat4 car_T = glm::translate(g_car_position) * glm::rotate(g_car_rotation_y, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE,  glm::value_ptr(car_T));
		DrawCarModel();
	}
	// ����
	for (int i = 0; i < 5; i++)
	{
		for (int j = 1; j < 6; j++)
		{
			int shininess_loc = glGetUniformLocation(s_program_id, "shininess_n");
			glUniform1f(shininess_loc, 100.f);

			int K_s_loc = glGetUniformLocation(s_program_id, "K_s");
			glUniform3f(K_s_loc, 0.7f, 0.7f, 0.7f);

			glm::mat4 model_T;
			model_T = glm::translate(glm::vec3(i * 2.f - 4.0f, 0.f, j * 2.f - 6.0f));
			glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));
			DrawTreeModel(tree_arr[i][j-1]);

			

			
		}
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 1; j < 6; j++)
		{
			int shininess_loc = glGetUniformLocation(s_program_id, "shininess_n");
			glUniform1f(shininess_loc, 100.f);

			int K_s_loc = glGetUniformLocation(s_program_id, "K_s");
			glUniform3f(K_s_loc, 0.7f, 0.7f, 0.7f);
			glm::mat4 model_F;
			if (tree_arr[i][j - 1] >= 4)
			{
				float falling_Y = tree_arr[i][j - 1] - std::fmod(g_elaped_time_s, tree_arr[i][j - 1]) - ((5 - tree_arr[i][j - 1]) * 2 + 0.5);
				if (falling_Y > 0.f)
				{
					model_F = glm::translate(glm::vec3(sin(g_elaped_time_s + 5 * i), 0.f, sin(g_elaped_time_s + 3 * i)))
						* glm::translate(glm::vec3(i * 2.f - 3.8f, falling_Y, j * 2.f - 5.8f))
						* glm::rotate(sin(g_elaped_time_s), glm::vec3(1.f, 0.f, 0.f));
					glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_F));

					DrawDropFlowerModel();
				}
			}
		}
	}


	// flipping the double buffers
	// glutSwapBuffers�� �׻� Display �Լ� ���� �Ʒ� �κп��� �� ���� ȣ��Ǿ���Ѵ�.
	glutSwapBuffers();
}


/**
Timer: ������ �ð� �Ŀ� �ڵ����� ȣ��Ǵ� callback �Լ�.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node64.html#SECTION000819000000000000000
*/
void Timer(int value)
{
	// Timer ȣ�� �ð� ������ �����Ͽ�, ���� Timer�� ȣ��� �ĺ��� ������� �귯�� ����Ѵ�.
	g_elaped_time_s += value/1000.f;


	// Turn
	g_car_rotation_y += g_car_angular_speed;

	// Calculate Velocity
	glm::vec3 speed_v = glm::vec3(0.f, 0.f, g_car_speed);
	glm::vec3 velocity = glm::rotateY(speed_v, g_car_rotation_y);	// speed_v �� y���� �������� g_car_rotation_y ��ŭ ȸ���Ѵ�.

	// Move
	g_car_position += velocity;


	// glutPostRedisplay�� ������ ���� �ð� �ȿ� ��ü �׸��� �ٽ� �׸� ���� �ý��ۿ� ��û�Ѵ�.
	// ��������� Display() �Լ��� ȣ�� �ȴ�.
	glutPostRedisplay();

	// 1/60 �� �Ŀ� Timer �Լ��� �ٽ� ȣ��Ƿη� �Ѵ�.
	// Timer �Լ� �� ������ �ð� �������� �ݺ� ȣ��ǰ��Ͽ�,
	// �ִϸ��̼� ȿ���� ǥ���� �� �ִ�
	glutTimerFunc((unsigned int)(1000 / 60), Timer, (1000 / 60));
}




/**
Reshape: �������� ũ�Ⱑ ������ ������ �ڵ����� ȣ��Ǵ� callback �Լ�.

@param w, h�� ���� ������ �������� ���� ũ��� ���� ũ�� (�ȼ� ����).
ref: https://www.opengl.org/resources/libraries/glut/spec3/node48.html#SECTION00083000000000000000
*/
void Reshape(int w, int h)
{
	//  w : window width   h : window height
	g_window_w = w;
	g_window_h = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glutPostRedisplay();
}

/**
Keyboard: Ű���� �Է��� ���� ������ �ڵ����� ȣ��Ǵ� �Լ�.
@param key�� ������ Ű������ ���ڰ�.
@param x,y�� ���� ���콺 �������� ��ǥ��.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = -0.018f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.018f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = glm::radians( 1.f );		// ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = -1 * glm::radians( 1.f );		//  ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	case '1':
		g_camera_mode = 0;
		glutPostRedisplay();
		break;

	case '2':
		g_camera_mode = 1;
		glutPostRedisplay();
		break;
	case '3':
		g_camera_mode = 2;
		glutPostRedisplay();
		break;
	case '4':
		if (g_camera_mode == 2 || g_camera_mode == 4)
		{
			smooth_timer = 0.f;
			g_ex_camera_position = g_camera_position;
			g_camera_mode = 3;
		}
		
		glutPostRedisplay();
		break;
	case '5':
		if (g_camera_mode == 1 || g_camera_mode == 3)
		{
			smooth_timer = 0.f;
			g_ex_camera_position = g_camera_position;
			g_camera_mode = 4;
		}
		
		glutPostRedisplay();
		break;
	case 'n':
		n_mode = (!n_mode); // �㳷 ��ȯ
		// Fragment shader�� ���� �Ǿ��ִ� 'shading_mode' ������ location�� �޾ƿ´�.
		int shading_mode_loc = glGetUniformLocation(s_program_id, "shading_mode");
		if (n_mode)
			glUniform1i(shading_mode_loc, 2);// 'shading_mode' ������ 2�� ����.
		else
			glUniform1i(shading_mode_loc, 1);
		

		break;
	}
	
}

/**
KeyboardUp: �������� Ű�� ������ ������ �ڵ����� ȣ��Ǵ� �Լ�.
@param key�� �ش� Ű������ ���ڰ�.
@param x,y�� ���� ���콺 �������� ��ǥ��.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = 0.f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = 0.f;		// ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = 0.f;		//  ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	}

}



/**
Mouse: ���콺 ��ư�� �Էµ� ������ �ڵ����� ȣ��Ǵ� �Լ�.
�Ķ������ �ǹ̴� ������ ����.
@param button: ���� ��ư�� ����
  GLUT_LEFT_BUTTON - ���� ��ư
  GLUT_RIGHT_BUTTON - ������ ��ư
  GLUT_MIDDLE_BUTTON - ��� ��ư (���� �������� ��)
  3 - ���콺 �� (���� ���� ���� ����).
  4 - ���콺 �� (���� �Ʒ��� ���� ����).
@param state: ���� ����
  GLUT_DOWN - ���� ����
  GLUT_UP - ��������
@param x,y: ������ �Ͼ�� ��, ���콺 �������� ��ǥ��.
*/
void Mouse(int button, int state, int x, int y)
{
	float mouse_xd = (float)x / g_window_w;
	float mouse_yd = 1 - (float)y / g_window_h;
	float last_mouse_xd = (float)g_last_mouse_x / g_window_w;
	float last_mouse_yd = 1 - (float)g_last_mouse_y / g_window_h;


	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_left_button_pushed = true;

	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		g_left_button_pushed = false;

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		g_right_button_pushed = true;

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		g_right_button_pushed = false;
	else if (button == 3)
	{
		g_camera.inputMouse(Camera::IN_TRANS_Z, 0, -1, 0.01f);
		glutPostRedisplay();
	}
	else if (button == 4)
	{
		g_camera.inputMouse(Camera::IN_TRANS_Z, 0, 1, 0.01f);
		glutPostRedisplay();
	}

	g_last_mouse_x = x;
	g_last_mouse_y = y;
}





/**
MouseMotion: ���콺 �����Ͱ� ������ ������ �ڵ����� ȣ��Ǵ� �Լ�.
@prarm x,y�� ���� ���콺 �������� ��ǥ���� ��Ÿ����.
*/
void MouseMotion(int x, int y)
{
	float mouse_xd = (float)x / g_window_w;
	float mouse_yd = 1 - (float)y / g_window_h;
	float last_mouse_xd = (float)g_last_mouse_x / g_window_w;
	float last_mouse_yd = 1 - (float)g_last_mouse_y / g_window_h;

	if (g_left_button_pushed)
	{
		g_camera.inputMouse(Camera::IN_ROTATION_Y_UP, last_mouse_xd, last_mouse_yd, mouse_xd, mouse_yd);
		glutPostRedisplay();
	}
	else if (g_right_button_pushed)
	{
		g_camera.inputMouse(Camera::IN_TRANS, last_mouse_xd, last_mouse_yd, mouse_xd, mouse_yd, 0.01f);
		glutPostRedisplay();
	}

	g_last_mouse_x = x;
	g_last_mouse_y = y;
}