#include "Thread.h"

void Thread::Start()
{
	m_Handle = std::jthread([&]()
		{
			while (true)
			{
				Job job;
				{
					m_State = Thread::State::IDLE;

					std::unique_lock lock(m_QueueMutex);
					m_CV.wait(lock, [this] { return !m_JobsQueue.empty() || m_ShouldStop; });

					if (m_JobsQueue.empty() && m_ShouldStop)
						break;

					job = m_JobsQueue.front();

					if (job)
					{
						m_State = Thread::State::WORKING;
						job();
						m_State = Thread::State::IDLE;
					}

					m_JobsQueue.pop();

					if (m_JobsQueue.empty())
						m_CV.notify_one();
				}
			}
		});
}

void Thread::Stop()
{
	Wait();

	{
		std::scoped_lock lock(m_QueueMutex);

		m_ShouldStop = true;
		m_CV.notify_one();
	}

	Join();
}

void Thread::Submit(const Job& job)
{
	std::scoped_lock lock(m_QueueMutex);

	m_JobsQueue.push(job);
	m_CV.notify_one();
}

const size_t Thread::GetJobsCount() const
{
	return m_JobsQueue.size();
}

const bool Thread::IsIdle() const
{
	return Thread::State::IDLE == m_State;
}

void Thread::Join()
{
	if (m_Handle.joinable())
		m_Handle.join();
}

void Thread::Wait()
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_CV.wait(lock, [this] { return m_JobsQueue.empty(); });
}
