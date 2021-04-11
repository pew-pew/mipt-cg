#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 tex_coord_in;

out vec2 tex_coord;

uniform mat4 MVP;

void main() {
   gl_Position = MVP * vec4(pos, 1);
   tex_coord = tex_coord_in;
}