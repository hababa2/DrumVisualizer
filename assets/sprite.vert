#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec2 offset;
layout (location = 3) in vec3 color;
layout (location = 4) in uint textureIndex;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outTexcoord;
layout (location = 2) out flat uint outTextureIndex;

void main()
{
    gl_Position = vec4(position + offset, 0.0, 1.0);
    outColor = color;
    outTexcoord = texcoord;
    outTextureIndex = textureIndex;
}