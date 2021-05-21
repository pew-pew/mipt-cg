#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
  vec2 tex_coord;
  vec3 to_light, to_camera, normal;
} vs_out[3];

out GS_OUT {
  vec2 tex_coord;
  vec3 to_light, to_camera, normal;
} gs_out;

uniform mat4 M, V, P;
uniform float explosionTime;
uniform vec3 explosionDir_world;
uniform float explosionTotalTime;
uniform vec3 explosionPos_world;

vec3 rotateAround(vec3 v, vec3 axis, float angle) {
  vec4 q = vec4(sin(angle) * axis, cos(angle));
  return v + 2 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void main() {
  vec3 pos[3];
  for (int i = 0; i < 3; i++)
    pos[i] = gl_in[i].gl_Position.xyz;
  // vec3 normal = normalize(cross(pos[1] - pos[0], pos[2] - pos[0]));
  // vec3 normal = normalize(vs_out[0].normal + vs_out[1].normal + vs_out[2].normal);

  vec3 explosionPos = (V * vec4(explosionPos_world, 1)).xyz; // don't care
  // vec3 explosionPos = (V * M * vec4(vec3(0, 0, 0), 1)).xyz; // don't care
  vec3 partPos = (pos[0] + pos[1] + pos[2]) / 3;

  float explosionSpeed = 1 / (1 + pow(length(partPos - explosionPos), 6) * 100);
  vec3 explosionDir = ((V * vec4(explosionDir_world, 0)).xyz) + 10 * normalize(partPos - explosionPos);

  float pseudorandom = gl_PrimitiveIDIn;
  vec3 rotAxis = normalize(vec3(sin(pseudorandom * 10), cos(pseudorandom * 20), sin(pseudorandom * 30)));
  float rotSpeed = (cos(pseudorandom * 30) + 1) / 2;

  float scale = 1 - explosionTime / explosionTotalTime;

  for (int i = 0; i < 3; i++) {
    gs_out.tex_coord = vs_out[i].tex_coord;
    gs_out.to_light = vs_out[i].to_light;
    gs_out.to_camera = vs_out[i].to_camera;
    gs_out.normal = vs_out[i].normal;

    vec3 newPos = (
      partPos
      + scale * rotateAround(pos[i] - partPos, rotAxis, rotSpeed * explosionTime)
      + explosionSpeed * explosionTime * explosionDir
    );
    gl_Position = P * vec4(newPos, 1);
    EmitVertex();
  }    
  EndPrimitive();
}  