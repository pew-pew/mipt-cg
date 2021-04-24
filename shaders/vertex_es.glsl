#version 100

attribute vec3 pos_model;
attribute vec3 color_unused;
attribute vec2 tex_coord_in;
attribute vec3 normal_model;

varying vec2 tex_coord;
varying vec3 to_light, to_camera, normal;
varying float unused_hack;

uniform sampler2D tex;
uniform vec3 light_pos;
uniform mat4 M, V, P;

void main()
{
    unused_hack = color_unused.x;

    tex_coord = tex_coord_in;
    gl_Position = P * V * M * vec4(pos_model, 1.0);

    vec3 pos_camspace = (V * M * vec4(pos_model, 1)).xyz;
    vec3 normal_camspace = (V * M * vec4(normal_model, 0)).xyz;
    vec3 light_pos_camspace = (V * vec4(light_pos, 1)).xyz;
    vec3 camera_pos_camspace = vec3(0, 0, 0);

    to_light = light_pos_camspace - pos_camspace;
    normal = normal_camspace;
    to_camera = camera_pos_camspace - pos_camspace;
}