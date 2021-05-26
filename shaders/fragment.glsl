#version 330 core

#define MAX_NUM_OF_LIGHTS 10

in GS_OUT {
  vec2 tex_coord;
  vec3 to_camera, normal;
  vec3 to_light_array[MAX_NUM_OF_LIGHTS];
};

uniform sampler2D tex;
uniform float ambientK;
uniform int number_of_lights;

out vec4 color;

void main()
{
  vec2 tex_coord_wrapped = tex_coord - floor(tex_coord); // webgl can't handle GL_REPEAT (non 2^p textures), emulate it manually
  vec3 reflectance = texture(tex, tex_coord_wrapped).xyz;
  vec3 lightColor = vec3(0.9, 0.9, 0.9);

  float totalLight = 0;
  for (int i = 0; i < number_of_lights; ++i) {
    float lightPower = min(1 / dot(to_light_array[i], to_light_array[i]), 1);

    float diffuseK = max(0, dot(normalize(to_light_array[i]), normalize(normal)));
    float specularK = max(0, dot(normalize(to_camera), reflect(-normalize(to_light_array[i]), normalize(normal))));
    
    totalLight += lightPower * (pow(specularK, 5) * 0.2  + diffuseK);
  }
  
  vec3 incoming = lightColor * (totalLight + ambientK);

  color = vec4(reflectance * incoming, 1);
}