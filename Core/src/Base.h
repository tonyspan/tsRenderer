#pragma once

#include <memory>

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename Other>
inline Ref<T> RefAs(const Ref<Other>& other)
{
	return std::static_pointer_cast<T>(other);
};

template <typename T, typename Other>
inline Ref<T> RefAs(Ref<Other>&& other)
{
	return std::static_pointer_cast<T>(other);
};

#define BIND_FUNC(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define DELETE_COPY_SEMANTICS(CLASS) \
CLASS(const CLASS&) = delete; \
CLASS& operator=(const CLASS&) = delete;

#define DELETE_MOVE_SEMANTICS(CLASS) \
CLASS(CLASS&&) = delete; \
CLASS& operator=(CLASS&&) = delete;

#define DELETE_COPY_AND_MOVE(CLASS) \
DELETE_COPY_SEMANTICS(CLASS) \
DELETE_MOVE_SEMANTICS(CLASS)
