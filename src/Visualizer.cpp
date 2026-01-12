#include "Visualizer.hpp"

#include "Renderer.hpp"
#include "UI.hpp"
#include "Resources.hpp"

#include "GraphicsInclude.hpp"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "rtmidi\RtMidi.h"

#include <codecvt>
#include <filesystem>
#include <fstream>
#include <mutex>

#ifdef DV_PLATFORM_WINDOWS
#include "shlobj_core.h"
#include <objbase.h>
#endif

Settings Visualizer::settings{};
std::wstring Visualizer::cloneHeroFolder;
std::array<NoteInfo, 8> Visualizer::noteInfos = {
	NoteInfo{ "Snare", 0 },
	NoteInfo{ "Kick", 1 },
	NoteInfo{ "Cymbal 1", 2 },
	NoteInfo{ "Tom 1", 3 },
	NoteInfo{ "Cymbal 2", 4 },
	NoteInfo{ "Tom 2", 5 },
	NoteInfo{ "Cymbal 3", 6 },
	NoteInfo{ "Tom 3", 7 }
};
std::array<Stats, 8> Visualizer::noteStats;
ColorProfile Visualizer::colorProfile{};
std::vector<Profile> Visualizer::profiles;
std::vector<char*> Visualizer::profileNames;
std::vector<char*> Visualizer::colorProfileNames;
std::vector<char*> Visualizer::midiProfileNames;
std::vector<char*> Visualizer::midiPorts;
std::vector<Mapping> Visualizer::mappings;
Window Visualizer::settingsWindow;
Window Visualizer::visualizerWindow;
GLFWmonitor* Visualizer::monitor = nullptr;
RtMidiIn* Visualizer::midiIn = nullptr;
RtMidiOut* Visualizer::midiOut = nullptr;
F64 Visualizer::lastInput = 0.0;
bool Visualizer::configureMode = false;
std::mutex Visualizer::midiMutex;

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

bool Visualizer::Initialize()
{
#ifdef DV_DEBUG
	std::cout << "=== DrumVisualizer Debug Mode ===" << std::endl;
#endif
	if (!InitializeGlfw()) { return false; }
	if (!InitializeWindows()) { return false; }
	if (!InitializeCH()) { return false; }
	if (!InitializeMidi()) { return false; }
	if (!Resources::Initialize()) { return false; }
	PrepareTextures();
	if (!Renderer::Initialize()) { return false; }
	if (!UI::Initialize(&settingsWindow, &visualizerWindow)) { return false; }
#ifdef DV_DEBUG
	std::cout << "Initialized Successfully!" << std::endl;
#endif

	MainLoop();
	Shutdown();

	return true;
}

void Visualizer::Shutdown()
{
#ifdef DV_DEBUG
	std::cout << std::endl << "=== Shutting Down ===" << std::endl;
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
	std::cout << "Saving Configuration..." << std::endl;
#endif
	SaveConfig();

#ifdef DV_DEBUG
	std::cout << "Cleaning Up Resources..." << std::endl;
#endif

	for (char* str : profileNames) { delete[] str; }
	for (char* str : colorProfileNames) { delete[] str; }
	for (char* str : midiProfileNames) { delete[] str; }
	for (char* str : midiPorts) { delete[] str; }

	UI::Shutdown();
	Renderer::Shutdown();
	Resources::Shutdown();

#ifndef DV_PLATFORM_WINDOWS
	delete midiOut;
#endif
	delete midiIn;

	settingsWindow.Destroy();
	visualizerWindow.Destroy();
	glfwTerminate();

#ifdef DV_DEBUG
	std::cout << "Shutdown Complete" << std::endl;
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
	std::cout << "Initializing Windows..." << std::endl;
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

	WindowConfig config{};
	config.name = "Drum Visualizer Settings";
	config.transparent = false;
	config.floating = false;
	config.interactable = true;
	config.menu = true;
	config.clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
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

	visualizerWindow.Create(config, &settingsWindow);
	glfwSetKeyCallback(visualizerWindow, KeyCallback);

	SetScrollDirection(settings.scrollDirection);

	return true;
}

bool Visualizer::InitializeCH()
{
	cloneHeroFolder = GetCloneHeroFolder();
	LoadProfiles();
	LoadColorProfiles();
	LoadMidiProfiles();

	if (settings.profileId != U32_MAX)
	{
		settings.dynamicThreshold = profiles[settings.profileId].dynamicThreshold;
		settings.leftyFlip = profiles[settings.profileId].leftyFlip;

		if (settings.colorProfileName.empty()) { settings.colorProfileName = profiles[settings.profileId].colorProfile; }

		SetColorProfile(settings.colorProfileName);
	}

	SetMidiProfile(settings.midiProfileName);

	return true;
}

