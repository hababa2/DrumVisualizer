#include "Renderer.hpp"

#include "glad\glad.h"
#include "glfw\glfw3.h"

Vector2 positions[] = {
	{  0.05f,  0.05f },
	{  0.05f, -0.05f },
	{ -0.05f, -0.05f },
	{ -0.05f,  0.05f }
};

U32 indices[] = {
	0, 1, 3,
	1, 2, 3
};

const char* vertexShaderSource =
"#version 460 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 offset;\n"
"layout (location = 2) in vec3 color;\n"
"out vec3 fragColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position + offset, 0.0, 1.0);\n"
"   fragColor = color;\n"
"}\0";

const char* fragmentShaderSource =
"#version 460 core\n"
"out vec4 FragColor;\n"
"in vec3 fragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(fragColor, 1.0);\n"
"}\0";


U32 Renderer::vao;
U32 Renderer::shaderProgram;
Buffer Renderer::positionBuffer;
Buffer Renderer::offsetsBuffer;
Buffer Renderer::colorsBuffer;
std::vector<Vector2> Renderer::offsets;
std::vector<Vector3> Renderer::colors;

bool Renderer::Initialize()
{
	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	positionBuffer.Create(0, DataType::VECTOR2, positions, CountOf(positions) * sizeof(Vector2), false);
	offsetsBuffer.Create(1, DataType::VECTOR2, offsets.data(), offsets.capacity() * sizeof(Vector2), true);
	colorsBuffer.Create(2, DataType::VECTOR3, colors.data(), colors.capacity() * sizeof(Vector3), true);

	U32 vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	U32 fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	offsets.reserve(1000);
	colors.reserve(1000);

	return true;
}

void Renderer::Shutdown()
{
	positionBuffer.Destroy();
	offsetsBuffer.Destroy();
	colorsBuffer.Destroy();

	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);
}

void Renderer::Update(F32 deltaTime, Window& settingsWindow, Window& visualizerWindow)
{
	for (Vector2& offset : offsets)
	{
		offset.y -= deltaTime * 1.0f;
	}

	offsetsBuffer.Flush(offsets.data(), offsets.capacity() * sizeof(Vector2));
	colorsBuffer.Flush(colors.data(), colors.capacity() * sizeof(Vector3));

	settingsWindow.Update();
	settingsWindow.Render();

	visualizerWindow.Update();

	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, offsets.size());

	glBindVertexArray(0);

	visualizerWindow.Render();
}

void Renderer::SpawnNote(const Vector2& position, const Vector3& color)
{
	offsets.push_back(position);
	colors.push_back(color);
}