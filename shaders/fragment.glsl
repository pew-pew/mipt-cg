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
   gl_FragColor = vec4((alpha != 1.0 ? vec3(0) : color * log(gl_FragCoord.z / gl_FragCoord.w)), alpha);
   //gl_FragColor = vec4((alpha != 1.0 ? vec3(0) : color * gl_FragColor.z), alpha);
}                                          
