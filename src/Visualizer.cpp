#include "Visualizer.hpp"

#include "Renderer.hpp"

#include "glfw\glfw3.h"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "rtmidi\RtMidi.h"

#include <fstream>
#include <filesystem>
#include <codecvt>

#ifdef DV_PLATFORM_WINDOWS
#include "shlobj_core.h"
#endif

Settings Visualizer::settings{};
std::vector<Profile> Visualizer::profiles;
std::vector<Mapping> Visualizer::mappings;
Window Visualizer::settingsWindow;
Window Visualizer::visualizerWindow;
GLFWmonitor* Visualizer::monitor = nullptr;
RtMidiIn* Visualizer::midi = nullptr;
F64 Visualizer::lastInput = 0.0;
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
		std::ifstream configFile("settings.cfg", std::ios::binary | std::ios::in);

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
	std::wstring cloneHeroFolder = GetCloneHeroFolder();
	LoadProfiles(cloneHeroFolder);
	if (settings.profileId != U32_MAX)
	{
		settings.dynamicThreshold = profiles[settings.profileId].dynamicThreshold;
		settings.leftyFlip = profiles[settings.profileId].leftyFlip;
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		LoadColors(cloneHeroFolder + L"Custom\\Colors\\" + converter.from_bytes(profiles[settings.profileId].colorProfile) + L".ini");
	}

	if (!Renderer::Initialize()) { return false; }

	F64 previousTime = glfwGetTime();
	F64 deltaTime = 0.0;

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
	WindowConfig config = settingsWindow.Config();

	settings.settingWindowX = config.x;
	settings.settingWindowY = config.y;
	settings.settingWindowWidth = config.width;
	settings.settingWindowHeight = config.height;

	config = visualizerWindow.Config();

	settings.visualizerWindowX = config.x;
	settings.visualizerWindowY = config.y;
	settings.visualizerWindowWidth = config.width;
	settings.visualizerWindowHeight = config.height;

	std::ofstream configFile("settings.cfg", std::ios::binary | std::ios::out);

	configFile.write((char*)&settings, sizeof(Settings));

	Renderer::Shutdown();

	delete midi;

	settingsWindow.Destroy();
	visualizerWindow.Destroy();
	glfwTerminate();
}

std::wstring Visualizer::GetCloneHeroFolder()
{
#ifdef DV_PLATFORM_WINDOWS
	PWSTR path = nullptr;
	HRESULT hres = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path);

	std::wstring documents(path);

	documents.append(L"\\Clone Hero\\");

	return documents;
#endif
}

void Visualizer::LoadProfiles(const std::wstring& cloneHeroPath)
{
	std::ifstream t(cloneHeroPath + L"profiles.ini");
	std::stringstream buffer;
	buffer << t.rdbuf();

	std::string data = buffer.str();

	U64 i = 0;
	U64 end = 0;
	while ((i = data.find('[', i)) != std::string::npos)
	{
		Profile profile{};

		i = data.find("dynamics_threshold", i);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.dynamicThreshold = std::stoi(data.substr(i, end - i));

		i = data.find("color_profile_name", end);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.colorProfile = data.substr(i, end - i);

		i = data.find("player_name", end);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.name = data.substr(i, end - i);

		i = data.find("lefty_flip", end);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.leftyFlip = data.substr(i, end - i) == "1";

		profiles.push_back(profile);
	}

	if (profiles.empty())
	{
		profiles.push_back({ "Guest", "DefaultColors", 100, false });
	}
	else if (settings.profileId == U32_MAX)
	{
		settings.profileId = 0;
	}
}

void Visualizer::LoadColors(const std::wstring& path)
{
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();

	std::string data = buffer.str();

	U64 i = 0;
	U64 end = 0;

	i = data.find("note_kick ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	std::string hex = data.substr(i, end - i);
	settings.kickColor = HexToRBG(hex);

	i = data.find("cym_blue ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.cymbal2Color = HexToRBG(hex);

	i = data.find("cym_yellow ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.cymbal1Color = HexToRBG(hex);

	i = data.find("cym_green ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.cymbal3Color = HexToRBG(hex);

	i = data.find("tom_blue ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.tom2Color = HexToRBG(hex);

	i = data.find("tom_yellow ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.tom1Color = HexToRBG(hex);

	i = data.find("tom_red ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.snareColor = HexToRBG(hex);

	i = data.find("tom_green ", i);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	settings.tom3Color = HexToRBG(hex);
}

Vector3 Visualizer::HexToRBG(const std::string& hex)
{
	static constexpr U64 RMask = 0xFF0000;
	static constexpr U64 GMask = 0x00FF00;
	static constexpr U64 BMask = 0x0000FF;

	U64 rgb = std::stoull(hex, nullptr, 16);

	return { ((rgb & RMask) >> 16) / 255.0f, ((rgb & GMask) >> 8) / 255.0f, (rgb & BMask) / 255.0f };
}

void Visualizer::MidiCallback(F64 deltatime, std::vector<U8>* message, void* userData)
{
	U64 byteCount = message->size();

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
					case NoteType::Snare: { Renderer::SpawnNote({ -0.2f, 1.0f }, settings.snareColor * dynamic); } break;
					case NoteType::Tom1: { Renderer::SpawnNote({ -0.1f, 1.0f }, settings.tom1Color * dynamic); } break;
					case NoteType::Tom2: { Renderer::SpawnNote({ 0.0f, 1.0f }, settings.tom2Color * dynamic); } break;
					case NoteType::Tom3: { Renderer::SpawnNote({ 0.1f, 1.0f }, settings.tom3Color * dynamic); } break;
					case NoteType::Cymbal1: { Renderer::SpawnNote({ -0.1f, 1.0f }, settings.cymbal1Color * dynamic); } break;
					case NoteType::Cymbal2: { Renderer::SpawnNote({ 0.0f, 1.0f }, settings.cymbal2Color * dynamic); } break;
					case NoteType::Cymbal3: { Renderer::SpawnNote({ 0.1f, 1.0f }, settings.cymbal3Color * dynamic); } break;
					case NoteType::Kick: { Renderer::SpawnNote({ 0.2f, 1.0f }, settings.kickColor * dynamic); } break;
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
