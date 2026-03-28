/*******************************************************************************************
*
*   Standalone Vulkan Triangle - Phase 0 of raylib Vulkan backend
*
*   A minimal Vulkan program using GLFW for windowing. Renders a colored triangle
*   with validation layers enabled. No raylib dependency.
*
*   LINUX COMPILATION:
*       gcc -o vulkan_triangle vulkan_triangle.c -I../../src/external/glfw/include \
*           -L../../src/external/glfw/src -lglfw3 -lvulkan -lm -lpthread -ldl -lX11
*
*   Or with system GLFW:
*       gcc -o vulkan_triangle vulkan_triangle.c $(pkg-config --cflags --libs glfw3) -lvulkan
*
*   LICENSE: zlib/libpng
*
********************************************************************************************/

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//------------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------------
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   450
#define MAX_FRAMES_IN_FLIGHT 2
#ifndef ENABLE_VALIDATION
#define ENABLE_VALIDATION 1
#endif

//------------------------------------------------------------------------------------
// Embedded SPIR-V shaders (compiled from GLSL below)
//------------------------------------------------------------------------------------
// Vertex shader source (for reference):
//   #version 450
//   layout(location = 0) out vec3 fragColor;
//   vec2 positions[3] = vec2[](
//       vec2( 0.0, -0.5),
//       vec2( 0.5,  0.5),
//       vec2(-0.5,  0.5)
//   );
//   vec3 colors[3] = vec3[](
//       vec3(1.0, 0.0, 0.0),
//       vec3(0.0, 1.0, 0.0),
//       vec3(0.0, 0.0, 1.0)
//   );
//   void main() {
//       gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
//       fragColor = colors[gl_VertexIndex];
//   }

