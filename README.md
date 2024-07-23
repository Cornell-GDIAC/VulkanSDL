# VulkanSDL
---
VulkanSDL is a package to simplify the build processes for Vulkan applications built on top of SDL. It is a variation of SDL App, a package builder that we designed for SDL Apps on top of OpenGL (and which is the core of CUGL, our simple game library).

While this package does add a few minor C functions to SDL, it mainly exists to minimize the work need to build a new project. While this may seem like a non-issue (CMake exists), creating SDL applications for mobile devices is nontrivial. Beyond compiling the software, there are issues with configuring the app permissions, setting the unique identifier, and assigning the asset directories. This is especially tricky when many of these settings are in things like plists or manifests not manageable by CMake.

This configuration process is particularly a problem for teams doing cross-platform development on Android and iOS together. We use SDL at Cornell to teach a course on cross-platform mobile game development. Before SDL App, we regularly had students submit assignments that kept the unique identifiers of our sample code, and therefore conflicted with each other during testing. Furthermore, students would develop using their platform-specific IDE (Android Studio or XCode), and this often led to the Android and iOS builds going out-of-sync.

To solve this problem, VulkanSDL is itself a Python script. This Python script enables the creation of an IDE project (Android Studio, Visual Studio, XCode, or CMake) for your application. The configured project is ready to build and deploy, minimizing the amount of time needed to port to a new platform. See the [instructions](scripts/APP.md) for how to use this script. These instructions are stored as a README in the release folder.

VulkanSDL is regularly tested against Android, Linux, iOS, macOS, and Windows. It also provides a Flatpak project for packaging an x86 Linux build for the SteamDeck. While SDL itself supports a wider variety of platforms, these are the platforms we need for our course at Cornell, and are therefore the only ones we support at this time.

## Dependencies

VulkanSDL itself is a software release composed of the following packages:

- SDL2, version 2.30.5
- SDL2-Image, version 2.8.2
- SDL2-TTF, version 2.20.1

These are integrated by source. On mobile devices, these have to be built from source anyway, so we find it easiest to just to link these as submodules to this project.

Whenever possible, we use the Vulkan installed on the deployment platform. However, for deployment, it is always ideal to keep the Vulkan version stable, so we include tested libraries for Windows and macOS/iOS.

- **Windows**: VulkanSDK 1.3.290
- **macOS**: MoltenVK 1.2.5 (newer versions have an issue)
- **iOS**: MoltenVK 1.2.10

Feel free to update these with newer versions of the library as you see fit. Libraries are installed in the following folders:

- `templates/apple/Frameworks`
- `templates/windows/dlls`
- `templates/windows/libs`


## Building VulkanSDL

VulkanSDL is a source code release (with minimal dynamic libraries). Therefore, it is packaged, not compiled. To create a release from this repository, run the python program `build.py` in this folder as follows:

	python build.py [<directory>]

You will need the module [PyYAML](https://pyyaml.org) to run this command. The optional `<directory>` is the location to store the release. By default it is stored  in a folder called `release`.  The version of the package can be set with the optional flag, `-v, --version`.

## Demos

We have included ports of the first few steps of the [Vulkan Tutorial](https://vulkan-tutorial.com/) to show off VulkanSDL. They come complete with their own `config.yml` files for configuration. After creating a release, read the [instructions](scripts/APP.md) for how to use the release to build the demos.