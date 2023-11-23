#pragma once

#include <string>

std::string GetProjectDirectory();

void NormalizePath(std::string& path);

// Convenient way to delete .spv files after each run
void Delete(const std::string& path, const std::string& what);