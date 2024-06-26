#pragma once

#include <string>
#include <filesystem>

class Buffer;

std::string GetProjectDirectory();

void NormalizePath(std::filesystem::path& path);

bool ReadFromFile(Buffer& buffer, const std::filesystem::path& path);
bool WriteToFile(const std::filesystem::path& path, const Buffer& buffer);

// Taken from: https://www.strchr.com/hash_functions
uint64_t HashString(const std::string& str);

// Convenient way to delete .spv files after each run
void Delete(const std::filesystem::path& path, const std::string& what);