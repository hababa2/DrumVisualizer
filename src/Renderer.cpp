#include "Renderer.hpp"

#include "UI.hpp"
#include "Resources.hpp"
#include "Visualizer.hpp"

#include "GraphicsInclude.hpp"

#include <iostream>

Vector2 Renderer::positions[4] = {};

Vector2 Renderer::texCoords[4] = {
	{ 1.0f, 1.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f }
};

U32 Renderer::indices[6] = {
	0, 1, 3,
	1, 2, 3
};

U32 Renderer::vao;
U32 Renderer::textureBuffer;
U32 Renderer::shaderProgram;
Buffer Renderer::positionBuffer;
Buffer Renderer::texCoordsBuffer;
Buffer Renderer::offsetsBuffer;
Buffer Renderer::scalesBuffer;
Buffer Renderer::texCoordOffsetsBuffer;
Buffer Renderer::texCoordScalesBuffer;
Buffer Renderer::colorsBuffer;
Buffer Renderer::textureIdsBuffer;
Texture* Renderer::defaultTexture;
U32 Renderer::nextIndex = 0;
std::vector<Vector3> Renderer::offsets;
std::vector<Vector2> Renderer::scales;
std::vector<Vector2> Renderer::texCoordOffsets;
std::vector<Vector2> Renderer::texCoordScales;
std::vector<Vector3> Renderer::colors;
std::vector<U32> Renderer::textureIds;

bool Renderer::Initialize()
{
#ifdef DV_DEBUG
	std::cout << "Initializing Renderer..." << std::endl;
#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	defaultTexture = Resources::GetTexture("square");

	positionBuffer.Create(0, DataType::VECTOR2, positions, static_cast<U32>(CountOf(positions) * sizeof(Vector2)), false);
	texCoordsBuffer.Create(1, DataType::VECTOR2, texCoords, static_cast<U32>(CountOf(texCoords) * sizeof(Vector2)), false);
	offsetsBuffer.Create(2, DataType::VECTOR3, offsets.data(), static_cast<U32>(offsets.capacity() * sizeof(Vector3)), true);
	scalesBuffer.Create(3, DataType::VECTOR2, scales.data(), static_cast<U32>(scales.capacity() * sizeof(Vector2)), true);
	texCoordOffsetsBuffer.Create(4, DataType::VECTOR2, texCoordOffsets.data(), static_cast<U32>(texCoordOffsets.capacity() * sizeof(Vector2)), true);
	texCoordScalesBuffer.Create(5, DataType::VECTOR2, texCoordScales.data(), static_cast<U32>(texCoordScales.capacity() * sizeof(Vector2)), true);
	colorsBuffer.Create(6, DataType::VECTOR3, colors.data(), static_cast<U32>(colors.capacity() * sizeof(Vector3)), true);
	textureIdsBuffer.Create(7, DataType::UINT, textureIds.data(), static_cast<U32>(textureIds.capacity() * sizeof(U32)), true);

	glCreateBuffers(1, &textureBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, Resources::textureHandles.size() * sizeof(U64), Resources::textureHandles.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureBuffer);

	I32 success;
	C8 infoLog[512];

	U32 vertexShader;
	std::string vertexShaderSourceString = Resources::ReadFile("assets/sprite.vert");
	const C8* vertexShaderSource = vertexShaderSourceString.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Vertex shader compilation failed: " << infoLog << std::endl;
		return false;
	}

	U32 fragmentShader;
	std::string fragmentShaderSourceString = Resources::ReadFile("assets/sprite.frag");
	const C8* fragmentShaderSource = fragmentShaderSourceString.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "Fragment shader compilation failed: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		return false;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "Shader program linking failed: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	offsets.resize(MaxNotes, { -100.0f, -100.0f, 0.0f });
	scales.resize(MaxNotes, { 1.0f, 1.0f });
	texCoordOffsets.resize(MaxNotes, { 0.0f, 0.0f });
	texCoordScales.resize(MaxNotes, { 1.0f, 1.0f });
	colors.resize(MaxNotes, { 0.0f, 0.0f, 0.0f });
	textureIds.resize(MaxNotes, 0);

	return true;
}

void Renderer::Shutdown()
{
	positionBuffer.Destroy();
	texCoordsBuffer.Destroy();
	offsetsBuffer.Destroy();
	scalesBuffer.Destroy();
	texCoordOffsetsBuffer.Destroy();
	texCoordScalesBuffer.Destroy();
	colorsBuffer.Destroy();
	textureIdsBuffer.Destroy();

	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);
}