bool Visualizer::InitializeMidi()
{
#ifdef DV_DEBUG
	std::cout << "Initializing MIDI..." << std::endl;
#endif
	midiIn = new RtMidiIn();

	bool connected = false;
	std::string foundPortName;
	U32 portCount = midiIn->getPortCount();
#ifdef DV_DEBUG
	std::cout << "Found " << portCount << " MIDI port(s)" << std::endl;
#endif
	for (U32 i = 0; i < portCount; ++i)
	{
		std::string midiName = midiIn->getPortName(i);
		if (midiName.size() >= 2)
		{
			midiName = midiName.substr(0, midiName.size() - 2);
		}

#ifdef DV_DEBUG
		std::cout << "  Port " << i << ": " << midiName << std::endl;
#endif
		char* str = new char[midiName.length() + 1];
		memcpy(str, midiName.c_str(), midiName.length() + 1);
		midiPorts.push_back(str);

		if (midiName == settings.portName)
		{
			if (LoadPort(midiName)) { connected = true; }
		}
	}

	if (!connected)
	{
		for (const std::string& port : midiPorts)
		{
			if (LoadPort(port)) { connected = true; break; }
		}

		if (!connected)
		{
			std::cout << "Failed To Connect To Any Available Port, Shutting Down!" << std::endl;
			return false;
		}
	}

	midiIn->setCallback(MidiCallback, nullptr);
	midiIn->ignoreTypes(false, false, false);

#ifndef DV_PLATFORM_WINDOWS
	midiOut = new RtMidiOut();
	midiOut->openVirtualPort(foundPortName);
#endif

	return true;
}

bool Visualizer::LoadPort(const std::string& portName)
{
	if (midiIn->isPortOpen()) { midiIn->closePort(); }

	for (U32 i = 0; i < midiPorts.size(); ++i)
	{
		std::string midiName = midiIn->getPortName(i);
		if (midiName.size() >= 2)
		{
			midiName = midiName.substr(0, midiName.size() - 2);
		}

		if (midiName == portName)
		{
			try
			{
				midiIn->openPort(i, midiName);
#ifdef DV_DEBUG
				std::cout << "Connected to MIDI port: " << portName << std::endl;
#endif
				return true;
			}
			catch (...)
			{
				std::cout << "Failed To Open '" << portName << "' Port!" << std::endl;
				return false;
			}
		}
	}

	std::cout << "Failed To Find '" << portName << "' Port!" << std::endl;
	return false;
}

