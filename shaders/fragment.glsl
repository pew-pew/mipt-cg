// #version 330 core

// in vec3 color;
// out vec4 FragColor;
// uniform float alpha;

// void main() {
//    // FragColor = vec4(gl_FragCoord.z, 0, 0, 1);
//    FragColor = vec4((alpha != 1 ? vec3(0) : color), alpha);
// }


#version 100

precision mediump float;                   

varying vec3 color;
uniform float alpha;

void main()
{
   gl_FragColor = vec4(color, alpha);
}                                          
