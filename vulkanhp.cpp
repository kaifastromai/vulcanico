#include "vulkanhp.h"

#include <iostream>

<<<<<<< HEAD
PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance                                 instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger)
{
	return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance                    instance,
	VkDebugUtilsMessengerEXT      messenger,
	VkAllocationCallbacks const* pAllocator)
{
	return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}


=======
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
csl::vulkan::vulkan()
{
	init_vulkan();
}
csl::vulkan::~vulkan()
{
<<<<<<< HEAD
	device_.destroy();

	if (enable_validation_layers) {
		instance_.destroyDebugUtilsMessengerEXT(debug_messenger_);
	}
	instance_.destroy();

=======
	instance_.destroy();
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
	
}


void csl::vulkan::init_vulkan()
{
	create_instance();
<<<<<<< HEAD
	setup_debug_messenger();
	pick_physical_device();
	create_logical_device();
=======
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
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

<<<<<<< HEAD
	auto extensions = get_required_extensions();




	auto instance_create_info = vk::InstanceCreateInfo({},
		&application_info, enable_validation_layers ?static_cast<uint32_t>(validation_layers.size()):0, {enable_validation_layers? validation_layers.data():nullptr},
		extensions.size(),
		extensions.data());

	instance_ = vk::createInstance(instance_create_info);
=======
	uint32_t extension_count = 0;
	auto data = glfwGetRequiredInstanceExtensions(&extension_count);

	auto instance_create_info = vk::InstanceCreateInfo({},
		&application_info, enable_validation_layers ?static_cast<uint32_t>(validation_layers.size()):0, {enable_validation_layers? validation_layers.data():nullptr},
		extension_count,
		data);
	instance_ = vk::createInstance(instance_create_info);
	auto extensions = vk::enumerateInstanceExtensionProperties();
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb


}

<<<<<<< HEAD
inline bool csl::vulkan::check_validation_layer()
{
	auto layer_props = vk::enumerateInstanceLayerProperties();
	for (auto l : layer_props) {
		std::cout << l.layerName << std::endl;
	}




	for (const auto lname : validation_layers)
	{
		bool layer_found = false;
		for (const auto& lprop : layer_props)
		{
			if (strcmp((char*)lprop.layerName.data(), lname))
			{
				layer_found = true;
				break;
			}
		}
		if (!layer_found)
		{
			return false;
		}
	}
	return true;
}


inline std::vector<const char*> csl::vulkan::get_required_extensions()
{
	auto exts = glvk_.get_extensions();
	if (enable_validation_layers)
	{
		exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return exts;
}

VKAPI_ATTR VkBool32 VKAPI_CALL csl::vulkan::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,  VkDebugUtilsMessengerCallbackDataEXT const* p_call_back, void* user_data)
{
	

		if (message_severity >= (uint32_t)vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
			std::cout << p_call_back->pMessage << std::endl;
		}
		return VK_FALSE;
		

	
}

inline void csl::vulkan::setup_debug_messenger() {
	auto severity_flags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
	auto message_flags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	auto info = vk::DebugUtilsMessengerCreateInfoEXT({},severity_flags, message_flags
		, &debug_callback);

	pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance_.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
	if(!pfnVkCreateDebugUtilsMessengerEXT)
	{
		throw std::runtime_error("Could not create debug messenger");
	}
	pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance_.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
	if(!pfnVkDestroyDebugUtilsMessengerEXT)
	{
		throw std::runtime_error("Unable to find pfnDestroyDebugUtilsMessengerEXt extension ");
	}

    debug_messenger_=	instance_.createDebugUtilsMessengerEXT(info);
}

void csl::vulkan::pick_physical_device()
{
	auto devices = instance_.enumeratePhysicalDevices();
	if (devices.size() == 0)
	{
		throw std::runtime_error("Failed to create physical device");
	}
	for(const auto& device:devices)
	{
		if(is_device_suitable(device))
		{
			physical_device_ = device;
			break;
		}
	}
	if((void*)physical_device_==nullptr)
	{
		throw std::runtime_error(
			"failed to find suitable GPU"
		);
	}
}

bool csl::vulkan::is_device_suitable(vk::PhysicalDevice device)
{
	auto indices = find_queue_families(device);
	
	return indices.b_complete();

}

csl::vulkan::queue_find_queue_families csl::vulkan::find_queue_families(vk::PhysicalDevice device)
{
	queue_find_queue_families indices;
	auto queue_families = device.getQueueFamilyProperties();

	int i = 0;
	for(const auto& queue_fam:queue_families)
	{
		if(queue_fam.queueFlags==vk::QueueFlagBits::eGraphics)
		{
			indices.graphics_family = i;
		}
		if (indices.b_complete()) break;
		i++;
	}
	return indices;
}

void csl::vulkan::create_logical_device()
{
	auto indices = find_queue_families(physical_device_);
	float queue_priority = 1.0f;
	auto device_queue_info = vk::DeviceQueueCreateInfo({}, indices.graphics_family.value(), 1,&queue_priority);
	vk::PhysicalDeviceFeatures device_features;
	vk::DeviceCreateInfo device_create_info({},1,&device_queue_info);
	device_create_info.pEnabledFeatures = &device_features;

	device_=physical_device_.createDevice(device_create_info);

}

void csl::vulkan::main_loop()
{
	

=======

void csl::vulkan::main_loop()
{
>>>>>>> 82af8a3c4fd6fdc9b53ee95b7a4ab081c40066fb
}
