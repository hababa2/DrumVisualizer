#include "Visualizer.hpp"

#include "Renderer.hpp"

#include "glfw\glfw3.h"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "rtmidi\RtMidi.h"

#include <codecvt>
#include <fstream>
#include <mutex>

#ifdef DV_PLATFORM_WINDOWS
#include "shlobj_core.h"
#include <objbase.h>
#endif

namespace {
	I32 SafeStoi(const std::string& str, I32 defaultValue = 0)
	{
		try { return std::stoi(str); }
		catch (...) { return defaultValue; }
	}

	F32 SafeStof(const std::string& str, F32 defaultValue = 0.0f)
	{
		try { return std::stof(str); }
		catch (...) { return defaultValue; }
	}

	F64 SafeStod(const std::string& str, F64 defaultValue = 0.0)
	{
		try { return std::stod(str); }
		catch (...) { return defaultValue; }
	}

	U64 SafeStoull(const std::string& str, int base = 10, U64 defaultValue = 0)
	{
		try { return std::stoull(str, nullptr, base); }
		catch (...) { return defaultValue; }
	}
} // namespace

Settings Visualizer::settings{};
Layout Visualizer::layout{};
ColorProfile Visualizer::colorProfile{};
std::vector<Profile> Visualizer::profiles;
std::vector<Mapping> Visualizer::mappings;
Window Visualizer::settingsWindow;
Window Visualizer::visualizerWindow;
GLFWmonitor* Visualizer::monitor = nullptr;
RtMidiIn* Visualizer::midiIn = nullptr;
RtMidiOut* Visualizer::midiOut = nullptr;
F64 Visualizer::lastInput = 0.0;
bool Visualizer::configureMode = false;
std::mutex Visualizer::midiMutex;

bool Visualizer::Initialize()
{
#ifdef DV_DEBUG
	std::cout << "=== DrumVisualizer Debug Mode ===" << std::endl;
#endif
	if (!InitializeGlfw())
	{
		Shutdown();
		return false;
	}
	if (!InitializeWindows())
	{
		Shutdown();
		return false;
	}
	if (!InitializeCH())
	{
		Shutdown();
		return false;
	}
	if (!InitializeMidi())
	{
		Shutdown();
		return false;
	}
	if (!Renderer::Initialize())
	{
		Shutdown();
		return false;
	}
#ifdef DV_DEBUG
	std::cout << "Initialized successfully!" << std::endl;
#endif

	MainLoop();
	Shutdown();

	return true;
}

void Visualizer::Shutdown()
{
#ifdef DV_DEBUG
	std::cout << std::endl << "=== Shutting down ===" << std::endl;
#endif
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

#ifdef DV_DEBUG
	std::cout << "Saving configuration..." << std::endl;
#endif
	SaveConfig();

#ifdef DV_DEBUG
	std::cout << "Cleaning up resources..." << std::endl;
#endif
	Renderer::Shutdown();

#ifndef DV_PLATFORM_WINDOWS
	delete midiOut;
#endif
	delete midiIn;

	settingsWindow.Destroy();
	visualizerWindow.Destroy();
	glfwTerminate();
#ifdef DV_DEBUG
	std::cout << "Shutdown complete" << std::endl;
#endif
}

void Visualizer::MainLoop()
{
	F64 previousTime = glfwGetTime();
	F64 deltaTime = 0.0;

	while (!glfwWindowShouldClose(settingsWindow))
	{
		deltaTime = glfwGetTime() - previousTime;
		previousTime = glfwGetTime();

		Vector2 velocity{ 0.0f, 0.0f };

		switch (settings.scrollDirection)
		{
		case ScrollDirection::Up: {
			velocity = Vector2{ 0.0f, 1.0f } *
				static_cast<F32>(deltaTime * settings.scrollSpeed);
		} break;
		case ScrollDirection::Down: {
			velocity = Vector2{ 0.0f, -1.0f } *
				static_cast<F32>(deltaTime * settings.scrollSpeed);
		} break;
		case ScrollDirection::Left: {
			velocity = Vector2{ -1.0f, 0.0f } *
				static_cast<F32>(deltaTime * settings.scrollSpeed);
		} break;
		case ScrollDirection::Right: {
			velocity = Vector2{ 1.0f, 0.0f } *
				static_cast<F32>(deltaTime * settings.scrollSpeed);
		} break;
		}

		Renderer::Update(velocity, settingsWindow, visualizerWindow);

		glfwPollEvents();
	}
}

