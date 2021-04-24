#version 100

precision mediump float;                   

varying vec2 tex_coord;
varying vec3 to_light, to_camera, normal;

uniform sampler2D tex;
uniform float ambientK;

varying float unused_hack;

void main()
{
    vec3 reflectance = texture2D(tex, tex_coord).xyz;

    float lightPower = min(1.0 / dot(to_light, to_light), 1.0);

    float diffuseK = max(0.0, dot(normalize(to_light), normalize(normal)));
    float specularK = max(0.0, dot(normalize(to_camera), reflect(-normalize(to_light), normalize(normal))));

    vec3 lightColor = vec3(0.9, 0.7, 0);
    vec3 incoming = lightColor * (lightPower * (pow(specularK, 5.0) * 0.2  + diffuseK) + ambientK);

    gl_FragColor = vec4(reflectance * incoming, 1.0) + clamp(unused_hack, 0., 1e-10);
}