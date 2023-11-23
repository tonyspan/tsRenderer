#pragma once

#include "Timer.h"

#include <string>
#include <unordered_map>

class PerFramePerfProfiler
{

public:
	struct FrameData
	{
		float Time;
	};

	static void SetPerFrameData(const std::string& name, float time)
	{
		if (!s_PerFrameData.contains(name))
		{
			s_PerFrameData[name] = { .Time = time };
			return;
		}

		s_PerFrameData.at(name).Time = time;
	}

	static const std::unordered_map<std::string, FrameData>& GetPerFrameData() { return s_PerFrameData; }

	static void Clear() { s_PerFrameData.clear(); }
private:
	static inline std::unordered_map<std::string, FrameData> s_PerFrameData;
};

class ScopedPerfTimer
{
public:
	ScopedPerfTimer(const std::string_view name)
		: m_Name(name)
	{
	}

	~ScopedPerfTimer()
	{
		float time = m_Timer.ElapsedMS();
		PerFramePerfProfiler::SetPerFrameData(m_Name, time);
	}
private:
	std::string m_Name;
	Timer m_Timer;
};

#define PROFILE_SCOPE(name) ::ScopedPerfTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() ::ScopedPerfTimer timer##__LINE__(__FUNCTION__)