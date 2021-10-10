#pragma once
#include "../utils/vkutils.h"

namespace csl {
	class Skie
	{

		//member functions
	public:
		Skie();
		void run();
		void draw();
		~Skie();

		//member variables
	public:

		//private member functions
	private:
		void init_swapchain();
		void init_vulkan();
		void init_commands();
		void init_default_renderpass();
		void init_framebuffers();

		void init_sync();
		void init_pipelines();


		//private member variables
	private:
		uint64_t _frame_number;
		vk::Instance _instance;
		vk::DebugUtilsMessengerEXT _debug_messenger;
		vk::PhysicalDevice _gpu;
		vk::Device _device;
		vk::SurfaceKHR _surface;
		vk::SwapchainKHR _swapchain;
		vk::Format _swapchain_format;
		std::vector<VkImage> _swapchain_images;
		std::vector<VkImageView> _swapchain_image_views;

		vk::Queue _graphics_queue;
		uint32_t _graphics_queue_family_index;
		vk::CommandPool _command_pool;
		vk::CommandBuffer _main_command_buffer;
		vk::RenderPass _renderpass;
		std::vector<vk::Framebuffer> _framebuffers;

		vk::Semaphore _smph_present, _smph_render;
		vk::Fence _fnce_render;
		

		 vk::ShaderModule load_shader_module(const std::string& path);
		
		Glvk* _glvk;



	};
}

