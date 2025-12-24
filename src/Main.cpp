#include "Defines.hpp"

#include "Window.hpp"
#include "Buffer.hpp"

#include "glad\glad.h"
#include "glfw\glfw3.h"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "rtmidi\RtMidi.h"

#include <fstream>

Window* settingsWindow;
Window* visualizer;
GLFWmonitor* monitor = nullptr;
bool configure = false;

enum class ScrollDirection
{
	Up,
	Down,
	Left,
	Right
};

enum class NoteType
{
	Snare,
	Tom1,
	Tom2,
	Tom3,
	Cymbal1,
	Cymbal2,
	Cymbal3,
	Kick
};

struct Mapping
{
	NoteType type;
	I32 midiValue;
	I32 velocity;
	I32 overhitThreshold;
	F64 lastHit;
};

struct Settings
{
	I32 settingWindowX{ 100 };
	I32 settingWindowY{ 100 };
	I32 settingWindowWidth{ 600 };
	I32 settingWindowHeight{ 800 };

	I32 visualizerWindowX{ 700 };
	I32 visualizerWindowY{ 100 };
	I32 visualizerWindowWidth{ 600 };
	I32 visualizerWindowHeight{ 800 };

	F32 dynamicThreshold{ 60 };

	ScrollDirection scrollDirection{ ScrollDirection::Down };
	Vector3 snareColor = { 1.0f, 0.0f, 0.0f };
	Vector3 tom1Color = { 1.0f, 1.0f, 0.0f };
	Vector3 tom2Color = { 0.0f, 0.0f, 1.0f };
	Vector3 tom3Color = { 0.0f, 1.0f, 0.0f };
	Vector3 cymbal1Color = { 1.0f, 1.0f, 0.0f };
	Vector3 cymbal2Color = { 0.0f, 0.0f, 1.0f };
	Vector3 cymbal3Color = { 0.0f, 1.0f, 0.0f };
	Vector3 kickColor = { 1.0f, 0.65f, 0.0f };
} settings;

std::vector<Mapping> mappings;

Vector2 positions[] = {
	{  0.05f,  0.05f },
	{  0.05f, -0.05f },
	{ -0.05f, -0.05f },
	{ -0.05f,  0.05f }
};

std::vector<Vector2> offsets;
std::vector<Vector3> colors;

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

static void error(int error, const char* description)
{
	fputs(description, stderr);
}

void callback(double deltatime, std::vector<U8>* message, void* userData)
{
	if (message->at(0) == 144)
	{
		//Debug message
		U32 nBytes = message->size();
		for (U32 i = 0; i < nBytes; i++) { std::cout << "Byte " << i << " = " << (int)message->at(i) << ", "; }
		if (nBytes > 0) { std::cout << "stamp = " << deltatime << std::endl; }

		for (Mapping& mapping : mappings)
		{
			if (message->at(1) == mapping.midiValue) //TODO: check overhit threshold
			{
				F32 dynamic = message->at(2) < settings.dynamicThreshold ? 0.5f : 1.0f;

				switch (mapping.type)
				{
				case NoteType::Snare: { offsets.push_back({ -0.2f, 1.0f }); colors.push_back(settings.snareColor * dynamic); } break;
				case NoteType::Tom1: { offsets.push_back({ -0.1f, 1.0f }); colors.push_back(settings.tom1Color * dynamic); } break;
				case NoteType::Tom2: { offsets.push_back({ 0.0f, 1.0f }); colors.push_back(settings.tom2Color * dynamic); } break;
				case NoteType::Tom3: { offsets.push_back({ 0.1f, 1.0f }); colors.push_back(settings.tom3Color * dynamic); } break;
				case NoteType::Cymbal1: { offsets.push_back({ -0.1f, 1.0f }); colors.push_back(settings.cymbal1Color * dynamic); } break;
				case NoteType::Cymbal2: { offsets.push_back({ 0.0f, 1.0f }); colors.push_back(settings.cymbal2Color * dynamic); } break;
				case NoteType::Cymbal3: { offsets.push_back({ 0.1f, 1.0f }); colors.push_back(settings.cymbal3Color * dynamic); } break;
				case NoteType::Kick: { offsets.push_back({ 0.2f, 1.0f }); colors.push_back(settings.kickColor * dynamic); } break;
				}

				break;
			}
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

	std::ifstream configFile("settings.cfg", std::ios::binary);

	//if (configFile.is_open())
	//{
	//	configFile.read((char*)&settings, sizeof(Settings));
	//}

	WindowConfig config{};
	config.name = "Drum Visualizer Settings";
	config.x = settings.settingWindowX;
	config.y = settings.settingWindowY;
	config.width = settings.settingWindowWidth;
	config.height = settings.settingWindowHeight;

	settingsWindow = new Window(config);
	glfwSetKeyCallback(*settingsWindow, keyCallback);

	config.name = "Drum Visualizer";
	config.transparent = true;
	config.floating = true;
	config.interactable = false;
	config.menu = false;
	config.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	config.x = settings.visualizerWindowX;
	config.y = settings.visualizerWindowY;
	config.width = settings.visualizerWindowWidth;
	config.height = settings.visualizerWindowHeight;

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

	mappings.push_back({ NoteType::Snare, 44, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Tom1, 45, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Tom2, 46, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Tom3, 47, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Cymbal1, 48, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Cymbal2, 49, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Cymbal3, 50, 10, 0, 0.0f });
	mappings.push_back({ NoteType::Kick, 51, 10, 0, 0.0f });

	std::string name = midi->getPortName(0);
	name = name.substr(0, name.size() - 2);
	midi->setCallback(callback, nullptr);
	midi->openPort(0, name);
	midi->ignoreTypes(false, false, false);

	//TODO: load Documents\Clone Hero\MIDI Profiles\[name].yaml for midi config

	F32 previousTime = glfwGetTime();
	F32 deltaTime = 0.0f;

	while (!glfwWindowShouldClose(*settingsWindow))
	{
		deltaTime = glfwGetTime() - previousTime;
		previousTime = glfwGetTime();

		for (Vector2& offset : offsets)
		{
			offset.y -= deltaTime * 1.0f;
		}

		offsetsBuffer.Flush(offsets.data(), offsets.capacity() * sizeof(Vector2));
		colorsBuffer.Flush(colors.data(), colors.capacity() * sizeof(Vector3));

		settingsWindow->Update();
		settingsWindow->Render();

		visualizer->Update();

		glUseProgram(shaderProgram);
		glBindVertexArray(vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, offsets.size());

		glBindVertexArray(0);

		visualizer->Render();
		glfwPollEvents();
	}

	delete midi;

	delete settingsWindow;
	delete visualizer;
	glDeleteVertexArrays(1, &vao);
	glfwTerminate();

	return 0;
}
