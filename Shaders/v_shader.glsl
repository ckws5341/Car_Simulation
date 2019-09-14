#version 330

layout (location = 0) in vec3 vs_position;
layout (location = 1) in vec3 vs_normal;
layout (location = 2) in vec4 vs_color;

uniform mat4 proj_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

out vec3 fs_normal;
out vec4 fs_color;
out vec3 fs_eye_dir;	// 보는 대상 (꼭지점)이 있는 곳으로 향하는 벡터.


void main()
{
	mat4 modelview = view_matrix * model_matrix;

	gl_Position = proj_matrix * view_matrix * model_matrix * vec4(vs_position, 1.f);
	fs_normal = vec3(view_matrix * model_matrix * vec4(vs_normal, 0.f));
	fs_eye_dir = vec3(modelview * vec4(vs_position, 1.f));	// Projection을 적용하지 않음.

	fs_color = vs_color;
}