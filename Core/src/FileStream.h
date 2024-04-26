#pragma once

#include <fstream>

class Buffer;

class FileStreamReader
{
public:
	FileStreamReader(const std::filesystem::path& path);
	~FileStreamReader();

	bool IsStreamGood();

	void SetStreamPosition(size_t pos);
	size_t GetStreamPosition();

	size_t GetFileSize();

	void Read(char* data, size_t size);
	void Read(Buffer& buffer);

	template<typename T>
		requires (!std::is_same_v<T, Buffer> && !std::is_same_v<T, std::string>)
	void Read(T& data)
	{
		Read(reinterpret_cast<char*>(&data), sizeof(data));
	}
private:
	std::ifstream m_Stream;
};

class FileStreamWriter
{
public:
	FileStreamWriter(const std::filesystem::path& path);
	~FileStreamWriter();

	bool IsStreamGood();
	void SetStreamPosition(size_t pos);
	size_t GetStreamPosition();

	void Write(const char* data, size_t size);
	void Write(const Buffer& buffer);

	template<typename T>
		requires (!std::is_same_v<T, Buffer> && !std::is_same_v<T, std::string>)
	void Write(const T& data)
	{
		Write(reinterpret_cast<const char*>(&data), sizeof(data));
	}
private:
	std::ofstream m_Stream;
};
