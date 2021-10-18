#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

namespace vk {
	class Instance;
}

//forward declaration
struct GLFWwindow;
typedef void (*GLFWkeyfun)(GLFWwindow*, int, int, int, int);
namespace sk {
	class Skie;



	namespace window {
		class Glvk
		{
			//Define glfw keys
			
		public:
			static void init(const uint32_t& width, const uint32_t& height, const std::string& title);
			enum class key: unsigned int
			{
				eSpace = 32,
			};
			
		private:
			Glvk() = delete;

			~Glvk();

		private:
			static GLFWwindow* window;

			 static std::vector<const char*> glfw_extensions_;
			static void init_window(uint32_t width, uint32_t height, const std::string title);
		public:

			static [[nodiscard]] bool window_should_close();

			static std::vector<const char*> get_glfw_extensions_();

			static void poll_events();
			static void* get_window_user_ptr();
				

			template<class T>
			struct Extent2d
			{
				T width;
				T height;
			};

			static void create_surface(vk::Instance instance, VkSurfaceKHR* surface);

			static vk::Extent2D get_framebuffer_size();

			static Extent2d<float> content_scale();

			static void set_key_callback(GLFWkeyfun callback, sk::Skie* ctx);
		};

	}
}

