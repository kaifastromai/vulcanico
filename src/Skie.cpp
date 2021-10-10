#include "Skie.h"
#include "../vk-bootstrap/src/VkBootstrap.h"

using namespace csl;
Skie::Skie() {


	_glvk =new Glvk(kWidth, kHeight,"Skie");
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
	cv.color = vk::ClearColorValue( std::array<float,4>{1.0,0.0,0.0,1.0});
	vk::RenderPassBeginInfo rpci{ _renderpass,_framebuffers[swapchain_image_indx],{{0,0},_glvk->get_framebuffer_size()},1,&cv };
	cmd.beginRenderPass(rpci,vk::SubpassContents::eInline);
	cmd.endRenderPass();
	cmd.end();
	auto wait_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	auto si = vk::SubmitInfo(1, &_smph_present,&wait_stage , 1, &cmd, 1, &_smph_render);
	SkCheck(_graphics_queue.submit(1, &si, _fnce_render));

	auto pi = vk::PresentInfoKHR(1, &_smph_render, 1, &_swapchain, &swapchain_image_indx);
	SkCheck(_graphics_queue.presentKHR(pi));

	_frame_number++;


}

PipelineBuilder::PipelineBuilder(vk::Device device, vk::RenderPass pass) {
	
}


void Skie::init_swapchain() {
	vkb::SwapchainBuilder swapchain_builder{ _gpu,_device,_surface };

	auto vkb_swapchain = swapchain_builder.use_default_format_selection().set_desired_present_mode(static_cast<VkPresentModeKHR>(vk::PresentModeKHR::eFifo)).set_desired_extent(kWidth, kHeight).build();

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
	if(!inst_ret.has_value())
	{
		throw inst_ret.error();
	}
	_instance = inst_ret.value();
	_debug_messenger = inst_ret.value().debug_messenger;
	VkSurfaceKHR t_surface;
	_glvk->create_surface(_instance, &t_surface);
	auto ret = _glvk->get_glfw_extensions_();
	_surface = vk::SurfaceKHR(t_surface);
	vkb::PhysicalDeviceSelector selector{ inst_ret.value() };
	auto physical_device = selector.set_minimum_version(1, 1).set_surface(_surface)
		.select();
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
	auto ca = vk::AttachmentDescription({},_swapchain_format,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eDontCare,vk::AttachmentLoadOp::eDontCare,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR);
	auto car = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	auto sd = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, {}, 1, &car);

	auto rpci = vk::RenderPassCreateInfo({}, 1, &ca, 1, &sd);
	_renderpass = _device.createRenderPass(rpci);

}

void Skie::init_framebuffers() {

	VkFramebufferCreateInfo fbi= {};
	fbi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbi.pNext = nullptr;
	fbi.renderPass = _renderpass;
	fbi.width = _glvk->get_framebuffer_size().width;
	fbi.height = _glvk->get_framebuffer_size().height;
	fbi.layers = 1;
	fbi.attachmentCount = 1;

	_framebuffers = std::vector<vk::Framebuffer>(_swapchain_images.size());
	for(int i{0};auto fb: _framebuffers)
	{

		fbi.pAttachments = &_swapchain_image_views[i];
		_framebuffers[i] = _device.createFramebuffer(fbi, nullptr);


		i++;
	}
}

void Skie::init_sync() {
	vk::FenceCreateInfo fci{ vk::FenceCreateFlagBits::eSignaled };
	_fnce_render = _device.createFence(fci);

	_smph_present = _device.createSemaphore(vk::SemaphoreCreateInfo{});
	_smph_render = _device.createSemaphore(vk::SemaphoreCreateInfo{});
}

void Skie::init_pipelines() {
	auto vert_shader_module = load_shader_module("./shaders/shader.vert.spv");
	auto frag_shader_module = load_shader_module("./shaders/shader.frag.spv");

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
