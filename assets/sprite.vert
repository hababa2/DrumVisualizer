#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

layout (location = 2) in vec3 offset;
layout (location = 3) in vec2 scale;
layout (location = 4) in vec2 texcoordOffset;
layout (location = 5) in vec2 texcoordScale;
layout (location = 6) in vec3 color;
layout (location = 7) in uint textureIndex;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outTexcoord;
layout (location = 2) out flat uint outTextureIndex;

void main()
{
    gl_Position = vec4(vec2(position.x * scale.x, position.y * scale.y) + offset.xy, offset.z, 1.0);
    outColor = color;
    outTexcoord = vec2(texcoord.x * texcoordScale.x, texcoord.y * texcoordScale.y) + texcoordOffset.xy;
    outTextureIndex = textureIndex;
}