static const uint32_t vertShaderSPIRV[] = {
    0x07230203, 0x00010000, 0x000d000b, 0x00000036,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0008000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000022, 0x00000026, 0x00000031,
    0x00030003, 0x00000002, 0x000001c2, 0x000a0004,
    0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70,
    0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365,
    0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f,
    0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572,
    0x00657669, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00050005, 0x0000000c, 0x69736f70,
    0x6e6f6974, 0x00000073, 0x00040005, 0x00000017,
    0x6f6c6f63, 0x00007372, 0x00060005, 0x00000020,
    0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
    0x00060006, 0x00000020, 0x00000000, 0x505f6c67,
    0x7469736f, 0x006e6f69, 0x00070006, 0x00000020,
    0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953,
    0x00000000, 0x00070006, 0x00000020, 0x00000002,
    0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e,
    0x00070006, 0x00000020, 0x00000003, 0x435f6c67,
    0x446c6c75, 0x61747369, 0x0065636e, 0x00030005,
    0x00000022, 0x00000000, 0x00060005, 0x00000026,
    0x565f6c67, 0x65747265, 0x646e4978, 0x00007865,
    0x00050005, 0x00000031, 0x67617266, 0x6f6c6f43,
    0x00000072, 0x00030047, 0x00000020, 0x00000002,
    0x00050048, 0x00000020, 0x00000000, 0x0000000b,
    0x00000000, 0x00050048, 0x00000020, 0x00000001,
    0x0000000b, 0x00000001, 0x00050048, 0x00000020,
    0x00000002, 0x0000000b, 0x00000003, 0x00050048,
    0x00000020, 0x00000003, 0x0000000b, 0x00000004,
    0x00040047, 0x00000026, 0x0000000b, 0x0000002a,
    0x00040047, 0x00000031, 0x0000001e, 0x00000000,
    0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020,
    0x00040017, 0x00000007, 0x00000006, 0x00000002,
    0x00040015, 0x00000008, 0x00000020, 0x00000000,
    0x0004002b, 0x00000008, 0x00000009, 0x00000003,
    0x0004001c, 0x0000000a, 0x00000007, 0x00000009,
    0x00040020, 0x0000000b, 0x00000006, 0x0000000a,
    0x0004003b, 0x0000000b, 0x0000000c, 0x00000006,
    0x0004002b, 0x00000006, 0x0000000d, 0x00000000,
    0x0004002b, 0x00000006, 0x0000000e, 0xbf000000,
    0x0005002c, 0x00000007, 0x0000000f, 0x0000000d,
    0x0000000e, 0x0004002b, 0x00000006, 0x00000010,
    0x3f000000, 0x0005002c, 0x00000007, 0x00000011,
    0x00000010, 0x00000010, 0x0005002c, 0x00000007,
    0x00000012, 0x0000000e, 0x00000010, 0x0006002c,
    0x0000000a, 0x00000013, 0x0000000f, 0x00000011,
    0x00000012, 0x00040017, 0x00000014, 0x00000006,
    0x00000003, 0x0004001c, 0x00000015, 0x00000014,
    0x00000009, 0x00040020, 0x00000016, 0x00000006,
    0x00000015, 0x0004003b, 0x00000016, 0x00000017,
    0x00000006, 0x0004002b, 0x00000006, 0x00000018,
    0x3f800000, 0x0006002c, 0x00000014, 0x00000019,
    0x00000018, 0x0000000d, 0x0000000d, 0x0006002c,
    0x00000014, 0x0000001a, 0x0000000d, 0x00000018,
    0x0000000d, 0x0006002c, 0x00000014, 0x0000001b,
    0x0000000d, 0x0000000d, 0x00000018, 0x0006002c,
    0x00000015, 0x0000001c, 0x00000019, 0x0000001a,
    0x0000001b, 0x00040017, 0x0000001d, 0x00000006,
    0x00000004, 0x0004002b, 0x00000008, 0x0000001e,
    0x00000001, 0x0004001c, 0x0000001f, 0x00000006,
    0x0000001e, 0x0006001e, 0x00000020, 0x0000001d,
    0x00000006, 0x0000001f, 0x0000001f, 0x00040020,
    0x00000021, 0x00000003, 0x00000020, 0x0004003b,
    0x00000021, 0x00000022, 0x00000003, 0x00040015,
    0x00000023, 0x00000020, 0x00000001, 0x0004002b,
    0x00000023, 0x00000024, 0x00000000, 0x00040020,
    0x00000025, 0x00000001, 0x00000023, 0x0004003b,
    0x00000025, 0x00000026, 0x00000001, 0x00040020,
    0x00000028, 0x00000006, 0x00000007, 0x00040020,
    0x0000002e, 0x00000003, 0x0000001d, 0x00040020,
    0x00000030, 0x00000003, 0x00000014, 0x0004003b,
    0x00000030, 0x00000031, 0x00000003, 0x00040020,
    0x00000033, 0x00000006, 0x00000014, 0x00050036,
    0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0003003e, 0x0000000c,
    0x00000013, 0x0003003e, 0x00000017, 0x0000001c,
    0x0004003d, 0x00000023, 0x00000027, 0x00000026,
    0x00050041, 0x00000028, 0x00000029, 0x0000000c,
    0x00000027, 0x0004003d, 0x00000007, 0x0000002a,
    0x00000029, 0x00050051, 0x00000006, 0x0000002b,
    0x0000002a, 0x00000000, 0x00050051, 0x00000006,
    0x0000002c, 0x0000002a, 0x00000001, 0x00070050,
    0x0000001d, 0x0000002d, 0x0000002b, 0x0000002c,
    0x0000000d, 0x00000018, 0x00050041, 0x0000002e,
    0x0000002f, 0x00000022, 0x00000024, 0x0003003e,
    0x0000002f, 0x0000002d, 0x0004003d, 0x00000023,
    0x00000032, 0x00000026, 0x00050041, 0x00000033,
    0x00000034, 0x00000017, 0x00000032, 0x0004003d,
    0x00000014, 0x00000035, 0x00000034, 0x0003003e,
    0x00000031, 0x00000035, 0x000100fd, 0x00010038,
};

// Fragment shader source (for reference):
//   #version 450
//   layout(location = 0) in vec3 fragColor;
//   layout(location = 0) out vec4 outColor;
//   void main() {
//       outColor = vec4(fragColor, 1.0);
//   }

static const uint32_t fragShaderSPIRV[] = {
    0x07230203, 0x00010000, 0x000d000b, 0x00000013,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000009, 0x0000000c, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002,
    0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f,
    0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c,
    0x7269645f, 0x69746365, 0x00006576, 0x00080004,
    0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63,
    0x69645f65, 0x74636572, 0x00657669, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005,
    0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000,
    0x00050005, 0x0000000c, 0x67617266, 0x6f6c6f43,
    0x00000072, 0x00040047, 0x00000009, 0x0000001e,
    0x00000000, 0x00040047, 0x0000000c, 0x0000001e,
    0x00000000, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006,
    0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009,
    0x00000003, 0x00040017, 0x0000000a, 0x00000006,
    0x00000003, 0x00040020, 0x0000000b, 0x00000001,
    0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c,
    0x00000001, 0x0004002b, 0x00000006, 0x0000000e,
    0x3f800000, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005,
    0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c,
    0x00050051, 0x00000006, 0x0000000f, 0x0000000d,
    0x00000000, 0x00050051, 0x00000006, 0x00000010,
    0x0000000d, 0x00000001, 0x00050051, 0x00000006,
    0x00000011, 0x0000000d, 0x00000002, 0x00070050,
    0x00000007, 0x00000012, 0x0000000f, 0x00000010,
    0x00000011, 0x0000000e, 0x0003003e, 0x00000009,
    0x00000012, 0x000100fd, 0x00010038,
};

