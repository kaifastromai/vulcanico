#include <vulkan/vulkan.hpp>
#include "glsk.h"
#define GLFW_INCLUDE_NONE
#include <stdexcept>
using namespace  sk;

void window::Glvk::init(const uint32_t& width, const uint32_t& height, const std::string& title) {
	GLFWwindow* w;
	if(window!=nullptr)
	{
		throw std::runtime_error("Glfw already initiliazed!");
	}
	glfwInit();
	init_window(width, height, title);
	get_glfw_extensions_();
}
window::Glvk::~Glvk() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void window::Glvk::init_window(uint32_t width, uint32_t height, const std::string title) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

bool window::Glvk::window_should_close() {
	return glfwWindowShouldClose(window);
}

std::vector<const char*> window::Glvk::get_glfw_extensions_() {
	uint32_t extension_count = 0;
	auto data = glfwGetRequiredInstanceExtensions(&extension_count);
	glfw_extensions_ = std::vector<const char*>{ extension_count };
	for (size_t i = 0; i < extension_count; i++)
	{
		glfw_extensions_[i] = data[i];
	}

	return glfw_extensions_;
}

void window::Glvk::poll_events() {
	glfwPollEvents();
}

void* window::Glvk::get_window_user_ptr() {
	return glfwGetWindowUserPointer(window);
}

void sk::window::Glvk::create_surface(vk::Instance instance, VkSurfaceKHR* surface)  {
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

sk::window::Glvk::Extent2d<float> sk::window::Glvk::content_scale()  {
	float x;
	float y;
	glfwGetWindowContentScale(window, &x, &y);
	return Extent2d{ x, y };

}

void sk::window::Glvk::set_key_callback( GLFWkeyfun callback, sk::Skie* ctx)  {
	glfwSetWindowUserPointer(window, ctx);
	glfwSetKeyCallback(window, callback);
}

vk::Extent2D sk::window::Glvk::get_framebuffer_size() {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return vk::Extent2D(width, height);
}