bool Visualizer::LoadConfig()
{
#ifdef DV_DEBUG
	std::cout << "Loading Configuration..." << std::endl;
#endif
	std::string data = Resources::ReadFile("settings.cfg");

	if (data.empty()) { return false; }

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
		case "showDynamics"_Hash: {
			settings.showDynamics = SafeStoi(value, settings.showDynamics);
		} break;
		case "showStats"_Hash: {
			settings.showStats = SafeStoi(value, settings.showStats);
		} break;
		case "scrollSpeed"_Hash: {
			settings.scrollSpeed = SafeStof(value, settings.scrollSpeed);
		} break;
		case "scrollDirection"_Hash: {
			settings.scrollDirection =
				(ScrollDirection)SafeStoi(value, (I32)settings.scrollDirection);
		} break;
		case "noteWidth"_Hash: {
			settings.noteWidth = SafeStof(value, settings.noteWidth);
		} break;
		case "noteHeight"_Hash: {
			settings.noteHeight = SafeStof(value, settings.noteHeight);
		} break;
		case "noteGap"_Hash: {
			settings.noteGap = SafeStof(value, settings.noteGap);
		} break;
		case "noteSeparationMode"_Hash: {
			settings.noteSeparationMode =
				(NoteSeparationMode)SafeStoi(value, (I32)settings.noteSeparationMode);
		} break;
		case "tomTextureName"_Hash: {
			settings.tomTextureName = value;
		} break;
		case "cymbalTextureName"_Hash: {
			settings.cymbalTextureName = value;
		} break;
		case "kickTextureName"_Hash: {
			settings.kickTextureName = value;
		} break;
		case "backgroundColor"_Hash: {
			U64 vStart = 0;
			U64 vEnd = value.find(',', vStart);
			settings.backgroundColor.x = SafeStof(value.substr(vStart, vEnd - vStart), settings.backgroundColor.x);
			vStart = vEnd + 1;
			vEnd = value.find(',', vStart);
			settings.backgroundColor.y = SafeStof(value.substr(vStart, vEnd - vStart), settings.backgroundColor.y);
			vStart = vEnd + 1;
			vEnd = value.find(',', vStart);
			settings.backgroundColor.z = SafeStof(value.substr(vStart, vEnd - vStart), settings.backgroundColor.z);
			vStart = vEnd + 1;
			settings.backgroundColor.w = SafeStof(value.substr(vStart), settings.backgroundColor.w);
		} break;
		case "portName"_Hash: {
			settings.portName = value;
		} break;
		case "colorProfileName"_Hash: {
			settings.colorProfileName = value;
		} break;
		case "midiProfileName"_Hash: {
			settings.midiProfileName = value;
		} break;
		case "noteLayout"_Hash: {
			U32 i = 0;
			for (C8 c : value)
			{
				U32 index = c - '0';

				switch (index)
				{
				case 0: { noteInfos[i] = { "Snare", index }; } break;
				case 1: { noteInfos[i] = { "Kick", index }; } break;
				case 2: { noteInfos[i] = { "Cymbal 1", index }; } break;
				case 3: { noteInfos[i] = { "Tom 1", index }; } break;
				case 4: { noteInfos[i] = { "Cymbal 2", index }; } break;
				case 5: { noteInfos[i] = { "Tom 2", index }; } break;
				case 6: { noteInfos[i] = { "Cymbal 3", index }; } break;
				case 7: { noteInfos[i] = { "Tom 3", index }; } break;
				}

				++i;
			}
		} break;
		default: break;
		}

		if (i == 0) { break; }
	}

	settings.settingWindowX = max(settings.settingWindowX, 0);
	settings.settingWindowY = max(settings.settingWindowY, 0);
	settings.settingWindowWidth = max(settings.settingWindowWidth, 100);
	settings.settingWindowHeight = max(settings.settingWindowHeight, 100);
	settings.visualizerWindowX = max(settings.visualizerWindowX, 0);
	settings.visualizerWindowY = max(settings.visualizerWindowY, 0);
	settings.visualizerWindowWidth = max(settings.visualizerWindowWidth, 100);
	settings.visualizerWindowHeight = max(settings.visualizerWindowHeight, 100);

	return true;
}

void Visualizer::SaveConfig()
{
	settings.settingWindowX = max(settings.settingWindowX, 0);
	settings.settingWindowY = max(settings.settingWindowY, 0);
	settings.settingWindowWidth = max(settings.settingWindowWidth, 100);
	settings.settingWindowHeight = max(settings.settingWindowHeight, 100);
	settings.visualizerWindowX = max(settings.visualizerWindowX, 0);
	settings.visualizerWindowY = max(settings.visualizerWindowY, 0);
	settings.visualizerWindowWidth = max(settings.visualizerWindowWidth, 100);
	settings.visualizerWindowHeight = max(settings.visualizerWindowHeight, 100);

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
	output << "showDynamics=" << settings.showDynamics << '\n';
	output << "showStats=" << settings.showStats << '\n';
	output << "scrollSpeed=" << settings.scrollSpeed << '\n';
	output << "scrollDirection=" << static_cast<U32>(settings.scrollDirection) << '\n';
	output << "noteWidth=" << settings.noteWidth << '\n';
	output << "noteHeight=" << settings.noteHeight << '\n';
	output << "noteGap=" << settings.noteGap << '\n';
	output << "noteSeparationMode=" << static_cast<U32>(settings.noteSeparationMode) << '\n';
	output << "tomTextureName=" << settings.tomTextureName << '\n';
	output << "cymbalTextureName=" << settings.cymbalTextureName << '\n';
	output << "kickTextureName=" << settings.kickTextureName << '\n';
	output << "backgroundColor=" << settings.backgroundColor.x << ',' <<
		settings.backgroundColor.y << ',' << settings.backgroundColor.z <<
		',' << settings.backgroundColor.w << '\n';
	output << "portName=" << settings.portName << '\n';
	output << "colorProfileName=" << settings.colorProfileName << '\n';
	output << "midiProfileName=" << settings.midiProfileName << '\n';
	
	output << "noteLayout=";
	for (NoteInfo& info : noteInfos) { output << info.index; }
	output << '\n';

	output.flush();
	output.close();
}

