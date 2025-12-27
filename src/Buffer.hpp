#pragma once

#include "Defines.hpp"

enum class DataType
{
	INT,
	FLOAT,
	VECTOR2,
	VECTOR3,
	VECTOR4
};

struct Buffer
{
	void Create(U32 location, DataType type, void* data, U64 size, bool instance);
	void Destroy();

	void Flush(void* data, U64 size);

private:
	U32 id{ U32_MAX };
	U32 location;
	DataType type;
	void* data;
	U64 size;
	bool instance;
};