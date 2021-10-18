#pragma once
#define GLFW_INCLUDE_NONE
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"
#include <vector>
#include <ranges>
#include <vk_mem_alloc.h>


#ifndef NDEBUG
constexpr bool kDebug = true;
#elif
constexpr bool kDebug = false;
#endif



namespace sk
{
	//This needs to be refactored eventually...
	struct AllocatorCreateInfo
	{
	private:
		VmaAllocatorCreateInfo _allocator_create_info{};
	public:
		AllocatorCreateInfo(vk::PhysicalDevice physical_device, vk::Device device, vk::Instance instance) {
			_allocator_create_info.instance = instance;
			_allocator_create_info.physicalDevice = physical_device;
			_allocator_create_info.device = device;
		}
		operator VmaAllocatorCreateInfo*()  { return &_allocator_create_info; }


	};
	
	struct VkAllocator
	{

		inline static VmaAllocator allocator;

		VkAllocator() {
			allocator = nullptr;
		}
		static vk::Result init(AllocatorCreateInfo& info) {
			return vk::Result(vmaCreateAllocator(info, &allocator));
		}

		~VkAllocator() {
			vmaDestroyAllocator(allocator);
		}

		operator VmaAllocator() const {
			return allocator;
		}


	};

	struct VkAllocatedBuffer
	{
		
		VkBuffer buffer;
		VmaAllocation allocation;
		~VkAllocatedBuffer() {
			vmaDestroyBuffer(VkAllocator::allocator, buffer, allocation);
		}
	};
	
	
	//simpler version of vkbootstrap->group of helper function to ease dev
	namespace vkbs {
		class InstanceBuilder
		{
			vk::Instance _instance;
			vk::InstanceCreateInfo _instance_flags;
			vk::ApplicationInfo _app_info;
			std::vector<const char*> _enabled_extensions;
			std::vector<char*> get_supported_extensions() const {
				auto supported_extension_props = vk::enumerateInstanceLayerProperties();
				
			}
			
			
		};
		
	}
	//forward declaration
	class Skie;
	//Need to account for window size vs. framebuffer size later
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
		

		void create_surface(vk::Instance instance, VkSurfaceKHR* surface) const {
			if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create window surface!");
			}
		}
		vk::Extent2D get_framebuffer_size() const {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			return vk::Extent2D(width, height);

		}

		Extent2d<float> content_scale() const {
			float x;
			float y;
			glfwGetWindowContentScale(window, &x, &y);
			return Extent2d{ x, y };
			
		}
		void set_key_callback(GLFWkeyfun callback, sk::Skie* ctx) const {
			glfwSetWindowUserPointer(window, ctx);
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
