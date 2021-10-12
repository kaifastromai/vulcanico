#include "Skie.h"
#include "../vk-bootstrap/src/VkBootstrap.h"

using namespace sk;
Skie::Skie() {

	_glvk =new Glvk(kWidth, kHeight,"Skie");
	_glvk->set_key_callback(&key_callback,this);
	
	init_vulkan();
	init_swapchain();
	init_commands();
	init_default_renderpass();
	init_framebuffers();
	init_sync();
	init_pipelines();
}
void Skie::run() {
	while(!_glvk->window_should_close())
	{
		_glvk->poll_events();
		draw();
	}
}
void Skie::draw() {
	SkCheck(_device.waitForFences(1, &_fnce_render, true, 1e9));
	SkCheck(_device.resetFences(1, &_fnce_render));
	uint32_t swapchain_image_indx;
	swapchain_image_indx = _device.acquireNextImageKHR(_swapchain, 1e9, _smph_present).value;
	_main_command_buffer.reset();
	auto cmd = _main_command_buffer;
	cmd.begin(vk::CommandBufferBeginInfo({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }));

	vk::ClearValue cv;
	cv.color = vk::ClearColorValue( std::array<float,4>{(float)(1-sin(_frame_number/120.0))/2.0f, 0.0, 0.0, 1.0});
	auto extent = _glvk->get_framebuffer_size();
	
	vk::RenderPassBeginInfo rpci{ _renderpass,_framebuffers[swapchain_image_indx],{{0,0},{kWidth,kHeight}},1,&cv };
	cmd.beginRenderPass(rpci,vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _selected_shader==0? _ppln_triangle:_ppln_red_tri);
	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
	cmd.end();
	auto wait_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	auto si = vk::SubmitInfo(1, &_smph_present,&wait_stage , 1, &cmd, 1, &_smph_render);
	SkCheck(_graphics_queue.submit(1, &si, _fnce_render));

	auto pi = vk::PresentInfoKHR(1, &_smph_render, 1, &_swapchain, &swapchain_image_indx);
	SkCheck(_graphics_queue.presentKHR(pi));

	_frame_number++;


}


sk::PipelineBuilder::Result PipelineBuilder::build_pipeline(vk::raii::Device device, vk::raii::RenderPass pass) {
	auto viewport_state = vk::PipelineViewportStateCreateInfo({}, 1, &_viewport, 1, &_scissor);

	auto pcbscio = vk::PipelineColorBlendStateCreateInfo({}, false, vk::LogicOp::eCopy, _color_blend_attachment_states);
	auto gpcio = vk::GraphicsPipelineCreateInfo({}, _shader_stages, &_vertex_input_state_create_info, &_input_assembly_state_create_info, {}, &viewport_state, &_rasterization_state_create_info, &_multisample_state_create_info, {}, &pcbscio, {}, *_pipeline_layout, *pass, 0);
	vk::raii::Pipeline pipeline(device, nullptr, gpcio);
	switch (pipeline.getConstructorSuccessCode())
	{
	case vk::Result::eSuccess: break;
	case vk::Result::ePipelineCompileRequiredEXT:
		throw std::runtime_error("Building pipeline required extensions");
	default: throw std::runtime_error("Unknown error occured in creating graphics pipeline");
	}
	Result r{ std::move(_pipeline_layout),std::move(pipeline)};
	 return r;

}


void Skie::init_swapchain() {
	vkb::SwapchainBuilder swapchain_builder{ _gpu,_device,_surface };
	auto flags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
	swapchain_builder.set_image_usage_flags(static_cast<VkImageUsageFlags>(flags));

	auto vkb_swapchain = swapchain_builder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(_glvk->get_framebuffer_size().width, _glvk->get_framebuffer_size().height).build();
	

	_swapchain = vk::raii::SwapchainKHR(_device, vkb_swapchain->swapchain);
	_swapchain_image_views = vkb_swapchain->get_image_views().value();
	_swapchain_images = vkb_swapchain->get_images().value();
	_swapchain_format = vk::Format{ vkb_swapchain->image_format };


}

void Skie::init_vulkan() {

	vkb::InstanceBuilder builder;
	builder.set_app_name("Skie");
	builder.request_validation_layers();
	builder.require_api_version(1, 1, 2);
	builder.use_default_debug_messenger();
	auto inst_ret =
		builder.build();
	if(!inst_ret.has_value())
	{
		throw std::runtime_error("Could not create vk instance");
	}

	_instance = vk::raii::Instance(_vk_ctx, inst_ret.value().instance);
	_debug_messenger = vk::raii::DebugUtilsMessengerEXT(_instance, inst_ret.value().debug_messenger);
	VkSurfaceKHR t_surface;
	_glvk->create_surface(*_instance, &t_surface);
	//auto ret = _glvk->get_glfw_extensions_();
	_surface = vk::raii::SurfaceKHR(_instance, t_surface);
	vkb::PhysicalDeviceSelector selector{ inst_ret.value() };
	auto physical_device = selector.set_minimum_version(1, 1).set_surface(*_surface)
		.prefer_gpu_device_type().select();
	vkb::DeviceBuilder device_builder{ physical_device.value() };
	vkb::Device vkb_device = device_builder.build().value();
	_gpu = vk::raii::PhysicalDevice(_instance, physical_device.value().physical_device);

	_device = vk::raii::Device(_gpu,vkb_device.device);
	_graphics_queue = vk::raii::Queue(_device, vkb_device.get_queue(vkb::QueueType::graphics).value());
	_graphics_queue_family_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();


	
}

