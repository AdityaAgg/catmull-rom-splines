#version 330

in vec2 tc; //input texture coordinates (computed by interpolator)
out vec4 c;


uniform sampler2D textureImage;

//lighting
in vec3 viewNormal;
in vec3 viewPosition;
uniform vec3 viewLightDirection;


uniform vec4 matKa;
uniform vec4 matKd;
uniform vec4 matKs;
uniform float matKsExp;

void main()
{
    
    // camera is at (0,0,0) after the modelview transformation
    vec3 eyedir = normalize(vec3(0, 0, 0) - viewPosition);
    // reflected light direction
    vec3 reflectDir = -reflect(viewLightDirection, viewNormal);
    // Phong lighting
    float kd = max(dot(viewLightDirection, viewNormal), 0.0f);
    float ks = max(dot(reflectDir, eyedir), 0.0f);
    // compute the final color
    c = matKa   + matKd * kd  +
    matKs * pow(ks, matKsExp);
    c+= texture(textureImage, tc);
    
}

