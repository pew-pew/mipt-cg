#version 330 core

#define MAX_NUM_OF_LIGHTS 10

layout(location = 0) in vec3 pos_model;
layout(location = 2) in vec2 tex_coord_in;
layout(location = 3) in vec3 normal_model;

out GS_OUT {
  vec2 tex_coord;
  vec3 to_camera, normal;
  vec3 to_light_array[MAX_NUM_OF_LIGHTS];
};

uniform mat4 M, V, P;

uniform vec3 light_pos_array[MAX_NUM_OF_LIGHTS];
uniform int number_of_lights;

void main() {
  tex_coord = (M * vec4(pos_model, 1)).xz;
//  gl_Position = V * M * vec4(pos_model, 1);
  gl_Position = P * V * M * vec4(pos_model, 1);

  vec3 pos_camspace = (V * M * vec4(pos_model, 1.0)).xyz;
  vec3 normal_camspace = (V * M * vec4(normal_model, 0.0)).xyz;
  for (int i = 0; i < number_of_lights; ++i) {
    to_light_array[i] = (V * vec4(light_pos_array[i], 1.0)).xyz - pos_camspace;
  }

  vec3 camera_pos_camspace = vec3(0.0, 0.0, 0.0);

  normal = normal_camspace;
  to_camera = camera_pos_camspace - pos_camspace;
}