void Visualizer::PrepareTextures()
{
	settings.tomTexture = Resources::GetTexture(settings.tomTextureName);
	settings.cymbalTexture = Resources::GetTexture(settings.cymbalTextureName);
	settings.kickTexture = Resources::GetTexture(settings.kickTextureName);
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
	std::wcout << "Found Clone Hero path: '" << documents << "'" << std::endl;
#endif

	return documents;
#elif DV_PLATFORM_LINUX
	return "~/.clonehero";
#elif DV_PLATFORM_MAC
	return "~/Clone Hero";
#endif
}

void Visualizer::LoadProfiles()
{
#ifdef DV_DEBUG
	std::cout << "Loading Profiles..." << std::endl;
#endif
	std::string data = Resources::ReadFile(cloneHeroFolder + L"profiles.ini");

	if (data.empty())
	{
		std::cout << "Failed to open profiles.ini, using default profile" << std::endl;
		profiles.push_back({ U32_MAX, "Guest", "DefaultColors", 100, false });
		return;
	}

	U64 i = 0;
	U64 end = 0;
	U64 start = 0;

	U32 id = 0;
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
		profile.id = id;

		profiles.push_back(profile);

		char* name = new char[profile.name.size() + 1];
		memcpy(name, profile.name.c_str(), profile.name.size() + 1);
		profileNames.push_back(name);
	}

	if (settings.profileId == U32_MAX)
	{
		settings.profileId = 0;
	}
}

void Visualizer::LoadColorProfiles()
{
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(cloneHeroFolder + L"Custom\\Colors\\"))
	{
		std::string path = entry.path().string();

		std::string ext = path.substr(path.find_last_of('.') + 1);
		U64 start = path.find_last_of('\\') + 1;
		std::string file = path.substr(start, path.find_last_of('.') - start);

		if (ext == "ini")
		{
			char* name = new char[file.size() + 1];
			memcpy(name, file.c_str(), file.size() + 1);
			colorProfileNames.push_back(name);
		}
	}
}

void Visualizer::LoadMidiProfiles()
{
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(cloneHeroFolder + L"MIDI Profiles\\"))
	{
		std::string path = entry.path().string();

		std::string ext = path.substr(path.find_last_of('.') + 1);
		U64 start = path.find_last_of('\\') + 1;
		std::string file = path.substr(start, path.find_last_of('.') - start);

		if (ext == "yaml")
		{
			char* name = new char[file.size() + 1];
			memcpy(name, file.c_str(), file.size() + 1);
			midiProfileNames.push_back(name);
		}
	}
}

void Visualizer::SetProfile(I32 profileId)
{
	settings.dynamicThreshold = profiles[profileId].dynamicThreshold;
	settings.leftyFlip = profiles[profileId].leftyFlip;
}

void Visualizer::SetColorProfile(const std::string& name)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	LoadColors(cloneHeroFolder + L"Custom\\Colors\\" + converter.from_bytes(name) + L".ini");
}

void Visualizer::SetMidiProfile(const std::string& name)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	if (!LoadMidiProfile(cloneHeroFolder + L"MIDI Profiles\\" + converter.from_bytes(name) + L".yaml"))
	{
		settings.midiProfileName = midiProfileNames[0];
		LoadMidiProfile(cloneHeroFolder + L"MIDI Profiles\\" + converter.from_bytes(midiProfileNames[0]) + L".yaml");
	}
}