//------------------------------------------------------------------------------------
// Vulkan state
//------------------------------------------------------------------------------------
typedef struct VulkanState {
    GLFWwindow *window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    uint32_t graphicsFamily;
    uint32_t presentFamily;

    VkSwapchainKHR swapchain;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    uint32_t swapchainImageCount;
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *framebuffers;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore *renderFinishedSemaphores;  // One per swapchain image
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

    uint32_t currentFrame;
    bool framebufferResized;
} VulkanState;

static VulkanState vk = { 0 };

//------------------------------------------------------------------------------------
// Debug callback
//------------------------------------------------------------------------------------
#if ENABLE_VALIDATION
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData)
{
    (void)type; (void)userData;
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "[VULKAN] %s\n", callbackData->pMessage);
    }
    return VK_FALSE;
}
#endif

//------------------------------------------------------------------------------------
// Helper: check VkResult
//------------------------------------------------------------------------------------
static void vkCheck(VkResult result, const char *msg) {
    if (result != VK_SUCCESS) {
        fprintf(stderr, "VULKAN ERROR (%d): %s\n", result, msg);
        exit(1);
    }
}

//------------------------------------------------------------------------------------
// Framebuffer resize callback
//------------------------------------------------------------------------------------
static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    (void)width; (void)height;
    VulkanState *state = (VulkanState *)glfwGetWindowUserPointer(window);
    state->framebufferResized = true;
}

//------------------------------------------------------------------------------------
// Create Vulkan instance
//------------------------------------------------------------------------------------
static void createInstance(void) {
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    // Copy GLFW extensions + add debug utils
    uint32_t extCount = glfwExtCount;
    const char *extensions[32];
    for (uint32_t i = 0; i < glfwExtCount; i++) extensions[i] = glfwExts[i];

#if ENABLE_VALIDATION
    extensions[extCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    const char *validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
    uint32_t layerCount = 1;
#else
    const char **validationLayers = NULL;
    uint32_t layerCount = 0;
#endif

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = layerCount,
        .ppEnabledLayerNames = validationLayers,
    };

    vkCheck(vkCreateInstance(&createInfo, NULL, &vk.instance), "Failed to create instance");
}

//------------------------------------------------------------------------------------
// Setup debug messenger
//------------------------------------------------------------------------------------
static void setupDebugMessenger(void) {
#if ENABLE_VALIDATION
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        vkCheck(func(vk.instance, &createInfo, NULL, &vk.debugMessenger), "Failed to set up debug messenger");
    }
#endif
}

//------------------------------------------------------------------------------------
// Create surface
//------------------------------------------------------------------------------------
static void createSurface(void) {
    vkCheck(glfwCreateWindowSurface(vk.instance, vk.window, NULL, &vk.surface), "Failed to create window surface");
}

//------------------------------------------------------------------------------------
// Pick physical device
//------------------------------------------------------------------------------------
static void pickPhysicalDevice(void) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vk.instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "No Vulkan-capable GPU found!\n");
        exit(1);
    }

    VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(vk.instance, &deviceCount, devices);

    // Prefer discrete GPU
    vk.physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, NULL);
        VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);

        bool hasGraphics = false, hasPresent = false;
        for (uint32_t j = 0; j < queueFamilyCount; j++) {
            if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                vk.graphicsFamily = j;
                hasGraphics = true;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, vk.surface, &presentSupport);
            if (presentSupport) {
                vk.presentFamily = j;
                hasPresent = true;
            }
        }
        free(queueFamilies);

        if (hasGraphics && hasPresent) {
            vk.physicalDevice = devices[i];
            printf("Selected GPU: %s\n", props.deviceName);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) break; // prefer discrete
        }
    }
    free(devices);

    if (vk.physicalDevice == VK_NULL_HANDLE) {
        fprintf(stderr, "No suitable GPU found!\n");
        exit(1);
    }
}

