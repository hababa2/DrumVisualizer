#pragma once

#include "Defines.hpp"

#include <map>
#include <string>
#include <vector>

struct Texture
{
	std::string name{};
	U32 id;
	U32 width;
	U32 height;
};

struct Glyph
{
	F32 advance = 1.0f;
	F32 leftBearing = 0.0f;
	F32 x = 0.0f;
	F32 y = 0.0f;

	F32 kerning[96]{ 0.0f };
};

struct Font
{
	std::string name{};
	Texture* texture = nullptr;
	F32 ascent = 0.0f;
	F32 descent = 0.0f;
	F32 lineGap = 0.0f;
	F32 scale = 0.0f;
	U32 glyphSize = 0;

	Glyph glyphs[96];
};

class Resources
{
public:
	static bool Initialize();
	static void Shutdown();

	static Texture* GetTexture(const std::string& name);

private:
	static void LoadAssets();

	static void LoadTexture(const std::string& path);
	static void LoadFont(const std::string& path);

	static std::string GetFileName(const std::string& path);

	static std::map<std::string, Texture> textures;
	static std::map<std::string, Font> fonts;

	static std::vector<U64> textureHandles;

	STATIC_CLASS(Resources);

	friend class Renderer;
};