void Renderer::Update(Vector2 velocity, Window& settingsWindow, Window& visualizerWindow)
{
	Settings& settings = Visualizer::GetSettings();

	switch (settings.scrollDirection)
	{
	case ScrollDirection::Up:
	case ScrollDirection::Down: {
		positions[0].x = settings.noteWidth;
		positions[1].x = settings.noteWidth;
		positions[2].x = -settings.noteWidth;
		positions[3].x = -settings.noteWidth;
		positions[0].y = settings.noteHeight;
		positions[1].y = -settings.noteHeight;
		positions[2].y = -settings.noteHeight;
		positions[3].y = settings.noteHeight;
	} break;
	case ScrollDirection::Left:
	case ScrollDirection::Right: {
		positions[0].x = settings.noteHeight;
		positions[1].x = settings.noteHeight;
		positions[2].x = -settings.noteHeight;
		positions[3].x = -settings.noteHeight;
		positions[0].y = settings.noteWidth;
		positions[1].y = -settings.noteWidth;
		positions[2].y = -settings.noteWidth;
		positions[3].y = settings.noteWidth;
	} break;
	}

	positionBuffer.Flush(positions, static_cast<U32>(CountOf(positions) * sizeof(Vector2)));

	for (Vector3& offset : offsets)
	{
		offset += velocity;
	}

	offsetsBuffer.Flush(offsets.data(), static_cast<U32>(offsets.capacity() * sizeof(Vector3)));
	scalesBuffer.Flush(scales.data(), static_cast<U32>(scales.capacity() * sizeof(Vector2)));
	texCoordOffsetsBuffer.Flush(texCoordOffsets.data(), static_cast<U32>(texCoordOffsets.capacity() * sizeof(Vector2)));
	texCoordScalesBuffer.Flush(texCoordScales.data(), static_cast<U32>(texCoordScales.capacity() * sizeof(Vector2)));
	colorsBuffer.Flush(colors.data(), static_cast<U32>(colors.capacity() * sizeof(Vector3)));
	textureIdsBuffer.Flush(textureIds.data(), static_cast<U32>(textureIds.capacity() * sizeof(U32)));

	visualizerWindow.SetClearColor(settings.backgroundColor);

	settingsWindow.Update();

	UI::Update(&settingsWindow);
	UI::Render(&settingsWindow);

	visualizerWindow.Update();

	UI::Update(&visualizerWindow);

	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, static_cast<I32>(offsets.size()));

	glBindVertexArray(0);

	UI::Render(&visualizerWindow);

	settingsWindow.Render();
	visualizerWindow.Render();
}

void Renderer::SpawnNote(Stats& stats, const Vector3& color, Texture* texture)
{
	if (texture == nullptr) { texture = defaultTexture; }

	Vector2 scale = { 1.0f, 1.0f };
	Vector2 texCoordOffset = { 0.0f, 0.0f };
	Vector2 texCoordScale = { 1.0f, 1.0f };
	Settings& settings = Visualizer::GetSettings();

	if (settings.noteSeparationMode != NoteSeparationMode::None && stats.lastIndex != U32_MAX)
	{
		Vector3 prevOffset = offsets[stats.lastIndex];

		F32 allowedDistance = settings.noteHeight * 2 + settings.noteGap;

		switch (settings.scrollDirection)
		{
		case ScrollDirection::Up: {
			F32 distance = prevOffset.y - stats.spawn.y;

			if (distance < allowedDistance)
			{
				F32 adjustment = allowedDistance - distance;
				F32 percent = adjustment / (settings.noteHeight * 2.0f);

				offsets[stats.lastIndex].y += adjustment / 2.0f;
				scales[stats.lastIndex].y -= percent;
				if (settings.noteSeparationMode == NoteSeparationMode::Cutoff)
				{
					texCoordOffsets[stats.lastIndex].y += percent;
					texCoordScales[stats.lastIndex].y -= percent;
				}
			}
		} break;
		case ScrollDirection::Down: {
			F32 distance = stats.spawn.y - prevOffset.y;

			if (distance < allowedDistance)
			{
				F32 adjustment = allowedDistance - distance;
				F32 percent = adjustment / (settings.noteHeight * 2.0f);

				offsets[stats.lastIndex].y -= adjustment / 2.0f;
				scales[stats.lastIndex].y -= percent;
				if (settings.noteSeparationMode == NoteSeparationMode::Cutoff)
				{
					texCoordScales[stats.lastIndex].y -= percent;
				}
			}
		} break;
		case ScrollDirection::Left: {
			F32 distance = stats.spawn.x - prevOffset.x;

			if (distance < allowedDistance)
			{
				F32 adjustment = allowedDistance - distance;
				F32 percent = adjustment / (settings.noteHeight * 2.0f);

				offsets[stats.lastIndex].x -= adjustment / 2.0f;
				scales[stats.lastIndex].x -= percent;
				if (settings.noteSeparationMode == NoteSeparationMode::Cutoff)
				{
					texCoordScales[stats.lastIndex].x -= percent;
				}
			}
		} break;
		case ScrollDirection::Right: {
			F32 distance = prevOffset.x - stats.spawn.x;

			if (distance < allowedDistance)
			{
				F32 adjustment = allowedDistance - distance;
				F32 percent = adjustment / (settings.noteHeight * 2.0f);

				offsets[stats.lastIndex].x += adjustment / 2.0f;
				scales[stats.lastIndex].x -= percent;
				if (settings.noteSeparationMode == NoteSeparationMode::Cutoff)
				{
					texCoordOffsets[stats.lastIndex].x += percent;
					texCoordScales[stats.lastIndex].x -= percent;
				}
			}
		} break;
		}
	}

	offsets[nextIndex] = stats.spawn;
	scales[nextIndex] = scale;
	colors[nextIndex] = color;
	texCoordOffsets[nextIndex] = texCoordOffset;
	texCoordScales[nextIndex] = texCoordScale;
	textureIds[nextIndex] = texture->id;

	stats.lastIndex = nextIndex;

	++nextIndex %= MaxNotes;
}

void Renderer::ClearNotes()
{
	for (Vector3& offset : offsets)
	{
		offset = { -100.0f, -100.0f };
	}
}