#include "Buffer.hpp"

#include "GraphicsInclude.hpp"

void Buffer::Create(U32 location, DataType type, void* data, U64 size, bool instance)
{
	this->location = location;
	this->type = type;
	this->data = data;
	this->size = size;
	this->instance = instance;

	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, instance ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	glEnableVertexAttribArray(location);

	switch (type)
	{
	case DataType::INT: { glVertexAttribIPointer(location, 1, GL_INT, 0, 0); } break;
	case DataType::UINT: { glVertexAttribIPointer(location, 1, GL_UNSIGNED_INT, 0, 0); } break;
	case DataType::FLOAT: { glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE, 0, 0); } break;
	case DataType::VECTOR2: { glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0); } break;
	case DataType::VECTOR3: { glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0); } break;
	case DataType::VECTOR4: { glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0); } break;
	}

	glVertexAttribDivisor(location, instance);
}

void Buffer::Destroy()
{
	glDeleteBuffers(1, &id);
}

void Buffer::Flush(void* data, U64 size)
{
	this->data = data;
	this->size = size;

	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, instance ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}