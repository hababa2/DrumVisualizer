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
"layout (location = 4) in float textureId;\n" //Passing an integer into glsl doesn't work for some reason
"out vec3 fragColor;\n"
"out vec2 fragTexCoords;\n"
"out flat uint fragTextureId;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position + offset, 0.0, 1.0);\n"
"   fragColor = color;\n"
"   fragTexCoords = texCoords;\n"
"   fragTextureId = uint(textureId);\n"
"}\0";

const char* fragmentShaderSource =
"#version 460 core\n"
"out vec4 FragColor;\n"
"in vec3 fragColor;\n"
"in vec2 fragTexCoords;\n"
"in flat uint fragTextureId;\n"
"uniform sampler2DArray textures;\n"
"void main()\n"
"{\n"
"   FragColor = texture(textures, vec3(fragTexCoords.xy, fragTextureId)) * vec4(fragColor, 1.0);\n"
"}\0";

U32 Renderer::vao;
U32 Renderer::textureArray;
U32 Renderer::shaderProgram;
Buffer Renderer::positionBuffer;
Buffer Renderer::texCoordsBuffer;
Buffer Renderer::offsetsBuffer;
Buffer Renderer::colorsBuffer;
Buffer Renderer::textureIdsBuffer;
U32 Renderer::nextIndex = 0;
std::vector<Vector2> Renderer::offsets;
std::vector<Vector3> Renderer::colors;
std::vector<F32> Renderer::textureIds;

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
	textureIdsBuffer.Create(4, DataType::FLOAT, textureIds.data(), static_cast<U32>(textureIds.capacity() * sizeof(F32)), true);

	glGenTextures(1, &textureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 256, 256, 3);

	LoadTexture("assets/square.png");
	LoadTexture("assets/circle.png");
	LoadTexture("assets/triangle.png");

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
}

void Renderer::Update(Vector2 velocity, Window& settingsWindow, Window& visualizerWindow)
{
	for (Vector2& offset : offsets)
	{
		offset += velocity;
	}

	offsetsBuffer.Flush(offsets.data(), static_cast<U32>(offsets.capacity() * sizeof(Vector2)));
	colorsBuffer.Flush(colors.data(), static_cast<U32>(colors.capacity() * sizeof(Vector3)));
	textureIdsBuffer.Flush(textureIds.data(), static_cast<U32>(textureIds.capacity() * sizeof(F32)));

	settingsWindow.Update();
	settingsWindow.Render();

	visualizerWindow.Update();

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "textures"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, static_cast<I32>(offsets.size()));

	glBindVertexArray(0);

	visualizerWindow.Render();
}

void Renderer::SpawnNote(const Vector2& position, const Vector3& color, U32 textureId)
{
	offsets[nextIndex] = position;
	colors[nextIndex] = color;
	textureIds[nextIndex] = (F32)textureId;

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

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, id++, x, y, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	return true;
}