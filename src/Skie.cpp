#include "Skie.h"
#include "../vk-bootstrap/src/VkBootstrap.h"
#include "ranges"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi


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
	SkCheck(_device->waitForFences(**_fnce_render,true,1e9));
	_device->resetFences(std::array{ **_fnce_render });
	uint32_t swapchain_image_indx;
	
	swapchain_image_indx = _device->acquireNextImage2KHR(vk::AcquireNextImageInfoKHR(**_swapchain, 1e9, **_smph_present,{},1)).second;
	_main_command_buffer->front().reset();
	auto cmd = &_main_command_buffer->front();
	
	cmd->begin(vk::CommandBufferBeginInfo({vk::CommandBufferUsageFlagBits::eOneTimeSubmit}));
	vk::ClearValue cv;
	cv.color = vk::ClearColorValue( std::array<float,4>{(float)(1-sin(_frame_number/120.0))/2.0f, 0.0, 0.0, 1.0});
	auto extent = _glvk->get_framebuffer_size();
	
	vk::RenderPassBeginInfo rpci{ **_renderpass,*_framebuffers[swapchain_image_indx],{{0,0},{kWidth,kHeight}},1,&cv };
	cmd->beginRenderPass(rpci,vk::SubpassContents::eInline);

	cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, **_mesh_pipeline);
	vk::DeviceSize offset = 0;
	cmd->bindVertexBuffers(0, { _mesh_monkey.vertex_buffer->buffer }, {offset});

	glm::vec3 cam_vec = { 0.0f,0.0f,-3.0f };
	glm::mat4 view = translate(glm::mat4(1.0), cam_vec);
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), kWidth /(float)kHeight, 1.0f, 200.0f);
	projection[1][1] *= -1;

	glm::mat4 model = rotate(glm::mat4{ 1.0f }, glm::radians(_frame_number * 0.4f), glm::vec3(0, 1, 0));
	glm::mat4 mesh_matrix = projection * view * model;
	MeshPushConstants constants = { {},mesh_matrix };
	 std::array<MeshPushConstants, 1> pcs{ constants };

	cmd->pushConstants<MeshPushConstants>(**_ppln_lyt_mesh, vk::ShaderStageFlagBits::eVertex, 0, pcs);


	cmd->draw(_mesh_monkey.vertices.size(), 1, 0, 0);
	cmd->endRenderPass();
	cmd->end();
	auto wait_stage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	
		auto si = vk::SubmitInfo(1, &(**_smph_present) , &wait_stage, 
			1, &**cmd , 1,&(**_smph_render));
	_graphics_queue->submit({ si }, **_fnce_render);

	auto pi = vk::PresentInfoKHR(1, &(**_smph_render), 1, &(**_swapchain), &swapchain_image_indx);
	SkCheck(_graphics_queue->presentKHR(pi));

	_frame_number++;


}


PipelineBuilder::Result PipelineBuilder::build_pipeline(vk::raii::Device& device, vk::raii::RenderPass& pass) {
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
	vkb::SwapchainBuilder swapchain_builder{ **_gpu,**_device,**_surface };
	auto flags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
	swapchain_builder.set_image_usage_flags(static_cast<VkImageUsageFlags>(flags));

	auto vkb_swapchain = swapchain_builder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(_glvk->get_framebuffer_size().width, _glvk->get_framebuffer_size().height).build();
	

	_swapchain = std::make_unique<vk::raii::SwapchainKHR>(*_device, vkb_swapchain->swapchain);
	//_swapchain_image_views = vkb_swapchain->get_image_views().value();
	auto rawivs = vkb_swapchain->get_image_views().value();
	for (int i{ 0 };auto& iv : rawivs)
	{
		_swapchain_image_views.push_back(vk::raii::ImageView(*_device, iv));
	}

	_swapchain_images = vkb_swapchain->get_images().value();
	_swapchain_format = vk::Format{ vkb_swapchain->image_format };


}

