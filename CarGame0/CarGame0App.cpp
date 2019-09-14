


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
//// 자동차 제어 변수들.
//////////////////////////////////////////////////////////////////////
glm::vec3 g_car_position(0.f, 0.f, 0.f); //위치
float g_car_speed = 0;			          // 속도 (초당 이동 거리)
float g_car_rotation_y = 0;		          // 현재 방향 (y축 회전)
float g_car_angular_speed = 0;	          // 회전 속도 (각속도 - 초당 회전 각)

bool n_mode = false; // 밤&낮 전환 변수

std::string s_Type;
std::string s_Intensity;
std::string s_Position;
std::string s = "lights[";
const char * c;

int tree_arr[5][5] = { {2, 4, 5, 1, 3}, { 3, 5, 1, 2, 4}, {4, 2, 3, 1, 5} ,{1, 4, 3, 5, 2}, {5, 1, 4, 2, 3} };// 나무 배열

float ex_falling_Y;
/**
InitOpenGL: 프로그램 초기 값 설정을 위해 최초 한 번 호출되는 함수. (main 함수 참고)
OpenGL에 관련한 초기 값과 프로그램에 필요한 다른 초기 값을 설정한다.
예를들어, VAO와 VBO를 여기서 생성할 수 있다.
*/
void InitOpenGL()
{
	//////////////////////////////////////////////////////////////////////////////////////
	//// 3. Shader Programs 등록
	////    Ref: https://www.khronos.org/opengl/wiki/Shader_Compilation
	//////////////////////////////////////////////////////////////////////////////////////
	s_program_id = CreateFromFiles("../Shaders/v_shader.glsl", "../Shaders/f_shader.glsl");
	glUseProgram(s_program_id);



	////////////////////////////////////////////////////////////////////////////////////
	//// 4. OpenGL 설정
	//////////////////////////////////////////////////////////////////////////////////////
	glViewport(0, 0, (GLsizei)g_window_w, (GLsizei)g_window_h);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// Initial State of Camera
	// 카메라 초기 위치 설정한다.
	g_camera.lookAt(glm::vec3(3.f, 2.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	////////////////////////////////////////////////////////////////////////////////////
	//// 5. VAO, VBO 생성
	////    Ref: https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object
	///////////////////////////////////////////////////////////////////////////////////
	
	// basic meshes
	InitBasicShapeObjs();

	// Tree
	InitTreeModel();
	InitDropFlowerModel();
	// Car
	InitCarModel();

	// 바닥 격자 VAO 생성
	InitGround2();
}







/**
ClearOpenGLResource: 프로그램이 끝나기 메모리 해제를 위해 한 번 호출되는 함수. (main 함수 참고)
프로그램에서 사용한 메모리를 여기에서 해제할 수 있다.
예를들어, VAO와 VBO를 여기서 지울 수 있다.
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
Display: 윈도우 화면이 업데이트 될 필요가 있을 때 호출되는 callback 함수.

윈도우 상에 최종 결과를 렌더링 하는 코드는 이 함수 내에 구현해야한다.
원도우가 처음 열릴 때, 윈도우 크기가 바뀔 때, 다른 윈도우에 의해 화면의 일부
또는 전체가 가려졌다가 다시 나타날 때 등 시스템이 해당 윈도우 내의 그림에 대한
업데이트가 필요하다고 판단하는 경우 자동으로 호출된다.
강제 호출이 필요한 경우에는 glutPostRedisplay() 함수를 호출하면된다.

이 함수는 불시에 빈번하게 호출된다는 것을 명심하고, 윈도우 상태 변화와 무관한
1회성 코드는 가능한한 이 함수 밖에 구현해야한다. 특히 메모리 할당, VAO, VBO 생성
등의 하드웨어 점유를 시도하는 코드는 특별한 이유가 없다면 절대 이 함수에 포함시키면
안된다. 예를 들어, 메시 모델을 정의하고 VAO, VBO를 설정하는 부분은 최초 1회만
실행하면되므로 main() 함수 등 외부에 구현해야한다. 정의된 메시 모델을 프레임 버퍼에
그리도록 지시하는 코드만 이 함수에 구현하면 된다.

만일, 이 함수 내에서 동적 메모리 할당을 해야하는 경우가 있다면 해당 메모리는 반드시
이 함수가 끝나기 전에 해제 해야한다.

ref: https://www.opengl.org/resources/libraries/glut/spec3/node46.html#SECTION00081000000000000000
*/
void Display()
{
	// 전체 화면을 지운다.
	// glClear는 Display 함수 가장 윗 부분에서 한 번만 호출되어야한다.
	if(n_mode)
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	else
		glClearColor(1.f, 1.f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	// Vertex shader 의 matrix 변수들의 location을 받아온다.
	int m_proj_loc = glGetUniformLocation(s_program_id, "proj_matrix");
	int m_view_loc = glGetUniformLocation(s_program_id, "view_matrix");
	int m_model_loc = glGetUniformLocation(s_program_id, "model_matrix");

	glm::mat4 view_matrix = g_camera.GetGLViewMatrix();
	glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

	int num_of_lights_loc = glGetUniformLocation(s_program_id, "num_of_lights");
	glUniform1i(num_of_lights_loc, 100);


	if ( g_camera_mode == 1 )// Driver view
	{
		// Projection Transform Matrix 설정.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(90.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		// Camera Transform Matrix 설정.
		
		g_camera_position = { 0.f, 0.6f, -0.05f };
		g_driver_eye_position = glm::rotateY(g_camera_position + glm::vec3(0.f, 0.f, 3.f), g_car_rotation_y) + g_car_position;
		g_camera_position = glm::rotateY(g_camera_position, g_car_rotation_y) + g_car_position;
		view_matrix = glm::lookAt(g_camera_position, g_driver_eye_position, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else if (g_camera_mode == 2)// Birdeye view
	{
		// Projection Transform Matrix 설정.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
		g_camera_position = { 0.f, 3.5f, -4.f };
		g_camera_position = glm::rotateY(g_camera_position, g_car_rotation_y)+ g_car_position;

		view_matrix = glm::lookAt(g_camera_position, g_car_position, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else if (g_camera_mode == 3)// Smooth Driver view
	{
		// Projection Transform Matrix 설정.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.f) + glm::radians(45.f * smooth_timer / 60) , (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
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
		// Projection Transform Matrix 설정.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(90.f) - glm::radians(45.f * smooth_timer / 60), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
		
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
		// Projection Transform Matrix 설정.
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(g_camera.GetGLViewMatrix()));
	}
	if (n_mode)
	{
		{
			// Directional Light 설정.
				// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
			int type_loc = glGetUniformLocation(s_program_id, "lights[0].type");
			glUniform1i(type_loc, 0);

			// 빛의 방향 설정.
			glm::vec3 dir(-1.f, -1.f, 0.f);
			dir = glm::normalize(dir);

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[0].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// 빛의 세기 설정.
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[0].intensity");
			glUniform3f(intensity_loc, 0.1f, 0.1f, 0.1f);
		}
		// 왼쪽 Headlight
		{
			// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
			int type_loc = glGetUniformLocation(s_program_id, "lights[1].type");
			glUniform1i(type_loc, 2);

			// 빛이 출발하는 위치(광원) 설정.
			glm::vec3 pos = g_car_position + glm::rotateY(glm::vec3(0.17f, 0.4f, 0.36f), g_car_rotation_y);
			// Apply Camera Matrices
			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[1].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);


			// 빛의 방향 설정.
			glm::vec3 dir(0.f, -0.3f, 1.f);
			dir = glm::rotateY(dir, g_car_rotation_y);
			dir = glm::normalize(dir);

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[1].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);



			// 빛의 세기 설정
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[1].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 1.f);

			// 빛의 퍼짐 정도 설정.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[1].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(30.f / 180.f * glm::pi<float>()));
		}
		// 오른쪽 Headlight
		{
			// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
			int type_loc = glGetUniformLocation(s_program_id, "lights[2].type");
			glUniform1i(type_loc, 2);

			// 빛이 출발하는 위치(광원) 설정.
			glm::vec3 pos = g_car_position + glm::rotateY(glm::vec3(-0.17f, 0.4f, 0.36f), g_car_rotation_y);
			// Apply Camera Matrices
			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[2].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);


			// 빛의 방향 설정.
			glm::vec3 dir(0.f, -0.3f, 1.f);
			dir = glm::rotateY(dir, g_car_rotation_y);
			dir = glm::normalize(dir);

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[2].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);



			// 빛의 세기 설정
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[2].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 1.f);

			// 빛의 퍼짐 정도 설정.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[2].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(30.f / 180.f * glm::pi<float>()));
		}
		// Point Light 설정
		for (int j = 0; j < 6; j++)
		{
			for (int i = 3; i < 15-(2*j); i++)
			{
				
				s_Type = s + std::to_string(i + ((15 - 2 * j)*j)) + "].type";
				c = s_Type.c_str();
				glUniform1f(glGetUniformLocation(s_program_id, "emit"), true);
				// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light)
				int type_loc = glGetUniformLocation(s_program_id, c);
				glUniform1i(type_loc, 1);

				// 빛이 출발하는 위치(광원) 설정.
				// 시간에 따라 위치가 변하도록 함.
				glm::vec3 pos((2.f-(j*0.4f)) * cos(g_elaped_time_s - (1200/(12-2*j))*i), 0.5f + (j*0.5f), (2.f - (j*0.4f))* sin(g_elaped_time_s - (1200 / (12 - 2 * j))*i)) ;
				pos = pos + +g_car_position;

				// Apply Camera Matrices
				////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
				//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
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

				// 빛의 세기 설정.
				s_Intensity = s + std::to_string(i + ((15 - 2 * j)*j)) + "].intensity";
				c = s_Intensity.c_str();
				int intensity_loc = glGetUniformLocation(s_program_id, c);
				glUniform3f(intensity_loc, glm::abs(sin((g_elaped_time_s-(100.f*i)) + glm::pi<float>() / 2 )), glm::abs(sin((g_elaped_time_s - (100.f*i)))), glm::abs(cos(g_elaped_time_s)));

				glUniform1f(glGetUniformLocation(s_program_id, "emit"), false);
				
				
			}
		}
	}
	
	// 바닥 격자
	// Ground를 위한 Phong Shading 관련 변수 값을 설정한다.
	int shininess_loc = glGetUniformLocation(s_program_id, "shininess_n");
	glUniform1f(shininess_loc, 50.f);

	int K_s_loc = glGetUniformLocation(s_program_id, "K_s");
	glUniform3f(K_s_loc, 0.3f, 0.3f, 0.3f);

	glm::mat4 T0(1.f); // 단위 행렬
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
	// 나무
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
	// glutSwapBuffers는 항상 Display 함수 가장 아래 부분에서 한 번만 호출되어야한다.
	glutSwapBuffers();
}


