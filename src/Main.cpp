#include "Defines.hpp"

#include "Window.hpp"
#include "Buffer.hpp"

#include "glad\glad.h"
#include "glfw\glfw3.h"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "rtmidi\RtMidi.h"

Window* settings;
Window* visualizer;
GLFWmonitor* monitor = nullptr;
bool configure = false;

Vector2 positions[] = {
	{  0.05f,  0.05f },
	{  0.05f, -0.05f },
	{ -0.05f, -0.05f },
	{ -0.05f,  0.05f }
};

std::vector<Vector2> offsets;
std::vector<Vector3> colors;

const Vector3 RED = { 1.0f, 0.0f, 0.0f };
const Vector3 YELLOW = { 1.0f, 1.0f, 0.0f };
const Vector3 BLUE = { 0.0f, 0.0f, 1.0f };
const Vector3 GREEN = { 0.0f, 1.0f, 0.0f };
const Vector3 ORANGE = { 1.0f, 0.65f, 0.0f };

U32 indices[] = {
	0, 1, 3,
	1, 2, 3
};

const char* vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 offset;\n"
"layout (location = 2) in vec3 color;\n"
"out vec3 fragColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position + offset, 0.0, 1.0);\n"
"   fragColor = color;\n"
"}\0";

const char* fragmentShaderSource = "#version 460 core\n"
"out vec4 FragColor;\n"
"in vec3 fragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(fragColor, 1.0f);\n"
"}\0";

static void error(int error, const char* description)
{
	fputs(description, stderr);
}

void callback(double deltatime, std::vector<unsigned char>* message, void* userData)
{
	U32 nBytes = message->size();
	for (U32 i = 0; i < nBytes; i++) { std::cout << "Byte " << i << " = " << (int)message->at(i) << ", "; }
	if (nBytes > 0) { std::cout << "stamp = " << deltatime << std::endl; }

	if (message->at(0) == 144)
	{
		switch (message->at(1))
		{
		case 44: { offsets.push_back({ -0.2f, 1.0f }); colors.push_back(RED);    } break;
		case 45: { offsets.push_back({ -0.1f, 1.0f }); colors.push_back(YELLOW); } break;
		case 46: { offsets.push_back({  0.0f, 1.0f }); colors.push_back(BLUE);   } break;
		case 47: { offsets.push_back({  0.1f, 1.0f }); colors.push_back(GREEN);  } break;
		case 48: { offsets.push_back({ -0.1f, 1.0f }); colors.push_back(YELLOW); } break;
		case 49: { offsets.push_back({  0.0f, 1.0f }); colors.push_back(BLUE);   } break;
		case 50: { offsets.push_back({  0.1f, 1.0f }); colors.push_back(GREEN);  } break;
		case 51: { offsets.push_back({  0.2f, 1.0f }); colors.push_back(ORANGE); } break;
		}
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		configure = !configure;

		visualizer->SetMenu(configure);
		visualizer->SetInteractable(configure);
	}
}

int main()
{
	glfwSetErrorCallback(error);

	if (!glfwInit()) { return -1; }

	monitor = glfwGetPrimaryMonitor();

	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	WindowConfig config{};
	config.name = "Drum Visualizer Settings";

	settings = new Window(config);
	glfwSetKeyCallback(*settings, keyCallback);

	config.name = "Drum Visualizer";
	config.transparent = true;
	config.floating = true;
	config.interactable = false;
	config.menu = false;
	config.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	visualizer = new Window(config);
	glfwSetKeyCallback(*visualizer, keyCallback);

	glEnable(GL_DEPTH_TEST);

	offsets.reserve(1000);
	colors.reserve(1000);

	U32 vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	Buffer positionBuffer(0, DataType::VECTOR2, positions, CountOf(positions) * sizeof(Vector2), false);
	Buffer offsetsBuffer(1, DataType::VECTOR2, offsets.data(), offsets.capacity() * sizeof(Vector2), true);
	Buffer colorsBuffer(2, DataType::VECTOR3, colors.data(), colors.capacity() * sizeof(Vector3), true);

	U32 vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	U32 fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	U32 shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	RtMidiIn* midi = new RtMidiIn();

	midi->setCallback(callback, nullptr);
	midi->openPort(0, "Midi");
	midi->ignoreTypes(false, false, false);

	F32 previousTime = glfwGetTime();
	F32 deltaTime = 0.0f;

	while (!glfwWindowShouldClose(*settings))
	{
		deltaTime = glfwGetTime() - previousTime;
		previousTime = glfwGetTime();

		for (Vector2& offset : offsets)
		{
			offset.y -= deltaTime * 1.0f;
		}

		offsetsBuffer.Flush(offsets.data(), offsets.capacity() * sizeof(Vector2));
		colorsBuffer.Flush(colors.data(), colors.capacity() * sizeof(Vector3));

		settings->Update();
		settings->Render();

		visualizer->Update();

		glUseProgram(shaderProgram);
		glBindVertexArray(vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, offsets.size());

		glBindVertexArray(0);

		visualizer->Render();
		glfwPollEvents();
	}

	delete midi;

	delete settings;
	delete visualizer;
	glDeleteVertexArrays(1, &vao);
	glfwTerminate();

	return 0;
}
