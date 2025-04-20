#version 450
layout(location = 0) in vec3 fragColor;
layout(set=1,binding=0) uniform fraUBO {
    vec4 color;
}ubo; 

layout(location = 0) out vec4 outColor;


void main() {
    outColor = vec4(ubo.color); 
}