/**
Timer: 지정된 시간 후에 자동으로 호출되는 callback 함수.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node64.html#SECTION000819000000000000000
*/
void Timer(int value)
{
	// Timer 호출 시간 간격을 누적하여, 최초 Timer가 호출된 후부터 현재까지 흘러간 계산한다.
	g_elaped_time_s += value/1000.f;


	// Turn
	g_car_rotation_y += g_car_angular_speed;

	// Calculate Velocity
	glm::vec3 speed_v = glm::vec3(0.f, 0.f, g_car_speed);
	glm::vec3 velocity = glm::rotateY(speed_v, g_car_rotation_y);	// speed_v 를 y축을 기준으로 g_car_rotation_y 만큼 회전한다.

	// Move
	g_car_position += velocity;


	// glutPostRedisplay는 가능한 빠른 시간 안에 전체 그림을 다시 그릴 것을 시스템에 요청한다.
	// 결과적으로 Display() 함수가 호출 된다.
	glutPostRedisplay();

	// 1/60 초 후에 Timer 함수가 다시 호출되로록 한다.
	// Timer 함수 가 동일한 시간 간격으로 반복 호출되게하여,
	// 애니메이션 효과를 표현할 수 있다
	glutTimerFunc((unsigned int)(1000 / 60), Timer, (1000 / 60));
}




