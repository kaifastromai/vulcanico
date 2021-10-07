#pragma once
#include<vulkan\vulkan.hpp>
#include "vkutils.h"
#include <algorithm>

namespace csl
{

	

	const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
	constexpr bool enable_validation_layers = false;
#else
	constexpr bool enable_validation_layers = true;
#endif

	class vulkan
	{

		
	//public vars
	private:
		glvk glvk_=glvk(WIDTH, HEIGHT, "Vulkan");
		vk::Instance instance_;
		vk::PhysicalDevice physical_device_ = VK_NULL_HANDLE;

	public:
		vulkan();

		
		void run();

		~vulkan();

	private:
		void init_vulkan();
		void create_instance();

		static bool check_validation_layer();
		std::vector<const char*> get_required_extensions();
		VkDebugUtilsMessengerEXT debug_messenger_;
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
			VkDebugUtilsMessageTypeFlagsEXT message_type,  VkDebugUtilsMessengerCallbackDataEXT const * p_call_back, void* user_data);
		
		void setup_debug_messenger();
		void pick_physical_device();
		bool is_device_suitable(vk::PhysicalDevice device);
	void main_loop();



	public:

	};
}




