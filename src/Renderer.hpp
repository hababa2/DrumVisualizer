#pragma once

#include "Defines.hpp"

#include "Resources.hpp"
#include "Buffer.hpp"
#include "Window.hpp"

#include <vector>

class Renderer
{
public:
	static bool Initialize();
	static void Shutdown();

	static void Update(Vector2 velocity, Window& settingsWindow, Window& visualizerWindow);
	static void SpawnNote(const Vector2& position, const Vector3& color, Texture* texture);
	static void ClearNotes();

private:
	static U32 vao;
	static U32 textureBuffer;
	static U32 shaderProgram;
	static Buffer positionBuffer;
	static Buffer texCoordsBuffer;
	static Buffer offsetsBuffer;
	static Buffer colorsBuffer;
	static Buffer textureIdsBuffer;
	static Texture* defaultTexture;

	static constexpr U32 MaxNotes = 200;
	static U32 nextIndex;
	static std::vector<Vector2> offsets;
	static std::vector<Vector3> colors;
	static std::vector<U32> textureIds;

	STATIC_CLASS(Renderer)
};