//------------------------------------------------------------------------------------
// Create logical device
//------------------------------------------------------------------------------------
static void createLogicalDevice(void) {
    float queuePriority = 1.0f;

    // If graphics and present are the same family, only create one queue
    uint32_t uniqueFamilies[2] = { vk.graphicsFamily, vk.presentFamily };
    uint32_t uniqueCount = (vk.graphicsFamily == vk.presentFamily) ? 1 : 2;

    VkDeviceQueueCreateInfo queueCreateInfos[2];
    for (uint32_t i = 0; i < uniqueCount; i++) {
        queueCreateInfos[i] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };
    const char *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = uniqueCount,
        .pQueueCreateInfos = queueCreateInfos,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions,
    };

    vkCheck(vkCreateDevice(vk.physicalDevice, &createInfo, NULL, &vk.device), "Failed to create logical device");
    vkGetDeviceQueue(vk.device, vk.graphicsFamily, 0, &vk.graphicsQueue);
    vkGetDeviceQueue(vk.device, vk.presentFamily, 0, &vk.presentQueue);
}

//------------------------------------------------------------------------------------
// Create swapchain
//------------------------------------------------------------------------------------
static void createSwapchain(void) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.physicalDevice, vk.surface, &capabilities);

    // Choose surface format (prefer SRGB)
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physicalDevice, vk.surface, &formatCount, NULL);
    VkSurfaceFormatKHR *formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physicalDevice, vk.surface, &formatCount, formats);

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (uint32_t i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = formats[i];
            break;
        }
    }
    free(formats);

    // Choose present mode (prefer mailbox for triple buffering, fallback to FIFO)
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physicalDevice, vk.surface, &presentModeCount, NULL);
    VkPresentModeKHR *presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physicalDevice, vk.surface, &presentModeCount, presentModes);

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    free(presentModes);

    // Choose swap extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(vk.window, &width, &height);
        extent.width = (uint32_t)width;
        extent.height = (uint32_t)height;
        if (extent.width < capabilities.minImageExtent.width) extent.width = capabilities.minImageExtent.width;
        if (extent.width > capabilities.maxImageExtent.width) extent.width = capabilities.maxImageExtent.width;
        if (extent.height < capabilities.minImageExtent.height) extent.height = capabilities.minImageExtent.height;
        if (extent.height > capabilities.maxImageExtent.height) extent.height = capabilities.maxImageExtent.height;
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vk.surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    if (vk.graphicsFamily != vk.presentFamily) {
        uint32_t queueFamilyIndices[] = { vk.graphicsFamily, vk.presentFamily };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    vkCheck(vkCreateSwapchainKHR(vk.device, &createInfo, NULL, &vk.swapchain), "Failed to create swapchain");

    vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.swapchainImageCount, NULL);
    vk.swapchainImages = malloc(vk.swapchainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.swapchainImageCount, vk.swapchainImages);

    vk.swapchainFormat = surfaceFormat.format;
    vk.swapchainExtent = extent;
}

//------------------------------------------------------------------------------------
// Create image views
//------------------------------------------------------------------------------------
static void createImageViews(void) {
    vk.swapchainImageViews = malloc(vk.swapchainImageCount * sizeof(VkImageView));

    for (uint32_t i = 0; i < vk.swapchainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = vk.swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = vk.swapchainFormat,
            .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        vkCheck(vkCreateImageView(vk.device, &createInfo, NULL, &vk.swapchainImageViews[i]),
                "Failed to create image view");
    }
}

