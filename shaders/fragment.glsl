#version 330 core

in vec2 tex_coord;
in vec3 to_light, to_camera, normal;

uniform sampler2D tex;
uniform float ambientK;

out vec4 color;

void main()
{
  vec2 tex_coord_wrapped = tex_coord - floor(tex_coord); // webgl can't handle GL_REPEAT (non 2^p textures), emulate it manually
  vec3 reflectance = texture(tex, tex_coord_wrapped).xyz;

  float lightPower = min(1 / dot(to_light, to_light), 1);

  float diffuseK = max(0, dot(normalize(to_light), normalize(normal)));
  float specularK = max(0, dot(normalize(to_camera), reflect(-normalize(to_light), normalize(normal))));

  vec3 lightColor = vec3(0.9, 0.7, 0);
  vec3 incoming = lightColor * (lightPower * (pow(specularK, 5) * 0.2  + diffuseK) + ambientK);

  color = vec4(reflectance * incoming, 1);
}