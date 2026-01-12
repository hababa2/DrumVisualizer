#pragma once

#include "Defines.hpp"

#include "Resources.hpp"
#include "Window.hpp"

#include <vector>
#include <array>
#include <string>
#include <mutex>

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

enum class NoteSeparationMode
{
	None,
	Cutoff,
	Squish
};

enum class NoteType
{
	Snare,
	Kick,
	Cymbal1,
	Tom1,
	Cymbal2,
	Tom2,
	Cymbal3,
	Tom3
};

struct Mapping
{
	NoteType type;
	I32 midiValue;
	I32 velocityThreshold;
	F64 overhitThreshold;
	F64 lastHit{ -1.0 };
};

struct Profile
{
	U32 id;
	std::string name;
	std::string colorProfile;
	U32 dynamicThreshold{ 100 };
	bool leftyFlip;
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
	I32 settingWindowWidth{ 630 };
	I32 settingWindowHeight{ 800 };

	I32 visualizerWindowX{ 730 };
	I32 visualizerWindowY{ 100 };
	I32 visualizerWindowWidth{ 350 };
	I32 visualizerWindowHeight{ 800 };

	std::string portName{ "loopMIDI Visualizer" };
	std::string colorProfileName{};
	std::string midiProfileName{ "loopMIDI CH" };
	U32 profileId{ U32_MAX };
	U32 dynamicThreshold{ 100 };
	bool leftyFlip{ false };
	bool showDynamics{ true };
	bool showStats{ true };

	F32 scrollSpeed{ 1.0f };
	ScrollDirection scrollDirection{ ScrollDirection::Down };
	F32 noteWidth{ 0.1f };
	F32 noteHeight{ 0.025f };
	F32 noteGap{ 0.005f };
	NoteSeparationMode noteSeparationMode{ NoteSeparationMode::Cutoff };

	std::string tomTextureName{ "square" };
	std::string cymbalTextureName{ "triangle" };
	std::string kickTextureName{ "square" };

	Texture* tomTexture{ nullptr };
	Texture* cymbalTexture{ nullptr };
	Texture* kickTexture{ nullptr };
};

struct NoteInfo
{
	std::string name;
	U32 index;
};

struct Stats
{
	Vector3 spawn{ 0.0f, 0.0f, 0.0f };
	U32 lastIndex{ U32_MAX };
	U32 hitCount{ 0 };
	U32 ghostCount{ 0 };
};

class Visualizer
{
public:
	static bool Initialize();
	static void Shutdown();

	static void MidiCallback(F64 deltatime, std::vector<U8>* message, void* userData);
	static void KeyCallback(GLFWwindow* window, I32 key, I32 scancode, I32 action, I32 mods);
	static void ErrorCallback(I32 error, const C8* description);

	static void SetScrollDirection(ScrollDirection direction);
	static bool LoadPort(const std::string& portName);
	static void SetProfile(I32 profileId);
	static void SetColorProfile(const std::string& name);
	static void SetMidiProfile(const std::string& name);
	static Settings& GetSettings();
	static std::array<Stats, 8>& GetStats();
	static std::array<NoteInfo, 8>& GetNoteInfos();
	static std::vector<char*>& GetPorts();
	static std::vector<char*>& GetProfiles();
	static std::vector<char*>& GetColorProfiles();
	static std::vector<char*>& GetMidiProfiles();

private:
	static void MainLoop();

	static bool InitializeGlfw();
	static bool InitializeWindows();
	static bool InitializeCH();
	static bool InitializeMidi();
	static bool LoadConfig();
	static void SaveConfig();
	static void PrepareTextures();
	static std::wstring GetCloneHeroFolder();
	static void LoadProfiles();
	static void LoadColorProfiles();
	static void LoadMidiProfiles();
	static void LoadColors(const std::wstring& path);
	static Vector3 HexToRBG(const std::string& hex);
	static bool LoadMidiProfile(const std::wstring& path);
	static void ParseMappings(const std::string& data, NoteType type, U64 start, U64 end);

	static Settings settings;
	static std::wstring cloneHeroFolder;
	static std::array<NoteInfo, 8> noteInfos;
	static std::array<Stats, 8> noteStats;
	static ColorProfile colorProfile;
	static std::vector<Profile> profiles;
	static std::vector<char*> profileNames;
	static std::vector<char*> colorProfileNames;
	static std::vector<char*> midiProfileNames;
	static std::vector<char*> midiPorts;
	static std::vector<Mapping> mappings;
	static Window settingsWindow;
	static Window visualizerWindow;
	static GLFWmonitor* monitor;
	static rt::midi::RtMidiIn* midiIn;
	static rt::midi::RtMidiOut* midiOut;
	static F64 lastInput;
	static bool configureMode;
	static std::mutex midiMutex;

	STATIC_CLASS(Visualizer)
};