#version 330 core

layout (location = 0) in vec3 pos_model;

out vec3 tex_coord;

uniform mat4 V, P;

void main()
{
  tex_coord = pos_model;
  gl_Position = P * mat4(mat3(V)) * vec4(pos_model, 1.0);
}