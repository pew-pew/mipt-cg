// #version 330 core

// layout (location = 0) in vec3 pos;
// layout (location = 1) in vec3 inColor;
// flat out vec3 color;
// uniform mat4 trans;

// void main() {
//    gl_Position = trans * vec4(pos.x, pos.y, pos.z, 1.0);
//    color = inColor;
// }


#version 100

attribute vec3 pos;
attribute vec3 inColor;
varying vec3 color;
uniform mat4 trans;

void main()
{                           
   gl_Position = trans * vec4(pos.x, pos.y, pos.z, 1.0);
   color = inColor;
}                           