void Skie::init_vulkan() {
	


	vkb::InstanceBuilder builder;

	auto inst_ret =builder.set_app_name("Skie")
	.require_api_version(1, 1, 2).
	request_validation_layers(true).enable_validation_layers(true)
	.build();

	//if(!inst_ret.has_value())
	//{
	//	throw std::runtime_error("Could not create vk instance");
	//}

	_instance = std::make_unique<vk::raii::Instance>(_vk_ctx, inst_ret.value().instance);
	_debug_messenger =std::make_unique< vk::raii::DebugUtilsMessengerEXT>(*_instance, inst_ret.value().debug_messenger);
	VkSurfaceKHR t_surface;
	//This is pretty ugly
	_glvk->create_surface(**_instance, &t_surface);
	//auto ret = _glvk->get_glfw_extensions_();
	_surface = std::make_unique<vk::raii::SurfaceKHR>(*_instance, t_surface);
	vkb::PhysicalDeviceSelector selector{ inst_ret.value() };
	auto physical_device = selector.set_minimum_version(1, 1).set_surface(**_surface)
		.prefer_gpu_device_type().select();
	vkb::DeviceBuilder device_builder{ physical_device.value() };
	vkb::Device vkb_device = device_builder.build().value();
	_gpu = std::make_unique<vk::raii::PhysicalDevice>(*_instance, physical_device.value().physical_device);

	_device =std::make_unique< vk::raii::Device>(*_gpu,vkb_device.device);
	_graphics_queue = std::make_unique<vk::raii::Queue>(*_device, vkb_device.get_queue(vkb::QueueType::graphics).value());
	_graphics_queue_family_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.physicalDevice = **_gpu;
	allocator_create_info.device = **_device;
	allocator_create_info.instance = **_instance;
	vmaCreateAllocator(&allocator_create_info, &g_vma_allocator);

	load_mesh();
	
}

void Skie::init_commands() {

	auto cpci = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphics_queue_family_index);
	_command_pool = std::make_unique<vk::raii::CommandPool>(*_device, cpci);
	auto cbai = vk::CommandBufferAllocateInfo(**_command_pool, vk::CommandBufferLevel::ePrimary, 1);
	_main_command_buffer = std::make_unique<vk::raii::CommandBuffers>(*_device, cbai);
	



}

void Skie::init_default_renderpass() {
	auto ca = vk::AttachmentDescription({},_swapchain_format,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,vk::AttachmentLoadOp::eDontCare,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR);
	auto car = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	auto sd = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, {}, 1, &car);

	auto rpci = vk::RenderPassCreateInfo({}, 1, &ca, 1, &sd);
	_renderpass = std::make_unique<vk::raii::RenderPass>(*_device, rpci);


}

