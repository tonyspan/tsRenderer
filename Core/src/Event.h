#pragma once

// TODO: Seperate KeyCode, MouseButton enums from Input
#include "Input.h"

#include <cstdint>
#include <functional>
#include <concepts>

enum class EventType : int
{
	UNDEFINED = 0,
	QUIT,
	RESIZE,
	KEYPRESSED,
	MOUSEPRESSED,
	MOUSEMOTION
};

#define SET_EVENT_TYPE(CLASS, TYPE) \
		CLASS() : Event(TYPE) {} \
		static EventType GetStaticType() { return TYPE; }

struct Event
{
	Event(EventType type)
		: m_EventType(type) {}

	virtual ~Event() = default;

	EventType GetType() const { return m_EventType; }
private:
	EventType m_EventType = EventType::UNDEFINED;
};

struct ResizeEvent : public Event
{
	SET_EVENT_TYPE(ResizeEvent, EventType::RESIZE);

	uint32_t Width = 0;
	uint32_t Height = 0;
};

struct QuitEvent : public Event
{
	SET_EVENT_TYPE(QuitEvent, EventType::QUIT);
};

struct KeyPressedEvent : public Event
{
	SET_EVENT_TYPE(KeyPressedEvent, EventType::KEYPRESSED);

	KeyCode Keycode = KeyCode::NONE;
	int RepeatCount = 0;
};

struct MouseButtonPressedEvent : public Event
{
	SET_EVENT_TYPE(MouseButtonPressedEvent, EventType::MOUSEPRESSED);

	MouseButton Button = MouseButton::LMB;
};

struct MouseMotionEvent : public Event
{
	SET_EVENT_TYPE(MouseMotionEvent, EventType::MOUSEMOTION);

	glm::vec2 Position{};
};

class EventDispatcher
{
public:
	EventDispatcher(Event& event)
		: m_Event(event) {}

	template<std::derived_from<Event> T>
	void Dispatch(const std::function<void(T&)>& func)
	{
		//static_assert(!std::is_base_of_v<Event, T>);

		if (m_Event.GetType() == T::GetStaticType())
			func(static_cast<T&>(m_Event));
	}
private:
	Event& m_Event;
};