//------------------------------------------------------------------------------------
// Create render pass
//------------------------------------------------------------------------------------
static void createRenderPass(void) {
    VkAttachmentDescription colorAttachment = {
        .format = vk.swapchainFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorRef,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    vkCheck(vkCreateRenderPass(vk.device, &renderPassInfo, NULL, &vk.renderPass), "Failed to create render pass");
}

//------------------------------------------------------------------------------------
// Create graphics pipeline
//------------------------------------------------------------------------------------
static void createGraphicsPipeline(void) {
    // Create shader modules
    VkShaderModuleCreateInfo vertInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vertShaderSPIRV),
        .pCode = vertShaderSPIRV,
    };
    VkShaderModule vertModule;
    vkCheck(vkCreateShaderModule(vk.device, &vertInfo, NULL, &vertModule), "Failed to create vertex shader module");

    VkShaderModuleCreateInfo fragInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(fragShaderSPIRV),
        .pCode = fragShaderSPIRV,
    };
    VkShaderModule fragModule;
    vkCheck(vkCreateShaderModule(vk.device, &fragInfo, NULL, &fragModule), "Failed to create fragment shader module");

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertModule,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragModule,
            .pName = "main",
        },
    };

    // No vertex input (positions hardcoded in shader)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Dynamic viewport and scissor
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates,
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    vkCheck(vkCreatePipelineLayout(vk.device, &pipelineLayoutInfo, NULL, &vk.pipelineLayout),
            "Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = vk.pipelineLayout,
        .renderPass = vk.renderPass,
        .subpass = 0,
    };

    vkCheck(vkCreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vk.graphicsPipeline),
            "Failed to create graphics pipeline");

    vkDestroyShaderModule(vk.device, fragModule, NULL);
    vkDestroyShaderModule(vk.device, vertModule, NULL);
}

//------------------------------------------------------------------------------------
// Create framebuffers
//------------------------------------------------------------------------------------
static void createFramebuffers(void) {
    vk.framebuffers = malloc(vk.swapchainImageCount * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < vk.swapchainImageCount; i++) {
        VkFramebufferCreateInfo fbInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vk.renderPass,
            .attachmentCount = 1,
            .pAttachments = &vk.swapchainImageViews[i],
            .width = vk.swapchainExtent.width,
            .height = vk.swapchainExtent.height,
            .layers = 1,
        };
        vkCheck(vkCreateFramebuffer(vk.device, &fbInfo, NULL, &vk.framebuffers[i]),
                "Failed to create framebuffer");
    }
}

//------------------------------------------------------------------------------------
// Create command pool and buffers
//------------------------------------------------------------------------------------
static void createCommandBuffers(void) {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vk.graphicsFamily,
    };
    vkCheck(vkCreateCommandPool(vk.device, &poolInfo, NULL, &vk.commandPool), "Failed to create command pool");

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };
    vkCheck(vkAllocateCommandBuffers(vk.device, &allocInfo, vk.commandBuffers), "Failed to allocate command buffers");
}

//------------------------------------------------------------------------------------
// Create sync objects
//------------------------------------------------------------------------------------
static void createSyncObjects(void) {
    VkSemaphoreCreateInfo semInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,  // Start signaled so first frame doesn't block
    };

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCheck(vkCreateSemaphore(vk.device, &semInfo, NULL, &vk.imageAvailableSemaphores[i]), "Failed to create semaphore");
        vkCheck(vkCreateFence(vk.device, &fenceInfo, NULL, &vk.inFlightFences[i]), "Failed to create fence");
    }

    // One render-finished semaphore per swapchain image to avoid reuse conflicts
    vk.renderFinishedSemaphores = malloc(vk.swapchainImageCount * sizeof(VkSemaphore));
    for (uint32_t i = 0; i < vk.swapchainImageCount; i++) {
        vkCheck(vkCreateSemaphore(vk.device, &semInfo, NULL, &vk.renderFinishedSemaphores[i]), "Failed to create semaphore");
    }
}

//------------------------------------------------------------------------------------
// Cleanup swapchain resources (for recreation)
//------------------------------------------------------------------------------------
static void cleanupSwapchain(void) {
    for (uint32_t i = 0; i < vk.swapchainImageCount; i++) {
        vkDestroyFramebuffer(vk.device, vk.framebuffers[i], NULL);
        vkDestroyImageView(vk.device, vk.swapchainImageViews[i], NULL);
        vkDestroySemaphore(vk.device, vk.renderFinishedSemaphores[i], NULL);
    }
    free(vk.framebuffers);
    free(vk.swapchainImageViews);
    free(vk.swapchainImages);
    free(vk.renderFinishedSemaphores);
    vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);
}

//------------------------------------------------------------------------------------
// Recreate swapchain (on resize)
//------------------------------------------------------------------------------------
static void recreateSwapchain(void) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(vk.window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(vk.window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vk.device);
    cleanupSwapchain();
    createSwapchain();
    createImageViews();
    createFramebuffers();

    // Recreate render-finished semaphores (one per swapchain image)
    VkSemaphoreCreateInfo semInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vk.renderFinishedSemaphores = malloc(vk.swapchainImageCount * sizeof(VkSemaphore));
    for (uint32_t i = 0; i < vk.swapchainImageCount; i++) {
        vkCheck(vkCreateSemaphore(vk.device, &semInfo, NULL, &vk.renderFinishedSemaphores[i]), "Failed to create semaphore");
    }
}

