#include "Buffer.h"

Buffer::Buffer(void* data, uint64_t size)
	: m_Data((uint8_t*)data)
	, m_Size(size)
{
}

Buffer Buffer::Copy(const void* data, uint64_t size)
{
	Buffer buffer;

	buffer.Allocate(size);
	memcpy(buffer.m_Data, data, size);

	return buffer;
}

Buffer Buffer::Copy(const Buffer& buffer)
{
	return Copy(buffer.m_Data, buffer.m_Size);
}

void Buffer::Allocate(uint64_t size)
{
	Release();

	if (0 == size)
		return;

	m_Data = new uint8_t[size];
	m_Size = size;
}

void Buffer::Release()
{
	delete[] m_Data;
	m_Data = nullptr;
	m_Size = 0;
}

void Buffer::ZeroInit()
{
	if (m_Data)
		memset(m_Data, 0, m_Size);
}

const uint64_t Buffer::GetSize() const
{
	assert(0 != m_Size);

	return m_Size;
}
