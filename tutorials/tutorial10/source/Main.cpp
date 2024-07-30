//
//  Main.cpp
//  Tutorial10
//
//  This is a version of Sascha Willem's compute shader tutorial that renders
//  off of the main thread. This allows us to continue animating even in the
//  presence of blocking operations. See the tutorial README.md for an overview
//  of the design decisions in this document.
//
//  This file is for the main thread. It is limited to the window (which must
//  be on the main thread), the instance, and the surface. All other Vulkan
//  elements are in the second thread.
//
//  For comparison purposes, we have kept comments on code taken from the
//  tutorial to minimum, only pointing out changes that we have made.
//
//  Author: Walker White
//  Version: 7/27/24
//
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "RenderThread.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

// TUTORIAL CODE (Provided without comments)
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#if defined (__MACOSX__) || defined(__IPHONEOS__)
#define USE_MOLTEN 1
#endif

// Tutorial functions
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// APPLICATION CODE
/**
 * A variation of ComputerShaderApplication that renders off the main thread.
 *
 * This class creates the window on the main thread, and handles all window
 * events.  Because the surface is tied to the window, we create the instance
 * and window on this thread as well. But everything else is off thread and
 * synchronized explicitly.
 */
class ComputeShaderApplication {
public:
    
    /**
     * Runs the application.
     *
     * The mainLoop will continue to execute until it receives a SDL_QUIT
     * instruction.
     */
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // Window: must be made on main thread
    SDL_Window* window;

    // Instance: could be offthread, but has to be made before surface
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    // Surface: must be made on thread with window
    VkSurfaceKHR surface;

    // The offscreen renderer
    RenderThread* thread;
    
    // A promise barrier to synchronize with RenderThread
    std::future<void> barrier;

    /**
     * Initializes the SDL window.
     *
     * Resizing is currently turned off. That means discrete resizing is
     * possible, but continuous resizing is not (as it is not thread safe).
     */
    void initWindow() {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_Vulkan_LoadLibrary(nullptr);
        Uint32 sdlflags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;

        // UNCOMMENT AT YOUR PERIL
        // sdlflags |= SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow("Vulkan",
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                  WIDTH, HEIGHT, sdlflags);
        thread = nullptr;
    }
    
    /**
     * Initializes the Vulkan components
     *
     * This creates the instance and the surface. Once it has that, it creates
     * and starts the render thread. This method blocks until the render thread
     * has finished initialization.
     */
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        
        VkExtent2D extent = { WIDTH, HEIGHT };
        thread = new RenderThread(instance, surface, extent);
        
        std::promise<void> p;
        barrier = p.get_future();
        thread->start(std::move(p));
        
        // Block until render thread finishes initialization
        barrier.wait();
    }

    /**
     * Runs the main event loop
     *
     * This loop processes SDL events, but it does not do any drawing. Any
     * communication with the render thread takes place through methods that
     * have mutex lock guards to protect critical sections.
     */
    void mainLoop() {
        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        running = false;
                        break;
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                            thread->resizeSwapChain(event.window.data1, event.window.data2);
                        }
                        break;
                    case SDL_KEYDOWN:
                    {
                        SDL_Keycode key = event.key.keysym.sym;
                        if (key == SDLK_EQUALS && event.key.repeat == 0) {
                            thread->resizeWindow(window, WIDTH*1.5, HEIGHT*1.5);
                        } else if (key == SDLK_MINUS && event.key.repeat == 0) {
                            thread->resizeWindow(window, WIDTH, HEIGHT);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            
            // 120 FPS on input
            SDL_Delay(8);
        }
    }

    /**
     * Cleans up this application and shuts down SDL.
     *
     * This method destroys all Vulkan elements, including those in the render
     * thread.
     */
    void cleanup() {
        // This will block on thread cleanup
        thread->stop();
        delete thread;

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        SDL_DestroyWindow(window);
        SDL_Quit();
    }

// TUTORIAL CODE (Provided without comments)
    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        
#ifdef USE_MOLTEN
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        
        auto extensions = getRequiredExtensions();
        
#ifdef USE_MOLTEN
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif
        
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t extensionCount = 0;
        SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
        std::vector<const char*> extensions(extensionCount);
        SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

/**
 * Creates and runs the application
 *
 * @param argc  The number of command line arguments
 * @param argv  The command line arguments
 *
 * @return the application exit code
 */
int main(int argc, char* argv[]) {
    ComputeShaderApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
