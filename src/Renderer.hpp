#pragma once

#include "Defines.hpp"

#include "Buffer.hpp"
#include "Window.hpp"

#include <vector>

class Renderer
{
public:
	static bool Initialize();
	static void Shutdown();

	static void Update(F64 deltaTime, Window& settingsWindow, Window& visualizerWindow);
	static void SpawnNote(const Vector2& position, const Vector3& color);

private:
	static U32 vao;
	static U32 shaderProgram;
	static Buffer positionBuffer;
	static Buffer offsetsBuffer;
	static Buffer colorsBuffer;

	static std::vector<Vector2> offsets;
	static std::vector<Vector3> colors;
};