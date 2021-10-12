#pragma once
#include <deque>

#include "../utils/vkutils.h"
#include <vulkan/vulkan_raii.hpp>
#include <functional>
namespace sk {

	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;
		void push_function(std::function<void()>&& function) {
			deletors.push_back(function);
		}
		void flush() {
			std::ranges::for_each(deletors, [](const std::function<void()> f) {
				f();
				});
			deletors.clear();
		}
	};

	inline void SkCheck(vk::Result r) {
		if (r != vk::Result::eSuccess)
		{
			throw vk::make_error_code(r);
		}
	}
	namespace events
	{
		
	}
	class Skie
	{

		//member functions
	public:
		Skie();
		void run();
		void draw();
		~Skie();


		//private member functions
	private:
		vk::raii::Context _vk_ctx;
		void init_swapchain();
		void init_vulkan();
		void init_commands();
		void init_default_renderpass();
		void init_framebuffers();
		//temp
		static void key_callback(GLFWwindow*, int,int,int,int);

		void init_sync();
		void init_pipelines();
		uint32_t _selected_shader{ 0 };

		


		//private member variables
	private:
		DeletionQueue _main_delete_queue;
		uint64_t _frame_number;
		std::unique_ptr<vk::raii::Instance> _instance;
		std::unique_ptr< vk::raii::DebugUtilsMessengerEXT> _debug_messenger;
		vk::raii::PhysicalDevice _gpu;
		vk::raii::Device _device;
		vk::raii::SurfaceKHR _surface;
		vk::raii::SwapchainKHR _swapchain;
		vk::Format _swapchain_format;
		std::vector<VkImage> _swapchain_images;
		std::vector<VkImageView> _swapchain_image_views;

		vk::raii::Queue _graphics_queue;
		uint32_t _graphics_queue_family_index;
		vk::raii::CommandPool _command_pool;
		vk::raii::CommandBuffer _main_command_buffer;
		vk::raii::RenderPass _renderpass;
		std::vector<vk::raii::Framebuffer> _framebuffers;
		vk::raii::PipelineLayout _ppln_lyt_triangle;

		vk::raii::Pipeline _ppln_triangle;
		vk::raii::Pipeline _ppln_red_tri;
		vk::raii::Semaphore _smph_present, _smph_render;
		vk::raii::Fence _fnce_render;
		

		vk::raii::ShaderModule load_shader_module(const std::string& path);
		
		Glvk* _glvk;



	};
	class PipelineBuilder
	{
		struct Result
		{
			vk::raii::PipelineLayout _pipeline_layout;
			vk::raii::Pipeline _pipeline;
		};
		std::vector<vk::PipelineShaderStageCreateInfo> _shader_stages;
		vk::PipelineVertexInputStateCreateInfo _vertex_input_state_create_info;
		vk::PipelineInputAssemblyStateCreateInfo _input_assembly_state_create_info;

		vk::Viewport _viewport;
		vk::Rect2D _scissor;
		vk::PipelineRasterizationStateCreateInfo _rasterization_state_create_info;
		std::vector<vk::PipelineColorBlendAttachmentState> _color_blend_attachment_states;
		vk::PipelineMultisampleStateCreateInfo _multisample_state_create_info;
		vk::raii::PipelineLayout _pipeline_layout;

	
	public:
		PipelineBuilder();
		Result build_pipeline(vk::raii::Device device, vk::raii::RenderPass pass);

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
		PipelineBuilder& set_pipeline_layout(vk::raii::PipelineLayout &pipeline_layout) {
			_pipeline_layout = std::move(pipeline_layout);
			return *this;
		}

	};
}

