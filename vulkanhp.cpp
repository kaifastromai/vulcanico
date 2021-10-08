#include "vulkanhp.h"
#include <set>
#include <iostream>

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

namespace csl {
	csl::vulkan::vulkan()
	{
		init_vulkan();
	}
	csl::vulkan::~vulkan()
	{
		instance_.destroySurfaceKHR(surface_);
		device_.destroy();

		if (enable_validation_layers) {
			instance_.destroyDebugUtilsMessengerEXT(debug_messenger_);
		}
		instance_.destroy();


	}


	void csl::vulkan::init_vulkan()
	{
		create_instance();
		setup_debug_messenger();
		glvk_.create_surface(instance_,&surface_);
		pick_physical_device();
		create_logical_device();
	}

	void csl::vulkan::create_instance()
	{
		if (enable_validation_layers && !check_validation_layer())
		{
			throw std::runtime_error("validation layers requested, but not available");
		}
		vk::ApplicationInfo application_info("VkApp",
			1, "Sky",
			1, VK_API_VERSION_1_2);

		auto extensions = get_required_extensions();
		auto instance_create_info = vk::InstanceCreateInfo({},
			&application_info, enable_validation_layers ? static_cast<uint32_t>(validation_layers.size()) : 0, { enable_validation_layers ? validation_layers.data() : nullptr },
			extensions.size(),
			extensions.data());

		instance_ = vk::createInstance(instance_create_info);
	}

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

	VKAPI_ATTR VkBool32 VKAPI_CALL csl::vulkan::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, VkDebugUtilsMessengerCallbackDataEXT const* p_call_back, void* user_data)
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
		auto info = vk::DebugUtilsMessengerCreateInfoEXT({}, severity_flags, message_flags
			, &debug_callback);

		pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance_.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		if (!pfnVkCreateDebugUtilsMessengerEXT)
		{
			throw std::runtime_error("Could not create debug messenger");
		}
		pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance_.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
		if (!pfnVkDestroyDebugUtilsMessengerEXT)
		{
			throw std::runtime_error("Unable to find pfnDestroyDebugUtilsMessengerEXt extension ");
		}

		debug_messenger_ = instance_.createDebugUtilsMessengerEXT(info);
	}

	void csl::vulkan::pick_physical_device()
	{
		auto devices = instance_.enumeratePhysicalDevices();
		if (devices.size() == 0)
		{
			throw std::runtime_error("Failed to create physical device");
		}
		for (const auto& device : devices)
		{
			if (is_device_suitable(device))
			{
				physical_device_ = device;
				break;
			}
		}
		if ((void*)physical_device_ == nullptr)
		{
			throw std::runtime_error(
				"failed to find suitable GPU"
			);
		}
	}

	bool csl::vulkan::is_device_suitable(vk::PhysicalDevice device)
	{
		auto indices = find_queue_families(device);
		bool swap_chain_adequate = false;
		bool extension_support = check_device_extension_support(device);
		if(extension_support)
		{
			auto swap_chain_details = query_swapchain_support(device);
			swap_chain_adequate = !swap_chain_details.formats.empty() && !swap_chain_details.present_mode.empty();
		}

		return indices.b_complete() && extension_support && swap_chain_adequate;

	}

	vulkan::queue_family_indices csl::vulkan::find_queue_families(vk::PhysicalDevice device)
	{
		queue_family_indices indices;
		auto queue_families = device.getQueueFamilyProperties();

		int i = 0;
		for (const auto& queue_fam : queue_families)
		{
			if (queue_fam.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphics_family = i;
			}
			if(device.getSurfaceSupportKHR(i,surface_))
			{
				indices.present_family = i;
			}
			if (indices.b_complete()) break;
			i++;
		}
		return indices;
	}
	vk::SurfaceFormatKHR csl::vulkan::choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats)
	{
		for(const auto& format:formats)
		{
			if(format.format==vk::Format::eB8G8R8A8Srgb && format.colorSpace==vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}
		return formats[0];
	}

	vk::PresentModeKHR vulkan::choose_swap_present_mode(const std::vector<vk::PresentModeKHR> present_modes)
	{
		for(const auto& mode:present_modes)
		{
			if(mode==vk::PresentModeKHR::eMailbox)
			{
				return mode;
			}
		}
        
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D vulkan::choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if(capabilities.currentExtent.width!=UINT32_MAX)
		{
			return capabilities.currentExtent;
		}else
		{
			auto actual_extent = glvk_.get_framebuffer_size();
			actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

			actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            
			return actual_extent;
          
		}
	}

	void csl::vulkan::create_logical_device()
	{
		auto indices = find_queue_families(physical_device_);
		std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
		std::set<uint32_t> unique_queue_families{ indices.graphics_family.value(),indices.present_family.value() };
		float queue_priority = 1.0f;
		for(auto queue_fam:unique_queue_families)
		{
			device_queue_create_infos.push_back(vk::DeviceQueueCreateInfo({}, queue_fam, 1, &queue_priority));
		}
		auto device_queue_info = vk::DeviceQueueCreateInfo({}, indices.graphics_family.value(), 1, &queue_priority);
		vk::PhysicalDeviceFeatures device_features;
		vk::DeviceCreateInfo device_create_info({}, device_queue_create_infos.size(), device_queue_create_infos.data());
		device_create_info.pEnabledFeatures = &device_features;
		device_create_info.enabledExtensionCount = device_extensions.size();
		device_create_info.ppEnabledExtensionNames = device_extensions.data();

		device_ = physical_device_.createDevice(device_create_info);
		graphics_queue_ = device_.getQueue(indices.graphics_family.value(),0);
		present_queue_ = device_.getQueue(indices.present_family.value(), 0);

	}


	csl::vulkan::swapchain_support_details csl::vulkan::query_swapchain_support(vk::PhysicalDevice device)
	{
		swapchain_support_details details;
		details.capabilities=device.getSurfaceCapabilitiesKHR(surface_);
		details.formats = device.getSurfaceFormatsKHR(surface_);

		details.present_mode = device.getSurfacePresentModesKHR(surface_);
		return details;
	}


	 bool vulkan::check_device_extension_support(vk::PhysicalDevice device)
	{
		auto available_extensions = device.enumerateDeviceExtensionProperties();
		std::set<std::string> required_extensions{ device_extensions.begin(),device_extensions.end() };
		for(const auto& extension:available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	void csl::vulkan::main_loop()
	{
		
	}
}
