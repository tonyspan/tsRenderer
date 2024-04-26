#pragma once

#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

class Thread
{
	enum class State : int { IDLE = 0, WORKING };
public:
	using Job = std::function<void()>;

	Thread() = default;
	~Thread() = default;

	void Start();
	void Stop();

	void Submit(const Job& job);

	const size_t GetJobsCount() const;
	const bool IsIdle() const;
private:
	void Join();
	void Wait();
private:
	std::jthread m_Handle;

	std::queue<Job> m_JobsQueue;
	std::mutex m_QueueMutex;
	std::condition_variable m_CV;

	Thread::State m_State = Thread::State::IDLE;
	bool m_ShouldStop = false;
};
