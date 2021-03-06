#pragma once
#define GLFW_INCLUDE_NONE
#include <string>
#include "GLFW/glfw3.h"
#include <vector>
namespace csl
{
	const uint32_t WIDTH = 1200;
	const uint32_t HEIGHT = 1200;
	class glvk
	{
	public:
		glvk(uint32_t width, uint32_t height, const std::string title)
		{
			glfwInit();
			init_window(width, height,title);
			get_glfw_extensions_();
		}
	~glvk()
		{
		glfwDestroyWindow(window);
		glfwTerminate();
		}
	private:
		GLFWwindow* window;
		 std::vector<const char*> glfw_extensions_;



	public:
		void init_window(uint32_t width, uint32_t height,const std::string title)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		}

		[[nodiscard]] bool window_should_close() const
		{
			return glfwWindowShouldClose(window);
		}
		std::vector<const char*> get_glfw_extensions_()
		{
			uint32_t extension_count = 0;
			auto data = glfwGetRequiredInstanceExtensions(&extension_count);
			glfw_extensions_.resize(extension_count);
			for (size_t i = 0; i < extension_count; i++)
			{
				glfw_extensions_[i] = data[i];
			}

			return glfw_extensions_;
		}
		void poll_events()
		{
			glfwPollEvents();
		}
	public:
		std::vector<const char*> get_extensions() const
		{
			return glfw_extensions_;
		}

		void create_surface(vk::Instance instance, VkSurfaceKHR *surface)
		{
			if(glfwCreateWindowSurface(instance,window,nullptr,surface)!=VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create window surface!");
			}
		}
		vk::Extent2D get_framebuffer_size()
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			return vk::Extent2D(width, height);
		}
	};
}