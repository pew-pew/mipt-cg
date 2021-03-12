#version 100

attribute vec3 pos;
attribute vec3 inColor;
varying vec3 color;
uniform mat4 trans;

void main()
{
   gl_Position = trans * vec4(pos.x, pos.y, pos.z, 1.0);
   color = vec3(1, 1, 1) * pos.y / 3.;
}                           
