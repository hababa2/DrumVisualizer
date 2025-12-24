#pragma once

#include "Defines.hpp"

#include "Window.hpp"

#include <vector>

struct GLFWwindow;
struct GLFWmonitor;
namespace rt { namespace midi { struct RtMidiIn; } }

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
};

class Visualizer
{
public:
	static bool Initialize();
	static void Shutdown();

	static void MidiCallback(F64 deltatime, std::vector<U8>* message, void* userData);
	static void KeyCallback(GLFWwindow* window, I32 key, I32 scancode, I32 action, I32 mods);
	static void ErrorCallback(I32 error, const C8* description);

private:
	static Settings settings;
	static std::vector<Mapping> mappings;
	static Window settingsWindow;
	static Window visualizerWindow;
	static GLFWmonitor* monitor;
	static rt::midi::RtMidiIn* midi;
	static bool configureMode;
};