void Visualizer::LoadColors(const std::wstring& path)
{
#ifdef DV_DEBUG
	std::cout << "Loading Profile Colors..." << std::endl;
#endif
	std::string data = Resources::ReadFile(path);

	if (data.empty())
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

bool Visualizer::LoadMidiProfile(const std::wstring& path)
{
	std::string data = Resources::ReadFile(path);

	if (data.empty())
	{
		std::wcout << "Failed to open MIDI profile " << path << ", shutting down" << std::endl;
		return false;
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

	return true;
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
	I32 width, height;
	glfwGetFramebufferSize(visualizerWindow, &width, &height);
	F32 spawnPosition = 1.0f - settings.noteHeight;
	F32 layoutPosition = -0.875f;
	F32 layoutIncrement = 0.25f;

	switch (settings.scrollDirection)
	{
	case ScrollDirection::Down: {
		if (settings.showStats) { spawnPosition = (height - 100.0f) / height - settings.noteHeight; }
		for (Stats& s : noteStats)
		{
			s.spawn = { layoutPosition, spawnPosition, 0.0f };
			layoutPosition += layoutIncrement;
		}
	} break;
	case ScrollDirection::Right: {
		if (settings.showStats) { spawnPosition = (width - 100.0f) / width - settings.noteHeight; }
		for (Stats& s : noteStats)
		{
			s.spawn = { -spawnPosition, layoutPosition, 0.0f };
			layoutPosition += layoutIncrement;
		}
	} break;
	case ScrollDirection::Up: {
		if (settings.showStats) { spawnPosition = (height - 100.0f) / height - settings.noteHeight; }
		for (Stats& s : noteStats)
		{
			s.spawn = { layoutPosition, -spawnPosition, 0.0f };
			layoutPosition += layoutIncrement;
		}
	} break;
	case ScrollDirection::Left: {
		if (settings.showStats) { spawnPosition = (width - 100.0f) / width - settings.noteHeight; }
		for (Stats& s : noteStats)
		{
			s.spawn = { spawnPosition, layoutPosition, 0.0f };
			layoutPosition += layoutIncrement;
		}
	} break;
	default:
		break;
	}

	Renderer::ClearNotes();
}

Settings& Visualizer::GetSettings()
{
	return settings;
}

std::array<Stats, 8>& Visualizer::GetStats()
{
	return noteStats;
}

std::array<NoteInfo, 8>& Visualizer::GetNoteInfos()
{
	return noteInfos;
}

std::vector<char*>& Visualizer::GetPorts()
{
	return midiPorts;
}

std::vector<char*>& Visualizer::GetProfiles()
{
	return profileNames;
}

std::vector<char*>& Visualizer::GetColorProfiles()
{

	return colorProfileNames;
}

std::vector<char*>& Visualizer::GetMidiProfiles()
{
	return midiProfileNames;
}

void Visualizer::MidiCallback(F64 deltatime, std::vector<U8>* message,
	void* userData)
{
#ifndef DV_PLATFORM_WINDOWS
	midiOut->sendMessage(message);
#endif

	lastInput += deltatime;

	U64 byteCount = message->size();

	//#ifdef DV_DEBUG
	for (U32 i = 0; i < byteCount; ++i)
	{
		std::cout << "Byte " << i << " = " << (I32)message->at(i) << ", ";
	}
	std::cout << "stamp = " << deltatime << std::endl;
	//#endif

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

					bool ghost = message->at(2) < settings.dynamicThreshold;

					F32 dynamicMod = (ghost && settings.showDynamics) ? 0.5f : 1.0f;

					switch (mapping.type)
					{
					case NoteType::Snare: {
						Stats& stats = noteStats[0];
						Renderer::SpawnNote(noteStats[noteInfos[0].index], colorProfile.snareColor * dynamicMod, settings.tomTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Kick: {
						Stats& stats = noteStats[1];
						Renderer::SpawnNote(noteStats[noteInfos[1].index], colorProfile.kickColor * dynamicMod, settings.kickTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Cymbal1: {
						Stats& stats = noteStats[2];
						Renderer::SpawnNote(noteStats[noteInfos[2].index], colorProfile.cymbal1Color * dynamicMod, settings.cymbalTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Tom1: {
						Stats& stats = noteStats[3];
						Renderer::SpawnNote(noteStats[noteInfos[3].index], colorProfile.tom1Color * dynamicMod, settings.tomTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Cymbal2: {
						Stats& stats = noteStats[4];
						Renderer::SpawnNote(noteStats[noteInfos[4].index], colorProfile.cymbal2Color * dynamicMod, settings.cymbalTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Tom2: {
						Stats& stats = noteStats[5];
						Renderer::SpawnNote(noteStats[noteInfos[5].index], colorProfile.tom2Color * dynamicMod, settings.tomTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Cymbal3: {
						Stats& stats = noteStats[6];
						Renderer::SpawnNote(noteStats[noteInfos[6].index], colorProfile.cymbal3Color * dynamicMod, settings.cymbalTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
					} break;
					case NoteType::Tom3: {
						Stats& stats = noteStats[7];
						Renderer::SpawnNote(noteStats[noteInfos[7].index], colorProfile.tom3Color * dynamicMod, settings.tomTexture);
						++stats.hitCount;
						if (ghost) { ++stats.ghostCount; }
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
		case GLFW_KEY_F1: {
#ifdef DV_DEBUG
			std::cout << "Configure mode toggled" << std::endl;
#endif
			configureMode = !configureMode;

			visualizerWindow.SetMenu(configureMode);
			visualizerWindow.SetInteractable(configureMode);
		} break;
		}
	}
}

void Visualizer::ErrorCallback(I32 error, const C8* description)
{
	std::cout << description << std::endl;
}
