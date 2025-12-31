#pragma once

#include "Defines.hpp"

#include <string>

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
	//ResourceRef<Texture> texture = nullptr;
	F32 ascent = 0.0f;
	F32 descent = 0.0f;
	F32 lineGap = 0.0f;
	F32 scale = 0.0f;
	U32 glyphSize = 0;

	Glyph glyphs[96];
};

class UI
{
public:
	static bool Initialize();
	static void Shutdown();

private:


	STATIC_CLASS(UI)
};