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
		vk::PipelineLayout _ppln_lyt_triangle;

		vk::Pipeline _ppln_triangle;
		vk::Semaphore _smph_present, _smph_render;
		vk::Fence _fnce_render;
		

		 vk::ShaderModule load_shader_module(const std::string& path);
		
		Glvk* _glvk;



	};
	class PipelineBuilder
	{
		struct Result
		{
			vk::PipelineLayout _pipeline_layout;
			vk::Pipeline _pipeline;
		};
		std::vector<vk::PipelineShaderStageCreateInfo> _shader_stages;
		vk::PipelineVertexInputStateCreateInfo _vertex_input_state_create_info;
		vk::PipelineInputAssemblyStateCreateInfo _input_assembly_state_create_info;

		vk::Viewport _viewport;
		vk::Rect2D _scissor;
		vk::PipelineRasterizationStateCreateInfo _rasterization_state_create_info;
		std::vector<vk::PipelineColorBlendAttachmentState> _color_blend_attachment_states;
		vk::PipelineMultisampleStateCreateInfo _multisample_state_create_info;
		vk::PipelineLayout _pipeline_layout;

	
	public:
		PipelineBuilder();
		Result build_pipeline(vk::Device device, vk::RenderPass pass);

		 PipelineBuilder & set_shader_stages(const std::vector<vk::PipelineShaderStageCreateInfo> &stages) {
			_shader_stages = stages;
			return *this;
			
		}
		PipelineBuilder& set_vertex_input_state(vk::PipelineVertexInputStateCreateInfo vertex_input) {
			_vertex_input_state_create_info = vertex_input;
			return *this;
		 }
		PipelineBuilder& set_input_asm(vk::PipelineInputAssemblyStateCreateInfo input_state) {
			_input_assembly_state_create_info = input_state;
			return *this;
		}
		PipelineBuilder& set_viewport(vk::Viewport viewport) {
			_viewport = viewport;
			return *this;
		}
		PipelineBuilder& set_scissor(vk::Rect2D scissor) {
			_scissor = scissor;
			return *this;
		 }
		PipelineBuilder& set_rasterizer(vk::PipelineRasterizationStateCreateInfo rasterization) {
			_rasterization_state_create_info = rasterization;
			return *this;
		 }
		PipelineBuilder& set_color_blend(std::vector<vk::PipelineColorBlendAttachmentState >color_blend_attachment) {
			_color_blend_attachment_states = color_blend_attachment;
			return *this;
		}
		PipelineBuilder& set_multisample(vk::PipelineMultisampleStateCreateInfo multisample_state) {
			_multisample_state_create_info = multisample_state;
			return *this;
		}
		PipelineBuilder& set_pipeline_layout(vk::PipelineLayout pipeline_layout) {
			_pipeline_layout = pipeline_layout;
			return *this;
		}

	};
}

