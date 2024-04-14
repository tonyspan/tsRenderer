#pragma once

#include <cstdio>
#include <format>

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak();
#else
#include <signal.h>
#define DEBUG_BREAK() __builtin_debugtrap()
#endif

#if __has_include(<source_location>)
#include <source_location>
#define SRC_LOC() std::source_location loc = std::source_location::current(); \
	const char* func = loc.function_name(); \
	const char* file = loc.file_name(); \
	int line = int(loc.line())
#else
#define SRC_LOC() const char* func = __PRETTY_FUNCTION__; \
	const char* file = __FILE__; \
	int line = __LINE__
#endif

#define LOG(...) do { printf("" ##__VA_ARGS__); printf("\n"); } while(false)
#define LOG_TAGGED(TAG, ...) do { printf("%s: ", TAG); LOG(__VA_ARGS__); } while(false)

#define STR(...) #__VA_ARGS__
#define QUOTED(X) std::format("'{}'", X).data()

#define ASSERT(COND, ...) do { if(!(COND)) { SRC_LOC(); \
	LOG("Assertion failed: " STR(COND) ": %s. " ##__VA_ARGS__, func); DEBUG_BREAK(); } } while (false)
