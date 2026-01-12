#pragma once

#include "Defines.hpp"

#include "Resources.hpp"
#include "Buffer.hpp"
#include "Window.hpp"

#include <vector>

struct Stats;

class Renderer
{
public:
	static bool Initialize();
	static void Shutdown();

	static void Update(Vector2 velocity, Window& settingsWindow, Window& visualizerWindow);
	static void SpawnNote(Stats& stats, const Vector3& color, Texture* texture);
	static void ClearNotes();

private:
	static U32 vao;
	static U32 textureBuffer;
	static U32 shaderProgram;
	static Buffer positionBuffer;
	static Buffer texCoordsBuffer;
	static Buffer offsetsBuffer;
	static Buffer scalesBuffer;
	static Buffer texCoordOffsetsBuffer;
	static Buffer texCoordScalesBuffer;
	static Buffer colorsBuffer;
	static Buffer textureIdsBuffer;
	static Texture* defaultTexture;

	static constexpr U32 MaxNotes = 200;
	static U32 nextIndex;
	static std::vector<Vector3> offsets;
	static std::vector<Vector2> scales;
	static std::vector<Vector2> texCoordOffsets;
	static std::vector<Vector2> texCoordScales;
	static std::vector<Vector3> colors;
	static std::vector<U32> textureIds;

	static Vector2 positions[4];
	static Vector2 texCoords[4];
	static U32 indices[6];

	STATIC_CLASS(Renderer)
};