void Skie::init_commands() {

	auto cpci = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphics_queue_family_index);
	_command_pool = vk::raii::CommandPool(_device, vk::CommandPoolCreateInfo({}, +_graphics_queue_family_index));
	auto cbai = vk::CommandBufferAllocateInfo(*_command_pool, vk::CommandBufferLevel::ePrimary, 1);
	_main_command_buffer = std::move(vk::raii::CommandBuffers(_device, cbai).front());



}

void Skie::init_default_renderpass() {
	auto ca = vk::AttachmentDescription({},_swapchain_format,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,vk::AttachmentLoadOp::eDontCare,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR);
	auto car = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	auto sd = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, {}, 1, &car);

	auto rpci = vk::RenderPassCreateInfo({}, 1, &ca, 1, &sd);
	_renderpass = vk::raii::RenderPass(_device, rpci);


}

void Skie::init_framebuffers() {



	auto fb_info = vk::FramebufferCreateInfo({}, *_renderpass, 1, {}, kWidth, kHeight, 1);
	_framebuffers = std::vector<vk::raii::Framebuffer>(_swapchain_images.size());
	for(int i{0};auto &fb: _framebuffers)
	{
		auto iv=vk::ImageView(_swapchain_image_views[i]);

		fb_info.pAttachments = &iv;
		_framebuffers[i] = vk::raii::Framebuffer(_device, fb_info);
		i++;
	}
}

void Skie::key_callback(GLFWwindow* w, int key, int scancode,int action, int mods) {
	Skie* ctx = static_cast<Skie*>(glfwGetWindowUserPointer(w));
	if(key==GLFW_KEY_SPACE)
	{
		ctx->_selected_shader = (ctx->_selected_shader + 1) % 2;
		if(kDebug)
		{
			std::cout << "Spacebar pressed" << std::endl;
		}
	}
}

void Skie::init_sync() {
	vk::FenceCreateInfo fci{ vk::FenceCreateFlagBits::eSignaled };
	_fnce_render = vk::raii::Fence(_device, fci);

	_smph_present = vk::raii::Semaphore(_device,vk::SemaphoreCreateInfo());
	_smph_render = vk::raii::Semaphore(_device, vk::SemaphoreCreateInfo());
}

void Skie::init_pipelines() {
	auto vert_shader_module_tri = load_shader_module("./shaders/build/colored_triangle.vert.spv");
	auto frag_shader_module_tri = load_shader_module("./shaders/build/colored_triangle.frag.spv");
	auto vert_shader_module_red = load_shader_module("./shaders/build/vert.spv");
	auto frag_shader_module_red = load_shader_module("./shaders/build/frag.spv");
	PipelineBuilder ppln_colored_tri{};
	auto pipeline_layout0 = vk::raii::PipelineLayout(_device, vk::PipelineLayoutCreateInfo());
	ppln_colored_tri.
		set_pipeline_layout(*pipeline_layout0).
		set_shader_stages({ vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,*vert_shader_module_tri,"main"),
			vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,*frag_shader_module_tri,"main")
			}).
		set_vertex_input_state(vk::PipelineVertexInputStateCreateInfo()).
		set_input_asm(vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList)).
		set_rasterizer(vk::PipelineRasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false,{},{},{},1.0f)).
		set_multisample(vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 1.0)).
		set_viewport({ 0,0,static_cast<float>(_glvk->get_framebuffer_size().width),static_cast<float>(_glvk->get_framebuffer_size().height) }).
		set_scissor({ {0,0},_glvk->get_framebuffer_size() });

	PipelineBuilder ppln_red_tri{};
	auto pipeline_layout1 = vk::raii::PipelineLayout(_device, vk::PipelineLayoutCreateInfo());

	ppln_red_tri.
		set_pipeline_layout(*pipeline_layout1).
		set_shader_stages({ vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,*vert_shader_module_red,"main"),
			vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,*frag_shader_module_red,"main")
			}).
		set_vertex_input_state(vk::PipelineVertexInputStateCreateInfo()).
		set_input_asm(vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList)).
		set_rasterizer(vk::PipelineRasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false, {}, {}, {}, 1.0f)).
		set_multisample(vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 1.0)).
		set_viewport({ 0,0,static_cast<float>(_glvk->get_framebuffer_size().width),static_cast<float>(_glvk->get_framebuffer_size().height) }).
		set_scissor({ {0,0},_glvk->get_framebuffer_size() });

	auto color_blend_bits = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	ppln_red_tri.set_color_blend({ vk::PipelineColorBlendAttachmentState(false, {}, {}, {}, {}, {}, {}, color_blend_bits) });
	ppln_colored_tri.set_color_blend({ vk::PipelineColorBlendAttachmentState(false, {}, {}, {}, {}, {}, {}, color_blend_bits) });

	auto res_ppln_colored_tri = ppln_colored_tri.build_pipeline(*_device, *_renderpass);
	auto res_ppln_red_tri = ppln_red_tri.build_pipeline(*_device, *_renderpass);

	_ppln_lyt_triangle = res_ppln_colored_tri._pipeline_layout;
	_ppln_triangle = res_ppln_colored_tri._pipeline;
	_ppln_red_tri = res_ppln_red_tri._pipeline;

}

vk::raii::ShaderModule Skie::load_shader_module(const std::string& path) {

	auto code = read_shader(path);
	auto info = vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
	return vk::raii::ShaderModule(_device, info);
}

Skie::~Skie() = default;
