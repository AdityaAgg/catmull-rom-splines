#version 330

layout(location = 0) in vec3 position;
in vec2 texCoord;
in vec3 normal;
out vec2 tc;
out vec4 col;
out vec3 viewNormal;
out vec3 viewPosition;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;
void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
 
    
    
    vec4 viewPosition4 = modelViewMatrix * vec4(position, 1.0f);
    viewPosition = viewPosition4.xyz;
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
   viewNormal = normalize((normalMatrix*vec4((vec4(normal, 0.0)*modelViewMatrix).xyz, 0.0f)).xyz);
  
    tc = texCoord;

}