bool Visualizer::InitializeGlfw()
{
	glfwSetErrorCallback(ErrorCallback);
#ifdef DV_DEBUG
	std::cout << "Initializing GLFW..." << std::endl;
#endif

	if (!glfwInit())
	{
		std::cout << "Failed To Initialize GLFW, shutting down" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	return true;
}

bool Visualizer::InitializeWindows()
{
#ifdef DV_DEBUG
	std::cout << "Initializing windows..." << std::endl;
#endif
	if (!LoadConfig())
	{
		std::cout << "No settings.cfg found, using default settings" << std::endl;
		monitor = glfwGetPrimaryMonitor();

		I32 x = 0;
		I32 y = 0;
		I32 width = 0;
		I32 height = 0;
		glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

		// TODO: default visualizer size/position based on monitor
	}

	SetScrollDirection(settings.scrollDirection);

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

	return true;
}

bool Visualizer::InitializeCH()
{
	std::wstring cloneHeroFolder = GetCloneHeroFolder();
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	LoadProfiles(cloneHeroFolder);
	if (settings.profileId != U32_MAX)
	{
		settings.dynamicThreshold = profiles[settings.profileId].dynamicThreshold;
		settings.leftyFlip = profiles[settings.profileId].leftyFlip;
		LoadColors(cloneHeroFolder + L"Custom\\Colors\\" +
			converter.from_bytes(profiles[settings.profileId].colorProfile) +
			L".ini");
	}

	LoadMidiProfile(cloneHeroFolder + L"MIDI Profiles\\loopMIDI CH.yaml");

	return true;
}

bool Visualizer::InitializeMidi()
{
#ifdef DV_DEBUG
	std::cout << "Initializing MIDI..." << std::endl;
#endif
	midiIn = new RtMidiIn();

	bool found = false;
	std::string foundPortName;
	U32 portCount = midiIn->getPortCount();
#ifdef DV_DEBUG
	std::cout << "Found " << portCount << " MIDI port(s)" << std::endl;
#endif
	for (U32 i = 0; i < portCount; ++i)
	{
		std::string midiName = midiIn->getPortName(i);
#ifdef DV_DEBUG
		std::cout << "  Port " << i << ": " << midiName << std::endl;
#endif
		if (midiName.size() >= 2)
		{
			midiName = midiName.substr(0, midiName.size() - 2);
		}

		if (midiName == "loopMIDI Visualizer")
		{
			midiIn->openPort(i, midiName);
			foundPortName = midiName;
			found = true;
#ifdef DV_DEBUG
			std::cout << "Connected to MIDI port: " << midiName << std::endl;
#endif
			break;
		}
	}

	if (!found)
	{
		std::cout << "Failed To Find 'loopMIDI Visualizer' Port, shutting down" << std::endl;
		return false;
	}

	midiIn->setCallback(MidiCallback, nullptr);
	midiIn->ignoreTypes(false, false, false);

#ifndef DV_PLATFORM_WINDOWS
	midiOut = new RtMidiOut();
	midiOut->openVirtualPort(foundPortName);
#endif

	return true;
}

bool Visualizer::LoadConfig()
{
#ifdef DV_DEBUG
	std::cout << "Loading configuration..." << std::endl;
#endif
	std::ifstream file("settings.cfg");
	std::string data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	if (!file.is_open() || data.empty())
	{
		return false;
	}

	std::string name;
	std::string value;

	U64 i = 0;
	U64 end = 0;

	while ((end = data.find('=', i)) != std::string::npos)
	{
		name = data.substr(i, end - i);
		i = end + 1;
		end = data.find('\n', i);
		value = data.substr(i, end - i);
		i = end + 1;

		switch (Hash(name.data(), name.size()))
		{
		case "settingWindowX"_Hash: {
			settings.settingWindowX = SafeStoi(value, settings.settingWindowX);
		} break;
		case "settingWindowY"_Hash: {
			settings.settingWindowY = SafeStoi(value, settings.settingWindowY);
		} break;
		case "settingWindowWidth"_Hash: {
			settings.settingWindowWidth =
				SafeStoi(value, settings.settingWindowWidth);
		} break;
		case "settingWindowHeight"_Hash: {
			settings.settingWindowHeight =
				SafeStoi(value, settings.settingWindowHeight);
		} break;
		case "visualizerWindowX"_Hash: {
			settings.visualizerWindowX = SafeStoi(value, settings.visualizerWindowX);
		} break;
		case "visualizerWindowY"_Hash: {
			settings.visualizerWindowY = SafeStoi(value, settings.visualizerWindowY);
		} break;
		case "visualizerWindowWidth"_Hash: {
			settings.visualizerWindowWidth =
				SafeStoi(value, settings.visualizerWindowWidth);
		} break;
		case "visualizerWindowHeight"_Hash: {
			settings.visualizerWindowHeight =
				SafeStoi(value, settings.visualizerWindowHeight);
		} break;
		case "profileId"_Hash: {
			settings.profileId = SafeStoi(value, settings.profileId);
		} break;
		case "dynamicThreshold"_Hash: {
			settings.dynamicThreshold = SafeStoi(value, settings.dynamicThreshold);
		} break;
		case "leftyFlip"_Hash: {
			settings.leftyFlip = SafeStoi(value, settings.leftyFlip);
		} break;
		case "scrollSpeed"_Hash: {
			settings.scrollSpeed = SafeStof(value, settings.scrollSpeed);
		} break;
		case "scrollDirection"_Hash: {
			settings.scrollDirection =
				(ScrollDirection)SafeStoi(value, (I32)settings.scrollDirection);
		} break;
		case "tomTextureId"_Hash: {
			settings.tomTextureId =
				SafeStoi(value, settings.tomTextureId);
		} break;
		case "cymbalTextureId"_Hash: {
			settings.cymbalTextureId =
				SafeStoi(value, settings.cymbalTextureId);
		} break;
		case "kickTextureId"_Hash: {
			settings.kickTextureId =
				SafeStoi(value, settings.kickTextureId);
		} break;
		}
	}

	return true;
}

void Visualizer::SaveConfig()
{
	std::ofstream output("settings.cfg");
	output << "settingWindowX=" << settings.settingWindowX << '\n';
	output << "settingWindowY=" << settings.settingWindowY << '\n';
	output << "settingWindowWidth=" << settings.settingWindowWidth << '\n';
	output << "settingWindowHeight=" << settings.settingWindowHeight << '\n';
	output << "visualizerWindowX=" << settings.visualizerWindowX << '\n';
	output << "visualizerWindowY=" << settings.visualizerWindowY << '\n';
	output << "visualizerWindowWidth=" << settings.visualizerWindowWidth << '\n';
	output << "visualizerWindowHeight=" << settings.visualizerWindowHeight << '\n';
	output << "profileId=" << settings.profileId << '\n';
	output << "dynamicThreshold=" << settings.dynamicThreshold << '\n';
	output << "leftyFlip=" << settings.leftyFlip << '\n';
	output << "scrollSpeed=" << settings.scrollSpeed << '\n';
	output << "scrollDirection=" << static_cast<U32>(settings.scrollDirection) << '\n';
	output << "tomTextureId=" << settings.tomTextureId << '\n';
	output << "cymbalTextureId=" << settings.cymbalTextureId << '\n';
	output << "kickTextureId=" << settings.kickTextureId << '\n';

	output.flush();
	output.close();
}

std::wstring Visualizer::GetCloneHeroFolder()
{
#ifdef DV_PLATFORM_WINDOWS
	PWSTR path = nullptr;
	SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path);

	std::wstring documents(path);
	CoTaskMemFree(path);

	documents.append(L"\\Clone Hero\\");

#ifdef DV_DEBUG
	std::wcout << "Found Clone Hero path: " << documents << std::endl;
#endif

	return documents;
#endif
}

