#include "Visualizer.hpp"

#include "Renderer.hpp"

#include "glfw\glfw3.h"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "rtmidi\RtMidi.h"

#include <fstream>
#include <filesystem>

Settings Visualizer::settings{};
std::vector<Mapping> Visualizer::mappings;
Window Visualizer::settingsWindow;
Window Visualizer::visualizerWindow;
GLFWmonitor* Visualizer::monitor = nullptr;
RtMidiIn* Visualizer::midi = nullptr;
bool Visualizer::configureMode = false;

bool Visualizer::Initialize()
{
	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit()) { return false; }

	monitor = glfwGetPrimaryMonitor();

	I32 x = 0;
	I32 y = 0;
	I32 width = 0;
	I32 height = 0;
	glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	if (std::filesystem::exists("settings.cfg"))
	{
		std::ifstream configFile("settings.cfg", std::ios::binary);

		configFile.read((char*)&settings, sizeof(Settings));
	}
	else
	{
		//TODO: default visualizer size/position based on monitor
	}

	WindowConfig config{};
	config.name = "Drum Visualizer Settings";
	config.x = settings.settingWindowX;
	config.y = settings.settingWindowY;
	config.width = settings.settingWindowWidth;
	config.height = settings.settingWindowHeight;

	settingsWindow.Create(config);
	glfwSetKeyCallback(settingsWindow, KeyCallback);

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

	visualizerWindow.Create(config);
	glfwSetKeyCallback(visualizerWindow, KeyCallback);

	midi = new RtMidiIn();

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
	midi->setCallback(MidiCallback, nullptr);
	midi->openPort(0, name);
	midi->ignoreTypes(false, false, false);

	//TODO: load Documents\Clone Hero\MIDI Profiles\[name].yaml for midi config

	if (!Renderer::Initialize()) { return false; }

	F32 previousTime = glfwGetTime();
	F32 deltaTime = 0.0f;

	while (!glfwWindowShouldClose(settingsWindow))
	{
		deltaTime = glfwGetTime() - previousTime;
		previousTime = glfwGetTime();

		Renderer::Update(deltaTime, settingsWindow, visualizerWindow);

		glfwPollEvents();
	}

	return true;
}

void Visualizer::Shutdown()
{
	Renderer::Shutdown();

	delete midi;

	settingsWindow.Destroy();
	visualizerWindow.Destroy();
	glfwTerminate();
}

void Visualizer::MidiCallback(F64 deltatime, std::vector<U8>* message, void* userData)
{
	U32 byteCount = message->size();

	if (byteCount > 0 && message->at(0) == 144)
	{
		//Debug message
		for (U32 i = 0; i < byteCount; ++i) { std::cout << "Byte " << i << " = " << (I32)message->at(i) << ", "; }
		std::cout << "stamp = " << deltatime << std::endl;

		for (Mapping& mapping : mappings)
		{
			if (message->at(1) == mapping.midiValue)
			{
				if (message->at(2) >= mapping.velocityThreshold) //TODO: check overhit threshold
				{
					F32 dynamic = message->at(2) < settings.dynamicThreshold ? 0.5f : 1.0f;

					switch (mapping.type)
					{
						//TODO: use scroll direction
					case NoteType::Snare:	{ Renderer::SpawnNote({ -0.2f, 1.0f }, settings.snareColor * dynamic); } break;
					case NoteType::Tom1:	{ Renderer::SpawnNote({ -0.1f, 1.0f }, settings.tom1Color * dynamic); } break;
					case NoteType::Tom2:	{ Renderer::SpawnNote({  0.0f, 1.0f }, settings.tom2Color * dynamic); } break;
					case NoteType::Tom3:	{ Renderer::SpawnNote({  0.1f, 1.0f }, settings.tom3Color * dynamic); } break;
					case NoteType::Cymbal1: { Renderer::SpawnNote({ -0.1f, 1.0f }, settings.cymbal1Color * dynamic); } break;
					case NoteType::Cymbal2: { Renderer::SpawnNote({  0.0f, 1.0f }, settings.cymbal2Color * dynamic); } break;
					case NoteType::Cymbal3: { Renderer::SpawnNote({  0.1f, 1.0f }, settings.cymbal3Color * dynamic); } break;
					case NoteType::Kick:	{ Renderer::SpawnNote({  0.2f, 1.0f }, settings.kickColor * dynamic); } break;
					}
				}

				break;
			}
		}
	}
}

void Visualizer::KeyCallback(GLFWwindow* window, I32 key, I32 scancode, I32 action, I32 mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		configureMode = !configureMode;

		visualizerWindow.SetMenu(configureMode);
		visualizerWindow.SetInteractable(configureMode);
	}
}

void Visualizer::ErrorCallback(I32 error, const C8* description)
{
	std::cout << description << std::endl;
}
