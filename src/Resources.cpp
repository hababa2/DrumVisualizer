#include "Resources.hpp"

#include "GraphicsInclude.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <filesystem>
#include <iostream>
#include <fstream>

std::map<std::string, Texture> Resources::textures;

std::vector<char*> Resources::textureNames;
std::vector<U64> Resources::textureHandles;

bool Resources::Initialize()
{
#ifdef DV_DEBUG
	std::cout << "Loading Assets..." << std::endl;
#endif

	stbi_set_flip_vertically_on_load(true);

	LoadAssets();

	return true;
}

void Resources::Shutdown()
{

}

Texture* Resources::GetTexture(const std::string& name)
{
	std::map<std::string, Texture>::iterator result = textures.find(name);

	if (result == textures.end()) { return nullptr; }

	return &result->second;
}

const std::vector<char*>& Resources::GetTextureNames()
{
	return textureNames;
}

std::string Resources::ReadFile(const std::string& path)
{
	std::ifstream file(path);
	std::string data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	return data;
}

std::string Resources::ReadFile(const std::wstring& path)
{
	std::ifstream file(path);
	std::string data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	return data;
}

void Resources::LoadAssets()
{
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("assets"))
	{
		std::string path = entry.path().string();

		std::string ext = path.substr(path.find_last_of('.') + 1);

		switch (HashCI(ext.c_str(), ext.length()))
		{
			//Textures
		case "jpg"_Hash:
		case "jpeg"_Hash:
		case "png"_Hash:
		case "bmp"_Hash:
		case "tga"_Hash:
		case "jfif"_Hash:
		case "tiff"_Hash: {
			LoadTexture(entry.path().string());
		} break;
		}
	}
}

void Resources::LoadTexture(const std::string& path)
{
	I32 width, height, comp;
	U8* data = stbi_load(path.c_str(), &width, &height, &comp, 4);
	if (!data)
	{
		std::cout << "Failed To Load Texture: " << path << std::endl;
		return;
	}

	U32 textureLocation;
	glGenTextures(1, &textureLocation);
	glBindTexture(GL_TEXTURE_2D, textureLocation);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	stbi_image_free(data);

	U64 textureHandle = glGetTextureHandleARB(textureLocation);
	glMakeTextureHandleResidentARB(textureHandle);
	textureHandles.push_back(textureHandle);

	Texture texture{};
	texture.name = GetFileName(path);
	texture.width = width;
	texture.height = height;
	texture.id = (U32)textures.size();

	char* nameStorage = new char[texture.name.size() + 1];
	memcpy(nameStorage, texture.name.c_str(), texture.name.size());
	nameStorage[texture.name.size()] = '\0';

	textureNames.push_back(nameStorage);
	textures.insert({ texture.name, texture });
}

std::string Resources::GetFileName(const std::string& path)
{
	U64 end = path.find_last_of('.');
#ifdef DV_PLATFORM_WINDOWS
	U64 start = path.find_last_of('\\', end) + 1;
#else
	U64 start = path.find_last_of('/', end) + 1;
#endif

	return path.substr(start, end - start);
}