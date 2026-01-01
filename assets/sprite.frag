#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) readonly buffer textureHandles {
    sampler2D textures[];
};

layout (location = 0) in vec3 color;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in flat uint textureIndex;

out vec4 outColor;

void main()
{
    outColor = texture(textures[textureIndex], texcoord) * vec4(color, 1.0);
    if(outColor.a < 0.1) { discard; }
}