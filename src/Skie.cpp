#include "Skie.h"
#include "../vk-bootstrap/src/VkBootstrap.h"

using namespace sk;
Skie::Skie() {

	_glvk =new Glvk(kWidth, kHeight,"Skie");

	std::cout <<"Content scale "<< _glvk->content_scale().width << std::endl;
	_glvk->set_key_callback(&key_callback);
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
	
	vk::RenderPassBeginInfo rpci{ _renderpass,_framebuffers[swapchain_image_indx],{{0,0},extent},1,&cv };
	cmd.beginRenderPass(rpci,vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _ppln_triangle);
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

PipelineBuilder::PipelineBuilder() {
	//_shader_stages = {};
	//_scissor = vk::Rect2D{};
	//_viewport = vk::Viewport{};
	//_rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo{};
	//_color_blend_attachment_states = {};
	//_multisample_state_create_info = vk::PipelineMultisampleStateCreateInfo{};
	//_pipeline_layout = vk::PipelineLayout{};
}

sk::PipelineBuilder::Result PipelineBuilder::build_pipeline(vk::Device device, vk::RenderPass pass) {
	auto viewport_state = vk::PipelineViewportStateCreateInfo({}, 1, &_viewport, 1, &_scissor);

	auto pcbscio = vk::PipelineColorBlendStateCreateInfo({}, false, vk::LogicOp::eCopy, _color_blend_attachment_states);

	auto gpcio = vk::GraphicsPipelineCreateInfo({}, _shader_stages, &_vertex_input_state_create_info, &_input_assembly_state_create_info, {}, &viewport_state, &_rasterization_state_create_info, &_multisample_state_create_info, {}, &pcbscio, {}, _pipeline_layout, pass, 0);
	Result r{ _pipeline_layout,device.createGraphicsPipeline(VK_NULL_HANDLE,gpcio).value };
	 return r;

}


void Skie::init_swapchain() {
	vkb::SwapchainBuilder swapchain_builder{ _gpu,_device,_surface };
	auto flags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
	swapchain_builder.set_image_usage_flags(static_cast<VkImageUsageFlags>(flags));

	auto vkb_swapchain = swapchain_builder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(_glvk->get_framebuffer_size().width, _glvk->get_framebuffer_size().height).build();
	

	_swapchain = vkb_swapchain.value();
	_swapchain_image_views = vkb_swapchain->get_image_views().value();
	_swapchain_images = vkb_swapchain->get_images().value();

	_swapchain_format = vk::Format{ vkb_swapchain->image_format };

}

void Skie::init_vulkan() {
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name("Skie").request_validation_layers().require_api_version(1, 1, 2)
		.use_default_debug_messenger()

	.build();

	_instance = inst_ret.value();
	_debug_messenger = inst_ret.value().debug_messenger;
	VkSurfaceKHR t_surface;
	_glvk->create_surface(_instance, &t_surface);
	//auto ret = _glvk->get_glfw_extensions_();
	_surface = vk::SurfaceKHR(t_surface);
	vkb::PhysicalDeviceSelector selector{ inst_ret.value() };
	auto physical_device = selector.set_minimum_version(1, 1).set_surface(_surface)
		.prefer_gpu_device_type().select();
	vkb::DeviceBuilder device_builder{ physical_device.value() };
	vkb::Device vkb_device = device_builder.build().value();
	_device = vkb_device;
	_gpu = physical_device.value().physical_device;

	_graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	_graphics_queue_family_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();


	
}

void Skie::init_commands() {

	auto cpci = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphics_queue_family_index);
	_command_pool=	_device.createCommandPool(cpci);
	auto cbai = vk::CommandBufferAllocateInfo(_command_pool, vk::CommandBufferLevel::ePrimary, 1);
	_main_command_buffer = _device.allocateCommandBuffers(cbai).front();



}

void Skie::init_default_renderpass() {
	auto ca = vk::AttachmentDescription({},_swapchain_format,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,vk::AttachmentLoadOp::eDontCare,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR);
	auto car = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	auto sd = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, {}, 1, &car);

	auto rpci = vk::RenderPassCreateInfo({}, 1, &ca, 1, &sd);
	_renderpass = _device.createRenderPass(rpci);

}

void Skie::init_framebuffers() {



	auto fb_info = vk::FramebufferCreateInfo({}, _renderpass, 1, {}, kWidth, kHeight, 1);
	_framebuffers = std::vector<vk::Framebuffer>(_swapchain_images.size());
	for(int i{0};auto fb: _framebuffers)
	{
		auto iv=vk::ImageView(_swapchain_image_views[i]);

		fb_info.pAttachments = &iv;
		_framebuffers[i] = _device.createFramebuffer(fb_info, nullptr);
		i++;
	}
}

void Skie::key_callback(GLFWwindow*, int key, int scancode,int action, int mods) {
	if(key==GLFW_KEY_SPACE)
	{
		/*_selected_shader = (_selected_shader + 1) % 2;
		if(kDebug)
		{
			std::cout << "Spacebar pressed" << std::endl;
		}*/
	}
}

void Skie::init_sync() {
	vk::FenceCreateInfo fci{ vk::FenceCreateFlagBits::eSignaled };
	_fnce_render = _device.createFence(fci);

	_smph_present = _device.createSemaphore(vk::SemaphoreCreateInfo{});
	_smph_render = _device.createSemaphore(vk::SemaphoreCreateInfo{});
}

void Skie::init_pipelines() {
	auto vert_shader_module = load_shader_module("./shaders/build/vert.spv");
	auto frag_shader_module = load_shader_module("./shaders/build/frag.spv");
	PipelineBuilder pipeline_builder{};
	pipeline_builder.
		set_pipeline_layout(_device.createPipelineLayout(vk::PipelineLayoutCreateInfo())).
		set_shader_stages({ vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,vert_shader_module,"main"),
			vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,frag_shader_module,"main")
			}).
		set_vertex_input_state(vk::PipelineVertexInputStateCreateInfo()).
		set_input_asm(vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList)).
		set_rasterizer(vk::PipelineRasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false,{},{},{},1.0f)).
		set_multisample(vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 1.0)).

		set_viewport({ 0,0,static_cast<float>(_glvk->get_framebuffer_size().width),static_cast<float>(_glvk->get_framebuffer_size().height) }).
		set_scissor({ {0,0},_glvk->get_framebuffer_size() });

	auto color_blend_bits = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	pipeline_builder.set_color_blend({ vk::PipelineColorBlendAttachmentState(false, {}, {}, {}, {}, {}, {}, color_blend_bits) });
	auto result = pipeline_builder.build_pipeline(_device, _renderpass);
	_ppln_lyt_triangle = result._pipeline_layout;
	_ppln_triangle = result._pipeline;

}

vk::ShaderModule Skie::load_shader_module(const std::string& path) {

	auto code = read_shader(path);
	auto info = vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
	return _device.createShaderModule(info);
}

Skie::~Skie() {

	_device.waitIdle();
	_device.destroyCommandPool(_command_pool);
	_device.destroyFence(_fnce_render);
	_device.destroySemaphore(_smph_render);
		_device.destroySemaphore(_smph_present);

	_device.destroySwapchainKHR(_swapchain);
	_device.destroyRenderPass(_renderpass);
	for (int i{0};auto& fb : _framebuffers)
	{
		_device.destroyFramebuffer(fb);
		i++;
	}
	for(auto image: _swapchain_image_views)
	{
		_device.destroyImageView(image);
	}
	_device.destroy();
	_instance.destroySurfaceKHR(_surface);
	vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
	_instance.destroy();
}
