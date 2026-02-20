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
//  Version: 20/20/26
//
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

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

#include "RenderThread.h"

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

#if defined (SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
#define USE_MOLTEN 1
#endif

#if defined SDL_PLATFORM_IOS && SDL_PLATFORM_IOS == 1
#include <TargetConditionals.h>
#if TARGET_OS_MACCATALYST
#else
    #define MOBILE_PLATFORM 1
#endif
#elif defined SDL_PLATFORM_ANDROID
    #define MOBILE_PLATFORM 1
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
    bool initWindow() {
        SDL_Init(SDL_INIT_VIDEO);
        
        SDL_Vulkan_LoadLibrary(nullptr);
        if (!SDL_Vulkan_LoadLibrary(nullptr)) {
            SDL_Log("Error : %s",SDL_GetError());
            return false;
        }
#if defined(MOBILE_PLATFORM)
        Uint32 sdlflags = (SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY |
                           SDL_WINDOW_FULLSCREEN);
#else
        Uint32 sdlflags = (SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
#endif
        
        // UNCOMMENT AT YOUR PERIL
        sdlflags |= SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow("Vulkan", WIDTH, HEIGHT, sdlflags);
        if (window == NULL) {
            return false;
        }
        
        thread = nullptr;
        return true;
    }
    
    /**
     * Initializes the Vulkan components
     *
     * This creates the instance and the surface. Once it has that, it creates
     * and starts the render thread. This method blocks until the render thread
     * has finished initialization.
     */
    bool initVulkan() {
        try {
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
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
        return true;
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
        
        // This handles proper fallback
        uint32_t desiredVersion = VK_API_VERSION_1_3;
        uint32_t loaderVersion  = VK_API_VERSION_1_0;
        vkEnumerateInstanceVersion(&loaderVersion);
        
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = std::min(loaderVersion, desiredVersion);
        
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
        
        uint32_t apiVersion = VK_API_VERSION_1_0; // fallback default
        vkEnumerateInstanceVersion(&apiVersion);
        print_version("Instance",apiVersion);
        
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
        if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
    
    std::vector<const char*> getRequiredExtensions() {
        uint32_t extensionCount = 0;
        const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        std::vector<const char*> extensions(instance_extensions,instance_extensions+extensionCount);
        
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
    
    /** Class Interface */
public:
    
    ComputeShaderApplication() : window(NULL) {}
    
    ~ComputeShaderApplication() { cleanup(); }
    
    bool setup() {
        // Set the basic metadata
        if (!SDL_SetAppMetadata("Compute Shader", "1.0.0","com.vulkan-tutorial.tutorial10")) {
            SDL_Log("Setup Error: %s\n", SDL_GetError());
            return false;
        }
        if (!initWindow()) {
            SDL_Log("Setup Error: Failed to create window\n");
            return false;
        }
        if (!initVulkan()) {
            SDL_Log("Setup Error: Failed to initialize Vulkan\n");
            return false;
        }

        SDL_RaiseWindow(window);
        return true;
    }
    
    bool consume(SDL_Event *event) {
        switch (event->type) {
            case SDL_EVENT_QUIT:
                return false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                thread->resizeSwapChain(event->window.data1, event->window.data2);
                break;
            case SDL_EVENT_KEY_DOWN:
            {
                SDL_Keycode key = event->key.key;
                if (key == SDLK_EQUALS && event->key.repeat == 0) {
                    thread->resizeWindow(window, WIDTH*1.5, HEIGHT*1.5);
                } else if (key == SDLK_MINUS && event->key.repeat == 0) {
                    thread->resizeWindow(window, WIDTH, HEIGHT);
                }
                break;
            }
            default:
                break;
        }
        return true;
    }
    
    void run() {
        // 120 FPS on input
        SDL_Delay(8);
    }
};

/** SDL3 Callbacks */

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    ComputeShaderApplication* app = new ComputeShaderApplication();
    *appstate = app;
    if (app->setup()) {
        return SDL_APP_CONTINUE;
    }
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    ComputeShaderApplication* app = (ComputeShaderApplication*)appstate;
    try {
        app->run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ComputeShaderApplication* app = (ComputeShaderApplication*)appstate;
    if (app->consume(event)) {
        return SDL_APP_CONTINUE;
    }
    return SDL_APP_SUCCESS;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ComputeShaderApplication* app = (ComputeShaderApplication*)appstate;
    if (app != nullptr) {
        delete app;
    }
}
