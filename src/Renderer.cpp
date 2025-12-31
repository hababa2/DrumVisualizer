#include "Renderer.hpp"

#include "glad\glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Vector2 positions[] = {
	{  0.05f,  0.025f },
	{  0.05f, -0.025f },
	{ -0.05f, -0.025f },
	{ -0.05f,  0.025f }
};

Vector2 texCoords[] = {
	{ 1.0f, 1.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f }
};

U32 indices[] = {
	0, 1, 3,
	1, 2, 3
};

const char* vertexShaderSource =
"#version 460 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 texCoords;\n"
"layout (location = 2) in vec2 offset;\n"
"layout (location = 3) in vec3 color;\n"
"layout (location = 4) in uint textureId;\n"
"out vec3 fragColor;\n"
"out vec2 fragTexCoords;\n"
"out flat uint fragTextureId;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position + offset, 0.0, 1.0);\n"
"   fragColor = color;\n"
"   fragTexCoords = texCoords;\n"
"   fragTextureId = textureId;\n"
"}\0";

const char* fragmentShaderSource =
"#version 460 core\n"
"#extension GL_ARB_bindless_texture : require\n"
"layout(binding=0, std430) readonly buffer textureHandles {\n"
"    sampler2D textures[];\n"
"};\n"
"out vec4 FragColor;\n"
"in vec3 fragColor;\n"
"in vec2 fragTexCoords;\n"
"in flat uint fragTextureId;\n"
"void main()\n"
"{\n"
"	sampler2D tex = textures[fragTextureId];\n"
"   FragColor = texture(tex, fragTexCoords) * vec4(fragColor, 1.0);\n"
"}\0";

U32 Renderer::vao;
U32 Renderer::textureBuffer;
U32 Renderer::shaderProgram;
Buffer Renderer::positionBuffer;
Buffer Renderer::texCoordsBuffer;
Buffer Renderer::offsetsBuffer;
Buffer Renderer::colorsBuffer;
Buffer Renderer::textureIdsBuffer;
U32 Renderer::nextIndex = 0;
std::vector<Vector2> Renderer::offsets;
std::vector<Vector3> Renderer::colors;
std::vector<U32> Renderer::textureIds;
std::vector<U64> Renderer::textureHandles;

bool Renderer::Initialize()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	stbi_set_flip_vertically_on_load(true);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	positionBuffer.Create(0, DataType::VECTOR2, positions, static_cast<U32>(CountOf(positions) * sizeof(Vector2)), false);
	texCoordsBuffer.Create(1, DataType::VECTOR2, texCoords, static_cast<U32>(CountOf(texCoords) * sizeof(Vector2)), false);
	offsetsBuffer.Create(2, DataType::VECTOR2, offsets.data(), static_cast<U32>(offsets.capacity() * sizeof(Vector2)), true);
	colorsBuffer.Create(3, DataType::VECTOR3, colors.data(), static_cast<U32>(colors.capacity() * sizeof(Vector3)), true);
	textureIdsBuffer.Create(4, DataType::UINT, textureIds.data(), static_cast<U32>(textureIds.capacity() * sizeof(U32)), true);

	LoadTexture("assets/square.png");
	LoadTexture("assets/circle.png");
	LoadTexture("assets/triangle.png");

	glCreateBuffers(1, &textureBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, textureHandles.size() * sizeof(U64), textureHandles.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureBuffer);

	I32 success;
	C8 infoLog[512];

	U32 vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Vertex shader compilation failed: " << infoLog << std::endl;
		return false;
	}

	U32 fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "Fragment shader compilation failed: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		return false;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "Shader program linking failed: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	offsets.resize(MaxNotes, { -100.0f, -100.0f });
	colors.resize(MaxNotes, { 0.0f, 0.0f, 0.0f });
	textureIds.resize(MaxNotes, 0);

	return true;
}

void Renderer::Shutdown()
{
	positionBuffer.Destroy();
	texCoordsBuffer.Destroy();
	offsetsBuffer.Destroy();
	colorsBuffer.Destroy();
	textureIdsBuffer.Destroy();

	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);

	//TODO: delete textures
}

void Renderer::Update(Vector2 velocity, Window& settingsWindow, Window& visualizerWindow)
{
	for (Vector2& offset : offsets)
	{
		offset += velocity;
	}

	offsetsBuffer.Flush(offsets.data(), static_cast<U32>(offsets.capacity() * sizeof(Vector2)));
	colorsBuffer.Flush(colors.data(), static_cast<U32>(colors.capacity() * sizeof(Vector3)));
	textureIdsBuffer.Flush(textureIds.data(), static_cast<U32>(textureIds.capacity() * sizeof(U32)));

	settingsWindow.Update();
	settingsWindow.Render();

	visualizerWindow.Update();

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "textures"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, static_cast<I32>(offsets.size()));

	glBindVertexArray(0);

	visualizerWindow.Render();
}

void Renderer::SpawnNote(const Vector2& position, const Vector3& color, U32 textureId)
{
	offsets[nextIndex] = position;
	colors[nextIndex] = color;
	textureIds[nextIndex] = textureId;

	++nextIndex %= MaxNotes;
}

void Renderer::ClearNotes()
{
	for (Vector2& offset : offsets)
	{
		offset = { -100.0f, -100.0f };
	}
}

bool Renderer::LoadTexture(const std::string& path)
{
	static I32 id = 0;

	I32 x, y, comp;
	U8* data = stbi_load(path.c_str(), &x, &y, &comp, 4);
	if (!data)
	{
		std::cout << "Failed To Load Texture: " << path << std::endl;
		return false;
	}

	U32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, x, y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	stbi_image_free(data);

	U64 textureHandle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(textureHandle);
	textureHandles.push_back(textureHandle);

	textureIds.push_back(id++);

	return true;
}