#include "Context.h"

#include "Window.h"
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"

#include "Log.h"

struct ContextData
{
	Scope<Instance> Inst;
	Scope<Surface> Surf;
	Scope<Device> Dev;
	Scope<Swapchain> SwapChain;

	Scope<DescriptorPool> DescPool;

	void Init(const Window& window)
	{

		Inst = CreateScope<Instance>(window);
		Surf = CreateScope<Surface>(*Inst, window);
		Dev = CreateScope<Device>(*Inst, *Surf);

		{
			SwapchainDescription desc = {};

			desc.Width = window.GetWidth();
			desc.Height = window.GetHeight();
			desc.VSync = window.HasVSync();

			SwapChain = CreateScope<Swapchain>(*Dev, *Surf, desc);
		}

		DescPool = CreateScope<DescriptorPool>(*Dev);
	}

	void Shutdown()
	{
		DescPool.reset();
		SwapChain.reset();
		Dev.reset();
		Surf.reset();
		Inst.reset();
	}
};

static ContextData* s_Data = nullptr;

void Context::Init(const Window& window)
{
	s_Data = new ContextData();

	s_Data->Init(window);
}

void Context::Shutdown()
{
	s_Data->Shutdown();

	delete s_Data;
	s_Data = nullptr;
}

Instance& Context::GetInstance()
{
	ASSERT(s_Data && s_Data->Inst);
	return *s_Data->Inst;
}

//Surface& Context::GetSurface()
//{
//	ASSERT(s_Data && s_Data->Surf);
//	return *s_Data->Surf;
//}

Device& Context::GetDevice()
{
	ASSERT(s_Data && s_Data->Dev);
	return *s_Data->Dev;
}

Swapchain& Context::GetSwapchain()
{
	ASSERT(s_Data && s_Data->SwapChain);
	return *s_Data->SwapChain;
}

DescriptorPool& Context::GetDescriptorPool()
{
	ASSERT(s_Data && s_Data->DescPool);
	return *s_Data->DescPool;
}