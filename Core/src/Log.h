#pragma once

#include <cstdio>
#include <source_location>

#define LOCATION() std::source_location::current()

#define FUNC() LOCATION().function_name()

#define LOG(...) do { printf(__VA_ARGS__); printf("\n"); } while(false)

#define STR(...) #__VA_ARGS__

#define ASSERT(COND, ...) if(!(COND)) { LOG("%s: " __VA_ARGS__, FUNC());  __debugbreak(); }