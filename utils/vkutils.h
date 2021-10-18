#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
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
	struct VkAllocatedImage
	{
		vk::Image image;
		VmaAllocation allocation;
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

	namespace utils
	{
		inline std::vector<char> read_shader(const std::string& path) {
			std::ifstream file(path, std::ios::binary);
			if (!file.is_open())
			{

				throw std::runtime_error("File could not be opened");
			}

			std::filesystem::path p{ path };
			auto size = std::filesystem::file_size(p);
			std::vector<char> buf(size);
			file.read(buf.data(), size);
			if (kDebug)
			{
				std::cout << "Successfully loaded file" << std::endl;
			}
			return buf;




		}
	}
	


}
