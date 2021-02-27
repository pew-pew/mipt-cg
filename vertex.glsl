#version 330 core

layout (location = 0) in vec3 pos;
uniform mat4 trans;

void main() {
    gl_Position = trans * vec4(pos.x, pos.y, pos.z, 1.0);
    gl_Position.xyz /= gl_Position.w;
    gl_Position.w = 1;
    gl_Position.z *= -1;
}


// attribute vec4 vPosition;   
// void main()                 
// {                           
//    gl_Position = vPosition; 
// }                           
