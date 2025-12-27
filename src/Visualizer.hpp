#pragma once

#include "Defines.hpp"

#include "Window.hpp"

#include <vector>
#include <string>

struct GLFWwindow;
struct GLFWmonitor;
namespace rt { namespace midi { class RtMidiIn; } }
namespace rt { namespace midi { class RtMidiOut; } }

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
	I32 velocityThreshold;
	F64 overhitThreshold;
	F64 lastHit{ 0.0 };
};

struct Profile
{
	std::string name;
	std::string colorProfile;
	U32 dynamicThreshold{ 100 };
	bool leftyFlip;
};

struct Layout
{
	Vector2 snareStart{ -0.35f, 1.0f };
	Vector2 kickStart{ -0.25f, 1.0f };
	Vector2 cymbal1Start{ -0.15f, 1.0f };
	Vector2 tom1Start{ -0.05f, 1.0f };
	Vector2 cymbal2Start{ 0.05f, 1.0f };
	Vector2 tom2Start{ 0.15f, 1.0f };
	Vector2 cymbal3Start{ 0.25f, 1.0f };
	Vector2 tom3Start{ 0.35f, 1.0f };
};

struct ColorProfile
{
	Vector3 snareColor{ 1.0f, 0.0f, 0.0f };
	Vector3 tom1Color{ 1.0f, 1.0f, 0.0f };
	Vector3 tom2Color{ 0.0f, 0.537254930f, 1.0f };
	Vector3 tom3Color{ 0.0f, 1.0f, 0.0f };
	Vector3 cymbal1Color{ 1.0f, 0.898039222f, 0.192156866f };
	Vector3 cymbal2Color{ 0.113725491f, 0.388235301f, 1.0f };
	Vector3 cymbal3Color{ 0.0470588244f, 1.0f, 0.0470588244f };
	Vector3 kickColor{ 1.0f, 0.274509817f, 0.0f };
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

	U32 profileId{ U32_MAX };
	U32 dynamicThreshold{ 100 };
	bool leftyFlip{ false };

	F32 scrollSpeed{ 1.0f };
	ScrollDirection scrollDirection{ ScrollDirection::Down };
};

class Visualizer
{
public:
	static bool Initialize();

	static void MidiCallback(F64 deltatime, std::vector<U8>* message, void* userData);
	static void KeyCallback(GLFWwindow* window, I32 key, I32 scancode, I32 action, I32 mods);
	static void ErrorCallback(I32 error, const C8* description);

private:
	static void Shutdown();
	static void MainLoop();

	static bool InitializeGlfw();
	static bool InitializeWindows();
	static bool InitializeCH();
	static bool InitializeMidi();
	static bool LoadConfig();
	static void SaveConfig();
	static std::wstring GetCloneHeroFolder();
	static void LoadProfiles(const std::wstring& cloneHeroPath);
	static void LoadColors(const std::wstring& path);
	static Vector3 HexToRBG(const std::string& hex);
	static void LoadMidiProfile(const std::wstring& path);
	static void ParseMappings(const std::string& data, NoteType type, U64 start, U64 end);
	static void SetScrollDirection(ScrollDirection direction);

	static Settings settings;
	static Layout layout;
	static ColorProfile colorProfile;
	static std::vector<Profile> profiles;
	static std::vector<Mapping> mappings;
	static Window settingsWindow;
	static Window visualizerWindow;
	static GLFWmonitor* monitor;
	static rt::midi::RtMidiIn* midiIn;
	static rt::midi::RtMidiOut* midiOut;
	static F64 lastInput;
	static bool configureMode;
};