#version 330 core

out vec3 color;

void main()
{
   color = vec3(0.05, 0.5, 0.6) * gl_FragCoord.z / gl_FragCoord.w / 2.0 * gl_FragCoord.z / gl_FragCoord.w / 2.0;
}