void Visualizer::LoadProfiles(const std::wstring& cloneHeroPath)
{
#ifdef DV_DEBUG
	std::cout << "Loading profiles..." << std::endl;
#endif
	std::ifstream file(cloneHeroPath + L"profiles.ini");
	std::string data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	if (!file.is_open() || data.empty())
	{
		std::cout << "Failed to open profiles.ini, using default profile" << std::endl;
		profiles.push_back({ "Guest", "DefaultColors", 100, false });
		return;
	}

	U64 i = 0;
	U64 end = 0;
	U64 start = 0;

	while ((start = data.find('[', i)) != std::string::npos)
	{
		Profile profile{};

		i = data.find("dynamics_threshold", start);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.dynamicThreshold = SafeStoi(data.substr(i, end - i), 100);

		i = data.find("color_profile_name", start);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.colorProfile = data.substr(i, end - i);

		i = data.find("player_name", start);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.name = data.substr(i, end - i);

		i = data.find("lefty_flip", start);
		i = data.find('=', i) + 2;
		end = data.find('\n', i);

		profile.leftyFlip = data.substr(i, end - i) == "1";

		profiles.push_back(profile);
	}

	if (settings.profileId == U32_MAX)
	{
		settings.profileId = 0;
	}
}

void Visualizer::LoadColors(const std::wstring& path)
{
#ifdef DV_DEBUG
	std::cout << "Loading profile colors..." << std::endl;
#endif
	std::ifstream file(path);
	std::string data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	if (!file.is_open() || data.empty())
	{
		std::cout << "Failed to open profile colors, using default colors" << std::endl;
		return;
	}

	U64 i = 0;
	U64 end = 0;
	U64 start = data.find("[drums]");

	i = data.find("note_kick ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	std::string hex = data.substr(i, end - i);
	colorProfile.kickColor = HexToRBG(hex);

	i = data.find("cym_blue ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.cymbal2Color = HexToRBG(hex);

	i = data.find("cym_yellow ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.cymbal1Color = HexToRBG(hex);

	i = data.find("cym_green ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.cymbal3Color = HexToRBG(hex);

	i = data.find("tom_blue ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.tom2Color = HexToRBG(hex);

	i = data.find("tom_yellow ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.tom1Color = HexToRBG(hex);

	i = data.find("tom_red ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.snareColor = HexToRBG(hex);

	i = data.find("tom_green ", start);
	i = data.find('#', i) + 1;
	end = data.find('\n', i);

	hex = data.substr(i, end - i);
	colorProfile.tom3Color = HexToRBG(hex);
}

Vector3 Visualizer::HexToRBG(const std::string& hex)
{
	static constexpr U64 RMask = 0xFF0000;
	static constexpr U64 GMask = 0x00FF00;
	static constexpr U64 BMask = 0x0000FF;

	U64 rgb = SafeStoull(hex, 16, 0);

	return { ((rgb & RMask) >> 16) / 255.0f, ((rgb & GMask) >> 8) / 255.0f,
			(rgb & BMask) / 255.0f };
}

void Visualizer::LoadMidiProfile(const std::wstring& path)
{
	std::ifstream file(path);
	std::string data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	if (!file.is_open() || data.empty())
	{
		std::wcout << "Failed to open MIDI profile " << path << ", shutting down" << std::endl;
		return;
	}

	U64 snare = data.find("Red Pad:");
	U64 tom1 = data.find("Yellow Pad:", snare);
	U64 tom2 = data.find("Blue Pad:", tom1);
	U64 tom3 = data.find("Green Pad:", tom2);
	U64 kick = data.find("Kick Pad:", tom3);
	U64 cymbal1 = data.find("Yellow Cymbal:", kick);
	U64 cymbal2 = data.find("Blue Cymbal:", cymbal1);
	U64 cymbal3 = data.find("Green Cymbal:", cymbal2);
	U64 start = data.find("Start:", cymbal3);

	ParseMappings(data, NoteType::Snare, snare, tom1);
	ParseMappings(data, NoteType::Tom1, tom1, tom2);
	ParseMappings(data, NoteType::Tom2, tom2, tom3);
	ParseMappings(data, NoteType::Tom3, tom3, kick);
	ParseMappings(data, NoteType::Kick, kick, cymbal1);
	ParseMappings(data, NoteType::Cymbal1, cymbal1, cymbal2);
	ParseMappings(data, NoteType::Cymbal2, cymbal2, cymbal3);
	ParseMappings(data, NoteType::Cymbal3, cymbal3, start);
}

void Visualizer::ParseMappings(const std::string& data, NoteType type,
	U64 start, U64 end)
{
	U64 i = start;
	U64 lineEnd = 0;

	while ((i = data.find('-', i)) < end)
	{
		Mapping mapping{};
		mapping.type = type;

		i = data.find(':', i) + 2;
		lineEnd = data.find('\n', i);
		mapping.midiValue = SafeStoi(data.substr(i, lineEnd - i), 0);

		i = data.find(':', i) + 2;
		lineEnd = data.find('\n', i);
		mapping.velocityThreshold = SafeStoi(data.substr(i, lineEnd - i), 0);

		i = data.find(':', i) + 2;
		lineEnd = data.find('\n', i);
		mapping.overhitThreshold = SafeStod(data.substr(i, lineEnd - i), 0.0);

		mappings.push_back(mapping);
	}
}

void Visualizer::SetScrollDirection(ScrollDirection direction)
{
	settings.scrollDirection = direction;

	switch (settings.scrollDirection)
	{
	case ScrollDirection::Down: {
		layout.snareStart = { -0.35f, 1.0f };
		layout.kickStart = { -0.25f, 1.0f };
		layout.cymbal1Start = { -0.15f, 1.0f };
		layout.tom1Start = { -0.05f, 1.0f };
		layout.cymbal2Start = { 0.05f, 1.0f };
		layout.tom2Start = { 0.15f, 1.0f };
		layout.cymbal3Start = { 0.25f, 1.0f };
		layout.tom3Start = { 0.35f, 1.0f };
	} break;
	case ScrollDirection::Right: {
		layout.snareStart = { -1.0f, -0.35f };
		layout.kickStart = { -1.0f, -0.25f };
		layout.cymbal1Start = { -1.0f, -0.15f };
		layout.tom1Start = { -1.0f, -0.05f };
		layout.cymbal2Start = { -1.0f, 0.05f };
		layout.tom2Start = { -1.0f, 0.15f };
		layout.cymbal3Start = { -1.0f, 0.25f };
		layout.tom3Start = { -1.0f, 0.35f };
	} break;
	case ScrollDirection::Up: {
		layout.snareStart = { -0.35f, -1.0f };
		layout.kickStart = { -0.25f, -1.0f };
		layout.cymbal1Start = { -0.15f, -1.0f };
		layout.tom1Start = { -0.05f, -1.0f };
		layout.cymbal2Start = { 0.05f, -1.0f };
		layout.tom2Start = { 0.15f, -1.0f };
		layout.cymbal3Start = { 0.25f, -1.0f };
		layout.tom3Start = { 0.35f, -1.0f };
	} break;
	case ScrollDirection::Left: {
		layout.snareStart = { 1.0f, -0.35f };
		layout.kickStart = { 1.0f, -0.25f };
		layout.cymbal1Start = { 1.0f, -0.15f };
		layout.tom1Start = { 1.0f, -0.05f };
		layout.cymbal2Start = { 1.0f, 0.05f };
		layout.tom2Start = { 1.0f, 0.15f };
		layout.cymbal3Start = { 1.0f, 0.25f };
		layout.tom3Start = { 1.0f, 0.35f };
	} break;
	default:
		break;
	}
}

void Visualizer::MidiCallback(F64 deltatime, std::vector<U8>* message,
	void* userData)
{
#ifndef DV_PLATFORM_WINDOWS
	midiOut->sendMessage(message);
#endif

	lastInput += deltatime;

	U64 byteCount = message->size();

#ifdef DV_DEBUG
	for (U32 i = 0; i < byteCount; ++i)
	{
		std::cout << "Byte " << i << " = " << (I32)message->at(i) << ", ";
	}
	std::cout << "stamp = " << deltatime << std::endl;
#endif

	if (byteCount >= 3 && (message->at(0) == 153 || message->at(0) == 144))
	{
		std::lock_guard<std::mutex> lock(midiMutex);
		for (Mapping& mapping : mappings)
		{
			if (message->at(1) == mapping.midiValue)
			{
				if (message->at(2) >= mapping.velocityThreshold &&
					(lastInput - mapping.lastHit) >= mapping.overhitThreshold)
				{
					mapping.lastHit = lastInput;

					F32 dynamic =
						message->at(2) < settings.dynamicThreshold ? 0.5f : 1.0f;

					switch (mapping.type)
					{
					case NoteType::Snare: {
						Renderer::SpawnNote(layout.snareStart,
							colorProfile.snareColor * dynamic, settings.tomTextureId);
					} break;
					case NoteType::Kick: {
						Renderer::SpawnNote(layout.kickStart,
							colorProfile.kickColor * dynamic, settings.kickTextureId);
					} break;
					case NoteType::Cymbal1: {
						Renderer::SpawnNote(layout.cymbal1Start,
							colorProfile.cymbal1Color * dynamic, settings.cymbalTextureId);
					} break;
					case NoteType::Tom1: {
						Renderer::SpawnNote(layout.tom1Start,
							colorProfile.tom1Color * dynamic, settings.tomTextureId);
					} break;
					case NoteType::Cymbal2: {
						Renderer::SpawnNote(layout.cymbal2Start,
							colorProfile.cymbal2Color * dynamic, settings.cymbalTextureId);
					} break;
					case NoteType::Tom2: {
						Renderer::SpawnNote(layout.tom2Start,
							colorProfile.tom2Color * dynamic, settings.tomTextureId);
					} break;
					case NoteType::Cymbal3: {
						Renderer::SpawnNote(layout.cymbal3Start,
							colorProfile.cymbal3Color * dynamic, settings.cymbalTextureId);
					} break;
					case NoteType::Tom3: {
						Renderer::SpawnNote(layout.tom3Start,
							colorProfile.tom3Color * dynamic, settings.tomTextureId);
					} break;
					}
				}

				break;
			}
		}
	}
}

void Visualizer::KeyCallback(GLFWwindow* window, I32 key, I32 scancode,
	I32 action, I32 mods)
{
#ifdef DV_DEBUG
	std::cout << "Key: " << key << ", Action: " << action << ", Mods: " << mods << std::endl;
#endif
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE: {
			glfwSetWindowShouldClose(window, true);
		} break;
		case GLFW_KEY_F1: {
#ifdef DV_DEBUG
			std::cout << "Configure mode toggled" << std::endl;
#endif
			configureMode = !configureMode;

			visualizerWindow.SetMenu(configureMode);
			visualizerWindow.SetInteractable(configureMode);
		} break;
		case GLFW_KEY_F2: {
			switch (settings.scrollDirection)
			{
			case ScrollDirection::Down: { SetScrollDirection(ScrollDirection::Right); } break;
			case ScrollDirection::Right: { SetScrollDirection(ScrollDirection::Up); } break;
			case ScrollDirection::Up: { SetScrollDirection(ScrollDirection::Left); } break;
			case ScrollDirection::Left: { SetScrollDirection(ScrollDirection::Down); } break;
			default: break;
			}

			Renderer::ClearNotes();
#ifdef DV_DEBUG
			std::cout << "Scroll direction changed to: " << (I32)settings.scrollDirection << std::endl;
#endif
		} break;
		case GLFW_KEY_MINUS:
		case GLFW_KEY_KP_SUBTRACT: {
			settings.scrollSpeed = max(settings.scrollSpeed - 0.25f, 0.5f);
#ifdef DV_DEBUG
			std::cout << "Scroll speed decreased to: " << settings.scrollSpeed << std::endl;
#endif
		} break;
		case GLFW_KEY_EQUAL:
		case GLFW_KEY_KP_ADD: {
			settings.scrollSpeed = min(settings.scrollSpeed + 0.25f, 5.0f);
#ifdef DV_DEBUG
			std::cout << "Scroll speed increased to: " << settings.scrollSpeed << std::endl;
#endif
		} break;
		}
	}
}

void Visualizer::ErrorCallback(I32 error, const C8* description)
{
	std::cout << description << std::endl;
}
