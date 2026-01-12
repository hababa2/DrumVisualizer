#pragma once

#include "Defines.hpp"

#include <string>

struct GLFWwindow;

struct WindowConfig
{
	std::string name = "Window";
	I32 x = 100;
	I32 y = 100;
	I32 width = 800;
	I32 height = 600;
	bool menu = true;
	bool transparent = false;
	bool interactable = true;
	bool floating = false;
	Vector4 clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct Window
{
	void Create(WindowConfig config = {}, Window* share = nullptr);
	void Destroy();

	void Update();
	void Render();
	void SetMenu(bool b);
	void SetTransparent(bool b);
	void SetInteractable(bool b);
	void SetFloating(bool b);
	void SetClearColor(const Vector4& color);

	const WindowConfig& Config() const;

	operator bool() const;
	operator GLFWwindow* () const;

private:

	WindowConfig config;
	GLFWwindow* window = nullptr;
};