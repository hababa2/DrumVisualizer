#pragma once

#include "Defines.hpp"

#include "Resources.hpp"
#include "Buffer.hpp"
#include "Window.hpp"

struct Settings;
struct Stats;
struct NoteInfo;
struct Profile;
struct ImGuiContext;

class UI
{
public:
	static bool Initialize(Window* settingsWindow, Window* visualizerWindow);
	static void Shutdown();
	static void Update(Window* window);
	static void Render(Window* window);

	static F32 statsSize;

private:
	static void SetupColumn(U32 value1, U32 value2, F32 rowHeight, F32 blockHeight, bool showDynamics);
	static void SetupRow(U32 value1, U32 value2, F32 height, F32 blockHeight, bool showDynamics);
	static void SetupKick(U32 value1, U32 value2, bool showDynamics);

	static Window* settingsWindow;
	static Window* visualizerWindow;

	static ImGuiContext* settingsContext;
	static ImGuiContext* visualizerContext;

	static I32 flags;
	static Settings* settings;
	static std::array<Stats, 8>* stats;
	static std::array<NoteInfo, 8>* noteInfos;
	static const std::vector<char*>* ports;
	static const std::vector<char*>* profiles;
	static const std::vector<char*>* colorProfiles;
	static const std::vector<char*>* midiProfiles;
	static const std::vector<char*>* textures;

	static const char* directions[];
	static const char* separationModes[];
	static I32 direction;
	static I32 separationMode;
	static I32 tomId;
	static I32 cymbalId;
	static I32 kickId;
	static I32 portId;
	static I32 profileId;
	static I32 colorProfileId;
	static I32 midiProfileId;

	STATIC_CLASS(UI)
};