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

	public:
		vulkan();

		
		void run();

		~vulkan();

	private:
		void init_vulkan();
		void create_instance();

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


	public:

	};
}




