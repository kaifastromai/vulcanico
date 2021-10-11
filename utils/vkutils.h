#pragma once
#define GLFW_INCLUDE_NONE
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"
#include <vector>

#ifndef NDEBUG
constexpr bool kDebug = true;
#elif
constexpr bool kDebug = false;
#endif

inline void SkCheck(vk::Result r)  {
	if(r!=vk::Result::eSuccess)
	{
		
		throw vk::make_error_code(r);
	}
}
namespace sk
{
	constexpr uint32_t kWidth = 1000;
	constexpr uint32_t kHeight = 1000;
	class Glvk
	{
	public:
		Glvk(const uint32_t& width, const uint32_t& height, const std::string& title)
		{
			glfwInit();
			init_window(width, height, title);
			get_glfw_extensions_();
		}
		~Glvk()
		{
			glfwDestroyWindow(window);
			glfwTerminate();
		}

	private:
		GLFWwindow* window{};
		std::vector<const char*> glfw_extensions_;



	public:
		void init_window(uint32_t width, uint32_t height, const std::string title)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
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
		template<class T>
		struct Extent2d
		{
			T width;
			T height;
		};
		

		void create_surface(vk::Instance instance, VkSurfaceKHR* surface)
		{
			if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
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

		Extent2d<float> content_scale() {
			float x;
			float y;
			glfwGetWindowContentScale(window, &x, &y);
			return Extent2d{ x, y };
			
		}
		void set_key_callback(GLFWkeyfun callback) {
			glfwSetKeyCallback(window, callback);
		}
	

	};


	inline std::vector<char> read_shader(const std::string& path) {
		std::ifstream file(path,std::ios::binary);
		if(!file.is_open())
		{
		
			throw std::runtime_error("File could not be opened");
		}

		std::filesystem::path p{ path };
		auto size=std::filesystem::file_size(p);
		std::vector<char> buf(size);
		file.read(buf.data(), size);
		if(kDebug)
		{
			std::cout << "Successfully loaded file" << std::endl;
		}
		return buf;


		

	}



}
