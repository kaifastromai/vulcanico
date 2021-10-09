#pragma once
#include <algorithm>
#include <fstream>
#include <optional>
#include <vulkan\vulkan.hpp>
#include "vkutils.h"

namespace csl {

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"};
const std::vector device_extensions{
VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif
struct queue_family_indices
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool b_complete()
    {
        return graphics_family.has_value() && present_family.has_value();

    }
};
struct swapchain_support_details
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_mode;
};

class vulkan {
 private:
  glvk glvk_ = glvk(WIDTH, HEIGHT, "Vulkan");
  vk::Instance instance_;
  vk::PhysicalDevice physical_device_ = VK_NULL_HANDLE;
  vk::Device device_;
  vk::Queue graphics_queue_;
  VkSurfaceKHR surface_;
  vk::Queue present_queue_;
  vk::SwapchainKHR swap_chain_;
  int thisIsCool;
  std::vector<vk::Image> swap_chain_images;
  vk::Extent2D swap_extent;
  vk::Format swap_image_format;
  std::vector<vk::ImageView> swap_chain_images_views;
  vk::PipelineLayout pipeline_layout_;
  vk::RenderPass render_pass_;
  vk::Pipeline graphics_pipeline_;
  std::vector<vk::Framebuffer> framebuffers_;
  vk::CommandPool command_pool_;
  std::vector<vk::CommandBuffer> command_buffers_;
  vk::Semaphore smph_image_available_;
  vk::Semaphore smph_render_finished_;

public:
  vulkan();

  void run();

  ~vulkan();

 private:
  void create_command_pool();
  void create_command_buffers();
  void create_semaphores();
  void init_vulkan();
  static void populate_debug_messenger_info(vk::DebugUtilsMessengerCreateInfoEXT& info);
  void create_instance();

  static bool check_validation_layer();
  std::vector<const char*> get_required_extensions();
  VkDebugUtilsMessengerEXT debug_messenger_;

  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      VkDebugUtilsMessengerCallbackDataEXT const* p_call_back, void* user_data);

  void setup_debug_messenger();
  void pick_physical_device();
  bool is_device_suitable(vk::PhysicalDevice device);
   
    swapchain_support_details query_swapchain_support(vk::PhysicalDevice);

    void create_logical_device();
    bool check_device_extension_support(vk::PhysicalDevice device);
    
    vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats);
    vk::PresentModeKHR choose_swap_present_mode(const std::vector < vk::PresentModeKHR> present_modes);

    vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities);

    void create_swapchain();
    void create_image_views();
    void create_graphics_pipeline();
    void create_render_pass();
    vk::ShaderModule create_shader_module(std::vector<char>& code);

    void create_framebuffers();

  static std::vector<char> read_shader(const std::string& filename);

  queue_family_indices find_queue_families(vk::PhysicalDevice device);
  void draw_frame();
  void main_loop();

 public:
};
}  // namespace csl