//------------------------------------------------------------------------------------
// Record command buffer
//------------------------------------------------------------------------------------
static void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkCheck(vkBeginCommandBuffer(cmd, &beginInfo), "Failed to begin command buffer");

    VkClearValue clearColor = { .color = {{ 0.1f, 0.1f, 0.1f, 1.0f }} };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = vk.renderPass,
        .framebuffer = vk.framebuffers[imageIndex],
        .renderArea = { .offset = {0, 0}, .extent = vk.swapchainExtent },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.graphicsPipeline);

    VkViewport viewport = {
        .x = 0.0f, .y = 0.0f,
        .width = (float)vk.swapchainExtent.width,
        .height = (float)vk.swapchainExtent.height,
        .minDepth = 0.0f, .maxDepth = 1.0f,
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = vk.swapchainExtent,
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);
    vkCheck(vkEndCommandBuffer(cmd), "Failed to record command buffer");
}

//------------------------------------------------------------------------------------
// Draw frame
//------------------------------------------------------------------------------------
static void drawFrame(void) {
    vkWaitForFences(vk.device, 1, &vk.inFlightFences[vk.currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vk.device, vk.swapchain, UINT64_MAX,
                                             vk.imageAvailableSemaphores[vk.currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "Failed to acquire swapchain image!\n");
        exit(1);
    }

    vkResetFences(vk.device, 1, &vk.inFlightFences[vk.currentFrame]);
    vkResetCommandBuffer(vk.commandBuffers[vk.currentFrame], 0);
    recordCommandBuffer(vk.commandBuffers[vk.currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = { vk.imageAvailableSemaphores[vk.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { vk.renderFinishedSemaphores[imageIndex] };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk.commandBuffers[vk.currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    vkCheck(vkQueueSubmit(vk.graphicsQueue, 1, &submitInfo, vk.inFlightFences[vk.currentFrame]),
            "Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &vk.swapchain,
        .pImageIndices = &imageIndex,
    };

    result = vkQueuePresentKHR(vk.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vk.framebufferResized) {
        vk.framebufferResized = false;
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to present swapchain image!\n");
        exit(1);
    }

    vk.currentFrame = (vk.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//------------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------------
static void cleanup(void) {
    vkDeviceWaitIdle(vk.device);

    for (uint32_t i = 0; i < vk.swapchainImageCount; i++) {
        vkDestroySemaphore(vk.device, vk.renderFinishedSemaphores[i], NULL);
    }
    free(vk.renderFinishedSemaphores);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vk.device, vk.imageAvailableSemaphores[i], NULL);
        vkDestroyFence(vk.device, vk.inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(vk.device, vk.commandPool, NULL);
    cleanupSwapchain();
    vkDestroyPipeline(vk.device, vk.graphicsPipeline, NULL);
    vkDestroyPipelineLayout(vk.device, vk.pipelineLayout, NULL);
    vkDestroyRenderPass(vk.device, vk.renderPass, NULL);
    vkDestroyDevice(vk.device, NULL);

#if ENABLE_VALIDATION
    PFN_vkDestroyDebugUtilsMessengerEXT destroyFunc =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (destroyFunc != NULL) destroyFunc(vk.instance, vk.debugMessenger, NULL);
#endif

    vkDestroySurfaceKHR(vk.instance, vk.surface, NULL);
    vkDestroyInstance(vk.instance, NULL);

    glfwDestroyWindow(vk.window);
    glfwTerminate();
}

//------------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------------
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Init GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // No OpenGL context
    vk.window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Vulkan Triangle - Phase 0", NULL, NULL);
    if (!vk.window) {
        fprintf(stderr, "Failed to create GLFW window!\n");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowUserPointer(vk.window, &vk);
    glfwSetFramebufferSizeCallback(vk.window, framebufferResizeCallback);

    // Init Vulkan
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffers();
    createSyncObjects();

    printf("Vulkan initialized successfully! Rendering...\n");

    // Main loop
    while (!glfwWindowShouldClose(vk.window)) {
        glfwPollEvents();
        drawFrame();
    }

    // Cleanup
    cleanup();
    printf("Clean shutdown.\n");

    return 0;
}
