#pragma once
#include <deque>
#define  VULKAN_HPP_RAII_ENABLE_DEFAULT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <functional>
#include "skie_mesh.h"
#include <glm/glm.hpp>


struct GLFWwindow;

namespace sk {
	struct MeshPushConstants
	{
		glm::vec4 data;
		glm::mat4 render_matrix;
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
		void load_mesh();
		void upload_mesh(Mesh &mesh);

		


		//private member variables
	private:
		void delete_allocator(VmaAllocator allocator) {
			vmaDestroyAllocator(allocator);
		}
		uint64_t _frame_number;
		std::unique_ptr<vk::raii::Instance> _instance;
		std::unique_ptr< vk::raii::DebugUtilsMessengerEXT> _debug_messenger;
		std::unique_ptr<vk::raii::PhysicalDevice> _gpu;
		std::unique_ptr<vk::raii::Device> _device;
		std::unique_ptr<vk::raii::SurfaceKHR >_surface;
		std::unique_ptr<vk::raii::SwapchainKHR> _swapchain;
		VkAllocator g_vma_allocator;
		vk::Format _swapchain_format;
		std::vector<VkImage> _swapchain_images;
		vk::raii::ImageView _img_view_depth;
		VkAllocatedImage _img_depth;
		vk::Format _fmt_depth;
		std::vector<vk::raii::ImageView> _swapchain_image_views;
		

		std::unique_ptr<vk::raii::Queue> _graphics_queue;
		uint32_t _graphics_queue_family_index;
		std::unique_ptr<vk::raii::CommandPool> _command_pool;
		std::unique_ptr<vk::raii::CommandBuffers> _main_command_buffer;
		std::unique_ptr<vk::raii::RenderPass> _renderpass;

		std::vector<vk::raii::Framebuffer> _framebuffers;
		std::unique_ptr<vk::raii::PipelineLayout> _ppln_lyt_triangle;
		std::unique_ptr < vk::raii::PipelineLayout > _ppln_lyt_mesh;
		std::unique_ptr<vk::raii::Pipeline >_ppln_triangle;
		std::unique_ptr<vk::raii::Pipeline> _ppln_red_tri;
		std::unique_ptr<vk::raii::Pipeline> _ppln_mesh;
		std::unique_ptr<vk::raii::Semaphore> _smph_present, _smph_render;
		std::unique_ptr<vk::raii::Fence> _fnce_render;

		std::unique_ptr<vk::raii::Pipeline> _mesh_pipeline;
	    Mesh _tri_mesh;
		Mesh _mesh_monkey;

		vk::raii::ShaderModule load_shader_module(const std::string& path);

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
		PipelineBuilder() = default;
		Result build_pipeline(vk::raii::Device& device, vk::raii::RenderPass& pass);

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

