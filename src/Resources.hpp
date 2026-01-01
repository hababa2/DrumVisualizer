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

class Resources
{
public:
	static bool Initialize();
	static void Shutdown();

	static Texture* GetTexture(const std::string& name);
	static const std::vector<char*>& GetTextureNames();

	static std::string ReadFile(const std::string& path);
	static std::string ReadFile(const std::wstring& path);

private:
	static void LoadAssets();

	static void LoadTexture(const std::string& path);

	static std::string GetFileName(const std::string& path);

	static std::map<std::string, Texture> textures;

	static std::vector<char*> textureNames;
	static std::vector<U64> textureHandles;

	STATIC_CLASS(Resources);

	friend class Renderer;
};