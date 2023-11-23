#pragma once

// Don't include this in any header file

#include "Base.h"

#if 0
#include "Window.h"
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"
#else
class Window;

class Instance;
//class Surface;
class Device;
class Swapchain;
class DescriptorPool;
#endif

class Context
{
public:
	static void Init(const Window& window);
	static void Shutdown();

	static Instance& GetInstance();
	//static Surface& GetSurface();
	static Device& GetDevice();
	static Swapchain& GetSwapchain();

	static DescriptorPool& GetDescriptorPool();
};