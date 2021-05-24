#version 330 core
out vec4 color;

in vec3 tex_coord;

uniform samplerCube skybox;

void main()
{
  color = texture(skybox, tex_coord);
}