//
//  RenderThread.h
//  Tutorial10
//
//  This is a version of Sascha Willem's compute shader tutorial that renders
//  off of the main thread. This allows us to continue animating even in the
//  presence of blocking operations. See the tutorial README.md for an overview
//  of the design decisions in this document.
//
//  This thread has ALMOST all of the tutorial Vulkan code in it. It does not
//  have the instance or surface initialization code. That is because the
//  surface is attached to the window and it must be initialized on the main
//  thread with that window.
//
//  For comparison purposes, we have kept comments on code taken from the 
//  tutorial to minimum, only pointing out changes that we have made.
//
//  Author: Walker White
//  Version: 7/27/24
//
#ifndef __SDL_WINDOW_H__
#define __SDL_WINDOW_H__
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <cstring>
#include <vector>
#include <array>
#include <optional>
#include <chrono>
#include <random>

/** The clock data type, using the steady clock from chrono */
typedef std::chrono::steady_clock steadyclock_t;
/** The timestamp data type, relative to our clock */
typedef steadyclock_t::time_point timestamp_t;

// Forward declaration of structs
struct QueueFamilyIndices;
struct SwapChainSupportDetails;

/**
 * Prints out the API for the given version.
 *
 * The optional patch argument is for cases in which the path is not part of
 * the actual version number.
 *
 * @param source    The API source (instance, driver, etc.)
 * @param version    The version number
 * @param patch        The patch number (if >= 0)
 */
static void print_version(const char* source, uint32_t version, int patch=-1) {
    uint32_t major = VK_VERSION_MAJOR(version);
    uint32_t minor = VK_VERSION_MINOR(version);
    uint32_t impl  = patch;
    if (patch < 0) {
        impl = VK_VERSION_PATCH(version);
    }
    SDL_Log("%s %d.%d.%d",source,major,minor,impl);
}

/**
 * Returns the absolute path to the given asset.
 *
 * This function allows us to use the asset/bundle directory on most devices,
 * but switch to the working directory in Windows for better Visual Studio
 * support
 *
 * @param asset The asset name
 *
 * @return the absolute path to the given asset.
 */
static std::string get_asset(const std::string& asset) {
#if defined (SDL_PLATFORM_WINDOWS)
    char* path = SDL_GetCurrentDirectory();
    std::string result = std::string(path)+asset;
    SDL_free(path);
#else
    std::string result = std::string(SDL_GetBasePath())+asset;
#endif
    return result;
}

    
/**
 * An offscreen Vulkan renderer
 *
 * This class is essentially the application class from the compute shader
 * tutorial, minus the window, instance, and surface. Those three are created
 * on the main thread. In fact, we could create ALL of the Vulkan elements on
 * the main thread and simply move the mainloop to the offscreen renderer.
 * However, we wanted to illustrate that it is safe to move everything off
 * thread.
 */
class RenderThread {
public:
    /**
     * Creates a new render thread
     *
     * The render thread starts with an instance, surface, and extent, which it
     * uses to build all Vulkan elements. As long as we do not change the surface
     * extent, no explicit synchronization is required in Vulkan, as the relevant
     * functions are thread safe.
     *
     * This function does *not* start the thread. Call {@link start} to begin
     * the thread.
     */
    RenderThread(VkInstance instance, VkSurfaceKHR surface, VkExtent2D extent);
    
    /**
     * Destroys this render thread and all its contents.
     *
     * This destructor stops the drawing thread if it is not already stopped.
     */
    ~RenderThread();
    
    /**
     * Executes the code for the render thread
     */
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

    /**
     * Starts the render thread.
     *
     * The thread will first initialize the swap chain and pipelines. Once those
     * are ready, the promise will be signaled, so that any waiting thread can
     * move forward. It will then start the main loop and execute it until the
     * method {@link stop} is called.
     *
     * Calling this method on an active render thread has no effect.
     */
    void start(std::promise<void>&& p);
        
    /**
     * Stops the render thread.
     *
     * This function will stop the main loop and proceed to clean-up, disposing of
     * all Vulkan resources. It will block until the clean-up process is complete.
     * Technically, this thread can be restarted once it is stopped.
     *
     * Calling this method on a dormant render thread has no effect.
     */
    void stop();
    
    /**
     * Resizes the swap chain in response to a window resize event.
     *
     * This method is called either in response to a manual window change (e.g.
     * SDL_SetWindowSize) or in response to the user dragging the window. The
     * latter is not thread safe, and has been disabled (though you can re-enable
     * it yourself to learn the hard way).
     *
     * This method actually has no effect in response to a manual window change,
     * as it does not change any values that were not already changed in the 
     * method {@link resizeWindow}. It is mainly provided to support continuous
     * window resizing as a comparison.
     *
     * Note that this method is actually called on the main thread, not in the
     * render thread. Therefore it requires a lock guard to protect the critical
     * section.
     *
     * @param w The new swapchain width
     * @param h The new swapchain height
     */
    void resizeSwapChain(int w, int h);

    /**
     * Resizes the SDL window in response to a keyboard event.
     *
     * This method is called when the user presses either the "-" or the "=" key.
     * It provides discrete window resizing which (unlike continuous resizing) 
     * is thread safe. The "-" key puts the window at 800x600, while "=" puts it
     * at 1200x900.
     *
     * Technically calling this method will invoke {@link resizeSwapChain} which
     * will notify this thread that the swap chain should change. However, we 
     * cannot permit any race conditions between the time this method is called
     * and the next {@link drawFrame} is invoked. Therefore, this method
     * immediately signals that the swap chain should be recreated.
     *
     * Note that this method is actually called on the main thread, not in the
     * render thread. Therefore it requires a lock guard to protect the critical
     * section. This is also why it is safe to modify the window here -- as we
     * are modifying it on the main window thread.
     *
     * @param window    The SDL window to resize
     * @param w         The new swapchain width
     * @param h         The new swapchain height
     */
    void resizeWindow(SDL_Window* window, int w, int h);

private:
    // TUTORIAL CODE (Provided without comments)
    VkInstance instance;
    VkSurfaceKHR surface;
    
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue presentQueue;
    
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    
    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;
    
    VkCommandPool commandPool;

    std::vector<VkBuffer> shaderStorageBuffers;
    std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> computeDescriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkSemaphore> computeFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> computeInFlightFences;
    uint32_t currentFrame = 0;
    
    timestamp_t timestamp;
    float lastFrameTime = 0.0f;
    
    std::thread* thread;
    std::mutex guard;
    std::promise<void> barrier;

    std::atomic<bool> running = false;
    
    bool framebufferResized = false;
    VkExtent2D theExtent;
    VkExtent2D newExtent;
        
    void initVulkan();
    void mainLoop();
    void cleanup();

    void cleanupSwapChain();
    void recreateSwapChain();

    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();

    void createImageViews();
    void createRenderPass();
    void createComputeDescriptorSetLayout();
    void createGraphicsPipeline();
    void createComputePipeline();
    void createFramebuffers();
    void createCommandPool();
    void createShaderStorageBuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createComputeDescriptorSets();
    void createCommandBuffers();
    void createComputeCommandBuffers();
    void createSyncObjects();
    void drawFrame();

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkShaderModule createShaderModule(const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
    void updateUniformBuffer(uint32_t currentImage);
};

#endif /* __SDL_WINDOW_H__ */
