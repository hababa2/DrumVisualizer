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
	Buffer(U32 location, DataType type, void* data, U32 size, bool instance);
	~Buffer();

	void Flush(void* data, U32 size);

private:
	U32 id{ U32_MAX };
	U32 location;
	DataType type;
	void* data;
	U32 size;
	bool instance;
};