void Skie::init_framebuffers() {



	/**/
	std::array<vk::ImageView, 1> attachments;
	
	for(int i{0};auto const  &iv: _swapchain_image_views)
	{
		
		attachments[0] = *iv;
		auto fb_info = vk::FramebufferCreateInfo({}, **_renderpass,  {attachments}, kWidth, kHeight, 1);

		_framebuffers.push_back(vk::raii::Framebuffer(*_device, fb_info));
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
	_fnce_render = std::make_unique<vk::raii::Fence>(*_device, fci);

	_smph_present = std::make_unique<vk::raii::Semaphore>(*_device,vk::SemaphoreCreateInfo());
	_smph_render = std::make_unique<vk::raii::Semaphore>(*_device, vk::SemaphoreCreateInfo());
}

void Skie::init_pipelines() {


	auto vert_shader_module_tri = load_shader_module("./shaders/build/colored_triangle.vert.spv");
	auto frag_shader_module_tri = load_shader_module("./shaders/build/colored_triangle.frag.spv");
	auto vert_shader_module_mesh = load_shader_module("./shaders/build/tri_shader.vert.spv");
	auto frag_shader_module_mesh = load_shader_module("./shaders/build/tri_shader.frag.spv");
	PipelineBuilder ppln_colored_tri{};
	auto pipeline_layout0 = vk::raii::PipelineLayout(*_device, vk::PipelineLayoutCreateInfo());
	ppln_colored_tri.
		set_pipeline_layout(pipeline_layout0).
		set_shader_stages({ vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,*vert_shader_module_tri,"main"),
			vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,*frag_shader_module_tri,"main")
			}).
		set_vertex_input_state(vk::PipelineVertexInputStateCreateInfo()).
		set_input_asm(vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList)).
		set_rasterizer(vk::PipelineRasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false,{},{},{},1.0f)).
		set_multisample(vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 1.0)).
		set_viewport({ 0,0,static_cast<float>(_glvk->get_framebuffer_size().width),static_cast<float>(_glvk->get_framebuffer_size().height) }).
		set_scissor({ {0,0},_glvk->get_framebuffer_size() });

	PipelineBuilder ppln_bldr_mesh {};
	auto push_constants = vk::PushConstantRange();
	push_constants.offset = 0;
	push_constants.size = sizeof(MeshPushConstants);
	push_constants.stageFlags = { vk::ShaderStageFlagBits::eVertex };

	auto pipeline_layout1 = vk::raii::PipelineLayout(*_device, vk::PipelineLayoutCreateInfo({},{},{},1,&push_constants));
	VertexInputDescription vertex_input_description = Vertex::get_vertex_description();
	ppln_bldr_mesh .
		set_pipeline_layout(pipeline_layout1).
		set_shader_stages({ vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,*vert_shader_module_mesh,"main"),
			vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,*frag_shader_module_mesh,"main")
			}).
		set_vertex_input_state(vk::PipelineVertexInputStateCreateInfo({},vertex_input_description.bindings,vertex_input_description.attributes)).
		set_input_asm(vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList)).
		set_rasterizer(vk::PipelineRasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false, {}, {}, {}, 1.0f)).
		set_multisample(vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 1.0)).
		set_viewport({ 0,0,static_cast<float>(_glvk->get_framebuffer_size().width),static_cast<float>(_glvk->get_framebuffer_size().height) }).
		set_scissor({ {0,0},_glvk->get_framebuffer_size() });

	auto color_blend_bits = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	ppln_bldr_mesh .set_color_blend({ vk::PipelineColorBlendAttachmentState(false, {}, {}, {}, {}, {}, {}, color_blend_bits) });
	ppln_colored_tri.set_color_blend({ vk::PipelineColorBlendAttachmentState(false, {}, {}, {}, {}, {}, {}, color_blend_bits) });

	auto res_ppln_colored_tri = ppln_colored_tri.build_pipeline(*_device, *_renderpass);
	auto res_mesh_pipeline = ppln_bldr_mesh .build_pipeline(*_device, *_renderpass);
	_mesh_pipeline= std::make_unique<vk::raii::Pipeline>(std::move(res_mesh_pipeline._pipeline));
	_ppln_lyt_triangle = std::make_unique<vk::raii::PipelineLayout>(std::move(res_ppln_colored_tri._pipeline_layout));
	_ppln_triangle = std::make_unique<vk::raii::Pipeline>(std::move(res_ppln_colored_tri._pipeline));
	//_ppln_red_tri = std::make_unique<vk::raii::Pipeline>(std::move(res_mesh_pipeline._pipeline));
	_ppln_lyt_mesh = std::make_unique<vk::raii::PipelineLayout>(std::move(res_mesh_pipeline._pipeline_layout));


}

void Skie::load_mesh() {
	glm::vec3 pos1 = { 1.f, 1.f, 0.0f };
	glm::vec3 pos2 = { -1.f, 1.f, 0.0f };
	glm::vec3 pos3 = { 0.f,-1.f, 0.0f };
	glm::vec3 color1= { 0.f, 1.f, 0.0f };
	glm::vec3 color2= { 0.f, 1.f, 0.0f };
	glm::vec3 color3= { 0.f, 1.f, 0.0f };
	_tri_mesh = { std::vector{Vertex{pos1,{},color1},Vertex{pos2,{},color2},Vertex{pos3,{},color3}} };
	if(!_mesh_monkey.from_obj("./resources/susan.obj")) {
		throw std::runtime_error("Could not load mesh from obj");
	}
	upload_mesh(_tri_mesh);
	upload_mesh(_mesh_monkey);
	
};
	

	


void Skie::upload_mesh(Mesh& mesh) {
	auto bcf = vk::BufferCreateInfo({}, mesh.vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
	mesh.vertex_buffer = std::make_unique<AllocatedBuffer>();
	VmaAllocationCreateInfo vma_allocator_create = {};
	vma_allocator_create.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	SkCheck(vk::Result(vmaCreateBuffer(g_vma_allocator,reinterpret_cast<VkBufferCreateInfo*>(&bcf) , &vma_allocator_create,&mesh.vertex_buffer->buffer, &mesh.vertex_buffer->allocation, nullptr)));

	void* data;
	vmaMapMemory(g_vma_allocator, mesh.vertex_buffer->allocation, &data);
	memcpy(data, mesh.vertices.data(),mesh.vertices.size()*sizeof(Vertex));
	vmaUnmapMemory(g_vma_allocator, mesh.vertex_buffer->allocation);


}

vk::raii::ShaderModule Skie::load_shader_module(const std::string& path) {

	auto code = read_shader(path);
	auto info = vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
	return vk::raii::ShaderModule(*_device, info);
}

Skie::~Skie() {
	while (vk::Result::eTimeout == _device->waitForFences({ **_fnce_render }, true, 1e9));

}
