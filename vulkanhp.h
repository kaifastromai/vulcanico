#pragma once
#include <algorithm>
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

class vulkan {
  // public vars
 private:
  glvk glvk_ = glvk(WIDTH, HEIGHT, "Vulkan");
  vk::Instance instance_;
  vk::PhysicalDevice physical_device_ = VK_NULL_HANDLE;
  vk::Device device_;
  vk::Queue graphics_queue_;
  VkSurfaceKHR surface_;
  vk::Queue present_queue_;

 public:
  vulkan();

  void run();

  ~vulkan();

 private:
  void init_vulkan();
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
    swapchain_support_details query_swapchain_support(vk::PhysicalDevice);

    void create_logical_device();
    bool check_device_extension_support(vk::PhysicalDevice device);
    
    vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats);
    vk::PresentModeKHR choose_swap_present_mode(const std::vector < vk::PresentModeKHR> present_modes);

    vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities);

    void create_swapchain()
    {
        swapchain_support_details details = query_swapchain_support(physical_device_);
        auto surface_format = choose_swap_surface_format(details.formats);
        auto present_mode = choose_swap_present_mode(details.present_mode);
        auto extent = choose_swap_extent(details.capabilities);

        uint32_t image_count = std::clamp(details.capabilities.minImageCount + 1, details.capabilities.minImageCount, details.capabilities.maxImageCount);

        auto swapchain_info = vk::SwapchainCreateInfoKHR({}, surface_, image_count, surface_format.format, surface_format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

        auto indices = find_queue_families(physical_device_);
        uint32_t queue_family_indices[] = {indices.graphics_family.value(),indices.present_family.has_value()};

        if(indices.graphics_family!=indices.present_family)
        {
            swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchain_info.queueFamilyIndexCount = 2;
            swapchain_info.pQueueFamilyIndices = queue_family_indices;
        }else
        {
            swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
            swapchain_info.queueFamilyIndexCount = 1;
            swapchain_info.pQueueFamilyIndices = nullptr;
        }


    }

  queue_family_indices find_queue_families(vk::PhysicalDevice device);
  void main_loop();

 public:
};
}  // namespace csl
