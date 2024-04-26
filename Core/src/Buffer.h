#pragma once

#include <cstdint>
#include <memory>
#include <cassert>

// Non-owning Buffer
class Buffer
{
public:
	Buffer() = default;

	Buffer(void* data, uint64_t size);

	static Buffer Copy(const void* data, uint64_t size);
	static Buffer Copy(const Buffer& buffer);

	void Allocate(uint64_t size);
	void Release();
	void ZeroInit();

	explicit operator bool() const { return m_Data; }

	uint8_t& operator[](uint64_t index)
	{
		assert(index < m_Size);

		return m_Data[index];
	}

	const uint8_t& operator[](uint64_t index) const
	{
		assert(index < m_Size);

		return m_Data[index];
	}

	template <typename T>
	T& Read(uint64_t offset = 0)
	{
		assert(offset < m_Size);

		return *(T*)(m_Data + offset);
	}

	void Write(void* data, uint64_t size, uint64_t offset = 0)
	{
		assert(offset + size <= m_Size);

		memcpy(m_Data + offset, data, size);
	}

	template <typename T>
		requires std::is_pointer_v<T>
	T As()
	{
		return reinterpret_cast<T>(m_Data);
	}

	template <typename T>
		requires std::is_pointer_v<T>&& std::is_const_v<std::remove_pointer_t<T>>
	T As() const
	{
		return reinterpret_cast<T>(m_Data);
	}

	const uint64_t GetSize() const;
private:
	uint8_t* m_Data = nullptr;
	uint64_t m_Size = 0;
};