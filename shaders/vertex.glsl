#version 330 core

layout(location = 0) in vec3 pos_model;
layout(location = 2) in vec2 tex_coord_in;
layout(location = 3) in vec3 normal_model;

out vec2 tex_coord;
out vec3 to_light, to_camera, normal;

uniform mat4 M, V, P;
uniform vec3 light_pos;

void main() {
   tex_coord = tex_coord_in;
   gl_Position = P * V * M * vec4(pos_model, 1);

   vec3 pos_camspace = (V * M * vec4(pos_model, 1.0)).xyz;
   vec3 normal_camspace = (V * M * vec4(normal_model, 0.0)).xyz;
   vec3 light_pos_camspace = (V * vec4(light_pos, 1.0)).xyz;
   vec3 camera_pos_camspace = vec3(0.0, 0.0, 0.0);

   to_light = light_pos_camspace - pos_camspace;
   normal = normal_camspace;
   to_camera = camera_pos_camspace - pos_camspace;
}