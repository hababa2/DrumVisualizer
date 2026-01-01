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
	static Window* settingsWindow;
	static Window* visualizerWindow;

	static ImGuiContext* settingsContext;
	static ImGuiContext* visualizerContext;

	STATIC_CLASS(UI)
};