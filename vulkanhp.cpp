#include "vulkanhp.h"
#include <set>
#include <iostream>
#include <ranges>
#include <algorithm>

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

	void vulkan::run()
	{
		main_loop();
	}

	csl::vulkan::~vulkan()
	{
		device_.destroySemaphore(smph_image_available_);
		device_.destroySemaphore(smph_render_finished_);
		device_.destroyCommandPool(command_pool_);
		for (auto f : framebuffers_)
		{
			device_.destroyFramebuffer(f);
		}
		device_.destroyPipeline(graphics_pipeline_);
		device_.destroyPipelineLayout(pipeline_layout_);

		device_.destroyRenderPass(render_pass_);
		for (auto& image_view : swap_chain_images_views)
		{
			device_.destroyImageView(image_view, nullptr);
		}
		device_.destroySwapchainKHR(swap_chain_);
		instance_.destroySurfaceKHR(surface_);
		device_.destroy();

		if (enable_validation_layers) {
			instance_.destroyDebugUtilsMessengerEXT(debug_messenger_);
		}
		instance_.destroy();


	}

	void vulkan::create_command_pool()
	{
		auto family_indides = find_queue_families(physical_device_);
		auto command_info = vk::CommandPoolCreateInfo({}, family_indides.graphics_family.value());

		command_pool_ = device_.createCommandPool(command_info, nullptr);

	}

	void vulkan::create_command_buffers()
	{

		auto command_info = vk::CommandBufferAllocateInfo(command_pool_, vk::CommandBufferLevel::ePrimary, framebuffers_.size());

		//Assumes this succeeds!
		command_buffers_ = device_.allocateCommandBuffers(command_info);

		for (size_t i = 0; i < command_buffers_.size(); i++)
		{
			auto begin_info = vk::CommandBufferBeginInfo();
			if (command_buffers_[i].begin(&begin_info) != vk::Result::eSuccess)
			{
				throw std::runtime_error("Failed to begin recording command buffer");
			}

			auto render_pass_info = vk::RenderPassBeginInfo(render_pass_, framebuffers_[i]);
			render_pass_info.renderArea = vk::Rect2D{ {0,0},swap_extent };

			auto clear_color = vk::ClearValue();
			clear_color.color = vk::ClearColorValue(std::array<float, 4>( {1.0,0.0,0.0,1.0}));

			render_pass_info.clearValueCount = 1;
			render_pass_info.pClearValues = &clear_color;
			command_buffers_[i].beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

			command_buffers_[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline_);
			command_buffers_[i].draw(3, 1, 0, 0);

			command_buffers_[i].end();

		}
	}

	void vulkan::create_semaphores()
	{
		smph_image_available_ = device_.createSemaphore(vk::SemaphoreCreateInfo());
		smph_render_finished_ = device_.createSemaphore((vk::SemaphoreCreateInfo()));

	}


	void csl::vulkan::init_vulkan()
	{
		create_instance();
		setup_debug_messenger();
		glvk_.create_surface(instance_, &surface_);
		pick_physical_device();
		create_logical_device();
		create_swapchain();
		create_image_views();
		create_render_pass();
		create_graphics_pipeline();
		create_framebuffers();
		create_command_pool();
		create_command_buffers();
		create_semaphores();
	}

	void csl::vulkan::populate_debug_messenger_info(vk::DebugUtilsMessengerCreateInfoEXT& info)
	{
		auto severity_flags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
		auto message_flags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

		info = vk::DebugUtilsMessengerCreateInfoEXT({}, severity_flags, message_flags
			, &debug_callback);
	};

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
		auto instance_create_info = vk::InstanceCreateInfo({}, &application_info, {}, {}, extensions.size(), extensions.data());


		auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT();
		if (enable_validation_layers)
		{
			instance_create_info.enabledLayerCount = validation_layers.size();
			instance_create_info.ppEnabledLayerNames = validation_layers.data();

			populate_debug_messenger_info(debug_create_info);
			instance_create_info.pNext = &debug_create_info;
		}
		else
		{
			instance_create_info.enabledLayerCount = 0;
			instance_create_info.pNext = nullptr;
		}


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
		if (extension_support)
		{
			auto swap_chain_details = query_swapchain_support(device);
			swap_chain_adequate = !swap_chain_details.formats.empty() && !swap_chain_details.present_mode.empty();
		}

		return indices.b_complete() && extension_support && swap_chain_adequate;

	}

	queue_family_indices csl::vulkan::find_queue_families(vk::PhysicalDevice device)
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
			if (device.getSurfaceSupportKHR(i, surface_))
			{
				indices.present_family = i;
			}
			if (indices.b_complete()) break;
			i++;

		}
		return indices;
	}

	void vulkan::draw_frame()
	{
		uint32_t image_index = device_.acquireNextImageKHR(swap_chain_, UINT32_MAX, smph_image_available_, VK_NULL_HANDLE).value;

		auto submit_info = vk::SubmitInfo();

		std::array wait_smph = { smph_image_available_ };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_smph.data();
		std::array wait_stages = { vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) };
		submit_info.pWaitDstStageMask = wait_stages.data();
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers_[image_index];

		std::array signal_smph = {smph_render_finished_};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_smph.data();
		if (graphics_queue_.submit(1, &submit_info, VK_NULL_HANDLE) != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to submit draw command to buffer");

		}
		auto present_info = vk::PresentInfoKHR(1, signal_smph.data());

		std::array swapchains = { swap_chain_ };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains.data();
		present_info.pImageIndices = &image_index;

		if (present_queue_.presentKHR(&present_info) != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failted to present");
		}


	}

	vk::SurfaceFormatKHR csl::vulkan::choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats)
	{
		for (const auto& format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}
		return formats[0];
	}

	vk::PresentModeKHR vulkan::choose_swap_present_mode(const std::vector<vk::PresentModeKHR> present_modes)
	{
		for (const auto& mode : present_modes)
		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				return mode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D vulkan::choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			auto actual_extent = glvk_.get_framebuffer_size();
			actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

			actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actual_extent;

		}
	}

	void vulkan::create_swapchain()
	{
		swapchain_support_details details = query_swapchain_support(physical_device_);
		auto surface_format = choose_swap_surface_format(details.formats);
		swap_image_format = surface_format.format;
		auto present_mode = choose_swap_present_mode(details.present_mode);
		swap_extent = choose_swap_extent(details.capabilities);

		uint32_t image_count = std::clamp(details.capabilities.minImageCount + 1, details.capabilities.minImageCount, details.capabilities.maxImageCount);

		auto swapchain_info = vk::SwapchainCreateInfoKHR({}, surface_, image_count, surface_format.format, surface_format.colorSpace, swap_extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

		auto indices = find_queue_families(physical_device_);
		uint32_t queue_family_indices[] = { indices.graphics_family.value(),indices.present_family.has_value() };

		if (indices.graphics_family != indices.present_family)
		{
			swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchain_info.queueFamilyIndexCount = 2;
			swapchain_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
			swapchain_info.queueFamilyIndexCount = 1;
			swapchain_info.pQueueFamilyIndices = nullptr;
		}
		swapchain_info.preTransform = details.capabilities.currentTransform;
		swapchain_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_info.presentMode = present_mode;
		swapchain_info.oldSwapchain = VK_NULL_HANDLE;
		vulkan::swap_chain_ = device_.createSwapchainKHR(swapchain_info);

		swap_chain_images = device_.getSwapchainImagesKHR(swap_chain_);




	}

	void vulkan::create_image_views()
	{
		swap_chain_images_views.resize(swap_chain_images.size());

		using namespace  std;
		std::vector<int> ints;
		ints | views::filter([=](int i)
			{
				return i == 0;
			});

		for (size_t i = 0; i < swap_chain_images.size(); i++)
		{
			auto image_view_info = vk::ImageViewCreateInfo({}, swap_chain_images[i], vk::ImageViewType::e2D, swap_image_format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			if (device_.createImageView(&image_view_info, nullptr, &swap_chain_images_views[i]) != vk::Result::eSuccess)
			{
				throw std::runtime_error("Could not create image views");
			}
		}



	}

	void vulkan::create_graphics_pipeline()
	{
		auto vert_shader_code = read_shader("shaders/vert.spv");
		auto frag_shader_code = read_shader("shaders/frag.spv");

		auto vert_shader_module = create_shader_module(vert_shader_code);
		auto frag_shader_module = create_shader_module(frag_shader_code);

		auto vert_shader_stage_info = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vert_shader_module, "main");
		auto frag_shader_stage_info = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, frag_shader_module, "main");

		vk::PipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

		auto vert_input_info = vk::PipelineVertexInputStateCreateInfo();
		vert_input_info.vertexAttributeDescriptionCount = 0;
		vert_input_info.vertexAttributeDescriptionCount = 0;

		auto input_asm_info = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

		/*A viewport basically describes the region of the framebuffer that the output will be rendered to. This will almost always be (0, 0) to (width, height) and in this tutorial that will also be the case.*/

		auto viewport = vk::Viewport(0.0, 0.0, static_cast<float>(swap_extent.width), static_cast<float>(swap_extent.height), 0.0, 1.0);


		auto scissors = vk::Rect2D({ 0,0 }, swap_extent);

		auto viewport_state_info = vk::PipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissors);
		auto rasterizer = vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, VK_FALSE);
		rasterizer.lineWidth = 1.0f;

		auto multi_info = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);

		auto color_blend_attachment = vk::PipelineColorBlendAttachmentState(VK_FALSE);
		color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;


		auto color_blending_info = vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE);
		color_blending_info.logicOpEnable = VK_FALSE;
		color_blending_info.logicOp = vk::LogicOp::eCopy;
		color_blending_info.attachmentCount = 1;
		color_blending_info.pAttachments = &color_blend_attachment;

		color_blending_info.blendConstants = vk::ArrayWrapper1D<float, 4>({ 0.0,0.0,0.0,0.0 });
		auto pipeline_layout_info = vk::PipelineLayoutCreateInfo();

		if (device_.createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline_layout_) != vk::Result::eSuccess)
		{
			throw std::runtime_error("Could not create pipeline layout");
		}

		auto pipeline_info = vk::GraphicsPipelineCreateInfo({}, 2, shader_stages, &vert_input_info, &input_asm_info, {}, &viewport_state_info, &rasterizer, &multi_info, {}, &color_blending_info, nullptr, pipeline_layout_, render_pass_, 0);

		/*	auto res= device_.createGraphicsPipeline(VK_NULL_HANDLE, pipeline_info);*/

		if (device_.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create graphics pipeline");
		}





		device_.destroyShaderModule(vert_shader_module);
		device_.destroyShaderModule(frag_shader_module);

	}

	void vulkan::create_render_pass()
	{

		vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlags(0), vk::AccessFlagBits::eColorAttachmentWrite);


		vk::AttachmentDescription color_attachment{};
		color_attachment.format = swap_image_format;
		color_attachment.samples = vk::SampleCountFlagBits::e1;
		color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
		color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
		color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		color_attachment.initialLayout = vk::ImageLayout::eUndefined;
		color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference color_attachment_ref({}, vk::ImageLayout::eColorAttachmentOptimal);
		color_attachment_ref.attachment = 0;
		auto subpass = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, 1, &color_attachment_ref);

		auto render_pass_info = vk::RenderPassCreateInfo({}, 1, &color_attachment, 1, &subpass);
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;
		if (device_.createRenderPass(&render_pass_info, nullptr, &render_pass_) != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create render pass");
		}



	}

	vk::ShaderModule vulkan::create_shader_module(std::vector<char>& code)
	{
		auto shader_create_info = vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));

		auto shader_module = device_.createShaderModule(shader_create_info, nullptr);
		return shader_module;

	}

	void vulkan::create_framebuffers()
	{
		framebuffers_.resize(swap_chain_images_views.size());

		for (int i{ 0 }; auto frame:framebuffers_)
		{
			vk::ImageView attachments[] = { swap_chain_images_views[i] };

			auto framebuffer_info = vk::FramebufferCreateInfo({}, render_pass_, 1, attachments, swap_extent.width, swap_extent.height, 1);
			framebuffers_[i] = device_.createFramebuffer(framebuffer_info, nullptr);
			i++;
		}
	}

	std::vector<char> vulkan::read_shader(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open file!");
		}

		auto file_size = file.tellg();
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		file.close();
		return buffer;

	}

	void csl::vulkan::create_logical_device()
	{
		auto indices = find_queue_families(physical_device_);
		std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
		std::set<uint32_t> unique_queue_families{ indices.graphics_family.value(),indices.present_family.value() };
		float queue_priority = 1.0f;
		for (auto queue_fam : unique_queue_families)
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
		graphics_queue_ = device_.getQueue(indices.graphics_family.value(), 0);
		present_queue_ = device_.getQueue(indices.present_family.value(), 0);

	}


	csl::swapchain_support_details csl::vulkan::query_swapchain_support(vk::PhysicalDevice device)
	{
		swapchain_support_details details;
		details.capabilities = device.getSurfaceCapabilitiesKHR(surface_);
		details.formats = device.getSurfaceFormatsKHR(surface_);

		details.present_mode = device.getSurfacePresentModesKHR(surface_);
		return details;
	}


	bool vulkan::check_device_extension_support(vk::PhysicalDevice device)
	{
		auto available_extensions = device.enumerateDeviceExtensionProperties();
		std::set<std::string> required_extensions{ device_extensions.begin(),device_extensions.end() };
		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	void csl::vulkan::main_loop()
	{
		while (!glvk_.window_should_close())
		{
			glvk_.poll_events();
			draw_frame();
		}

	}
}
