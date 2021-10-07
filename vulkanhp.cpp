#include "vulkanhp.h"

#include <iostream>

csl::vulkan::vulkan()
{
	init_vulkan();
}
csl::vulkan::~vulkan()
{
	instance_.destroy();
	
}


void csl::vulkan::init_vulkan()
{
	create_instance();
}

void csl::vulkan::create_instance()
{
	if(enable_validation_layers&&!check_validation_layer())
	{
		throw std::runtime_error("validation layers requested, but not available");
	}
	vk::ApplicationInfo application_info("VkApp",
		1, "Sky", 
		1, VK_API_VERSION_1_2);

	uint32_t extension_count = 0;
	auto data = glfwGetRequiredInstanceExtensions(&extension_count);

	auto instance_create_info = vk::InstanceCreateInfo({},
		&application_info, enable_validation_layers ?static_cast<uint32_t>(validation_layers.size()):0, {enable_validation_layers? validation_layers.data():nullptr},
		extension_count,
		data);
	instance_ = vk::createInstance(instance_create_info);
	auto extensions = vk::enumerateInstanceExtensionProperties();


}


void csl::vulkan::main_loop()
{
}