/**
Reshape: 윈도우의 크기가 조정될 때마다 자동으로 호출되는 callback 함수.

@param w, h는 각각 조정된 윈도우의 가로 크기와 세로 크기 (픽셀 단위).
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
Keyboard: 키보드 입력이 있을 때마다 자동으로 호출되는 함수.
@param key는 눌려진 키보드의 문자값.
@param x,y는 현재 마우스 포인터의 좌표값.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = -0.018f;		// 후진 속도 설정
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.018f;		// 전진 속도 설정
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = glm::radians( 1.f );		// 좌회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = -1 * glm::radians( 1.f );		//  우회전 각속도 설정
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
		n_mode = (!n_mode); // 밤낮 전환
		// Fragment shader에 정의 되어있는 'shading_mode' 변수의 location을 받아온다.
		int shading_mode_loc = glGetUniformLocation(s_program_id, "shading_mode");
		if (n_mode)
			glUniform1i(shading_mode_loc, 2);// 'shading_mode' 값으로 2을 설정.
		else
			glUniform1i(shading_mode_loc, 1);
		

		break;
	}
	
}

/**
KeyboardUp: 눌려졌던 키가 놓여질 때마다 자동으로 호출되는 함수.
@param key는 해당 키보드의 문자값.
@param x,y는 현재 마우스 포인터의 좌표값.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = 0.f;		// 후진 속도 설정
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.f;		// 전진 속도 설정
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = 0.f;		// 좌회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = 0.f;		//  우회전 각속도 설정
		glutPostRedisplay();
		break;

	}

}



/**
Mouse: 마우스 버튼이 입력될 때마다 자동으로 호출되는 함수.
파라메터의 의미는 다음과 같다.
@param button: 사용된 버튼의 종류
  GLUT_LEFT_BUTTON - 왼쪽 버튼
  GLUT_RIGHT_BUTTON - 오른쪽 버튼
  GLUT_MIDDLE_BUTTON - 가운데 버튼 (휠이 눌러졌을 때)
  3 - 마우스 휠 (휠이 위로 돌아 갔음).
  4 - 마우스 휠 (휠이 아래로 돌아 갔음).
@param state: 조작 상태
  GLUT_DOWN - 눌러 졌음
  GLUT_UP - 놓여졌음
@param x,y: 조작이 일어났을 때, 마우스 포인터의 좌표값.
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
MouseMotion: 마우스 포인터가 움직일 때마다 자동으로 호출되는 함수.
@prarm x,y는 현재 마우스 포인터의 좌표값을 나타낸다.
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