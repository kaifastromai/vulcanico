#pragma once
#include<vulkan\vulkan.hpp>
#include "vkutils.h"
#include <algorithm>

<<<<<<< HEAD
namespace csl
{

	

=======



namespace csl
{

>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
	const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
	constexpr bool enable_validation_layers = false;
#else
	constexpr bool enable_validation_layers = true;
#endif

	class vulkan
	{
<<<<<<< HEAD

		
=======
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
	//public vars
	private:
		glvk glvk_=glvk(WIDTH, HEIGHT, "Vulkan");
		vk::Instance instance_;
<<<<<<< HEAD
		vk::PhysicalDevice physical_device_ = VK_NULL_HANDLE;
=======
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb

	public:
		vulkan();

		
		void run();

		~vulkan();

	private:
		void init_vulkan();
		void create_instance();

<<<<<<< HEAD
		static bool check_validation_layer();
		std::vector<const char*> get_required_extensions();
		VkDebugUtilsMessengerEXT debug_messenger_;
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
			VkDebugUtilsMessageTypeFlagsEXT message_type,  VkDebugUtilsMessengerCallbackDataEXT const * p_call_back, void* user_data);
		
		void setup_debug_messenger();
		void pick_physical_device();
		bool is_device_suitable(vk::PhysicalDevice device);
	void main_loop();



=======
		static bool check_validation_layer()
		{
			auto layer_props = vk::enumerateInstanceLayerProperties();

			for(const auto lname:validation_layers)
			{
				bool layer_found = false;
				for(const auto& lprop:layer_props)
				{
					if(strcmp((char*)lprop.layerName.data(),lname))
					{
						layer_found = true;
						break;
					}
				}
				if(!layer_found)
				{
					return false;
				}
			}
			return true;
		}

		 std::vector<const char*> get_required_extensions()
		{
			auto exts=glvk_.get_extensions();
			if(enable_validation_layers)
			{
				exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}
	void main_loop();


>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
	public:

	};
}




