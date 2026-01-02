#pragma once

#include "Defines.hpp"

#include "Resources.hpp"
#include "Buffer.hpp"
#include "Window.hpp"

struct ImGuiContext;

class UI
{
public:
	static bool Initialize(Window* settingsWindow, Window* visualizerWindow);
	static void Shutdown();
	static void Update(Window* window);
	static void Render(Window* window);

private:
	static void SetupColumn(U32 value1, U32 value2, F32 rowHeight, F32 blockHeight, bool showDynamics);
	static void SetupRow(U32 value1, U32 value2, F32 height, F32 blockHeight, bool showDynamics);

	static Window* settingsWindow;
	static Window* visualizerWindow;

	static ImGuiContext* settingsContext;
	static ImGuiContext* visualizerContext;

	STATIC_CLASS(UI)
};