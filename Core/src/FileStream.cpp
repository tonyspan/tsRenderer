#include "FileStream.h"

#include "Buffer.h"

#include <filesystem>
#include <cassert>

FileStreamReader::FileStreamReader(const std::filesystem::path& path)
	: m_Stream(path.string().data(), std::ios::in | std::ios::binary/*| std::ios::ate */)
{
}

FileStreamReader::~FileStreamReader()
{
	m_Stream.close();
}

bool FileStreamReader::IsStreamGood()
{
	return static_cast<bool>(m_Stream);
}

void FileStreamReader::SetStreamPosition(size_t pos)
{
	m_Stream.seekg(static_cast<std::streampos>(pos));
}

size_t FileStreamReader::GetStreamPosition()
{
	return static_cast<size_t>(m_Stream.tellg());
}

size_t FileStreamReader::GetFileSize()
{
	// In case std::ios::ate is used
	SetStreamPosition(0);

	m_Stream.seekg(0, std::ios::end);
	auto size = GetStreamPosition();

	// Reset
	SetStreamPosition(0);

	return size;
}

void FileStreamReader::Read(char* data, size_t size)
{
	assert(data);

	m_Stream.read(data, static_cast<std::streamsize>(size));
}

void FileStreamReader::Read(Buffer& buffer)
{
	if (!buffer)
		return;

	Read(buffer.As<char*>(), static_cast<size_t>(buffer.GetSize()));
}

FileStreamWriter::FileStreamWriter(const std::filesystem::path& path)
	: m_Stream(path.string().data(), std::ios::out | std::ios::binary)
{
}

FileStreamWriter::~FileStreamWriter()
{
	m_Stream.close();
}

bool FileStreamWriter::IsStreamGood()
{
	return static_cast<bool>(m_Stream);
}

void FileStreamWriter::SetStreamPosition(size_t pos)
{
	m_Stream.seekp(static_cast<std::streampos>(pos));
}

size_t FileStreamWriter::GetStreamPosition()
{
	return m_Stream.tellp();
}

void FileStreamWriter::Write(const char* data, size_t size)
{
	assert(data);

	if (0 < size)
		m_Stream.write(data, static_cast<std::streamsize>(size));
}

void FileStreamWriter::Write(const Buffer& buffer)
{
	if (!buffer)
		return;

	Write(buffer.As<const char*>(), static_cast<size_t>(buffer.GetSize()));
}
