# Vulkan Support
---
Ideally, we would just use the native Vulkan installed on the platform. And for both Android and Linux, we do just that. However, Windows, macOS, and iOS still require significant configuration in their projects (Visual Studio and Xcode). To automate the process, you need to install some files in the appropriate folders:

- `include`: The headers for using Vulkan
- `windows`: The Windows libraries and dlls
- `apple`: The macOS and iOS frameworks

These are larg(ish) files and so we have excluded them from the repository, though we do include them in the releases. To get these files, simply download the latest release and replace this folder with the `vulkan` folder from that one.

In general, these files are taken from the [VulkanSDK](https://vulkan.lunarg.com), and therefore you may wish to update them with the latest SDK. Note that you will need files from **both** the Windows version and the macOS version of the SDK. For the macOS SDK you must install the iOS tools, and you should use the `MoltenVK.xcframework` in the iOS folder (which supports both iOS and macOS).

You will note that in our releases we have an `.xcframework` for the vulkan loader and the validation layers. That is because we have added support for the iOS simulator, which is *not* currently supported by the VulkanSDK. We have compiled these ourselves and provided them as part of the release. This is another reason for using our files rather than relying on the VulkanSDK.

### LunarG VulkanSDL version 1.4.335