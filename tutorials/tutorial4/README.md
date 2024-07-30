Texture Mapping
--------

This code is the texture mapping tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com) up to and including the section 
on the [combined image sampler](https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler). 

This code replaces `stb_image.h` with an implementation using SDL_Image. This
is partly for portability reasons, but mainly because SDL_Image provides a 
wider array of file formats.

This code is presented without comments to make it easier to diff against 
the original (as an comments would appear in the diff).

### SDL Window Management

Our code uses `SDL_PollEvents` to handle windowed events rather than using a 
callback function like `SDL_AddEventWatch`. This is the preferred way to 
handle events in SDL, as events have to be processed on the same thread that
the window was created. We support two window events, namely quiting and 
resizing.

However, there is a [known issue with SDL](https://github.com/libsdl-org/SDL/issues/1059)
on Windows. Moving or resizing a window on Windows causes the application 
to freeze until the operating is complete. This is particularly egregious 
in this demo, as the spinning animation will freeze whenever the window is
moved. This delay is not present on Linux or macOS, which behave normally.

The solution to this problem is to move Vulkan rendering to a separate 
thread. However, this significantly alters the tutorial and comes with its
own challenges. Therefore, we do not do that here, and instead defer that
to a [later tutorial](../tutorial10/README.md).
