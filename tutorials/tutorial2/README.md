Vertex Buffers
--------

This code is the vertex buffer tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com) up to and including the section 
on the [index buffer](https://vulkan-tutorial.com/Vertex_buffers/Index_buffer). 
There is no unique SDL code beyond what was present in the Hello Triangle 
tutorial.

To build this tutorial, you will first need to compile the shaders in the
`assets/shaders` directory using the right script for your platform. Then
link this tutorial to VulkanSDL using the python script. No additional
configuration is necessary.

This code is presented without comments to make it easier to diff against 
the original (as an comments would appear in the diff).

### SDL Window Management

ur code uses `SDL_PollEvents` to handle windowed events rather than using a 
callback function like `SDL_AddEventWatch`. This is the preferred way to 
handle events in SDL, as events have to be processed on the same thread that
the window was created. We support two window events, namely quiting and 
resizing.

However, there is a [known issue with SDL](https://github.com/libsdl-org/SDL/issues/1059)
on Windows. Moving or resizing a window on Windows causes the application 
to freeze until the operating is complete. This can be seen in this demo
with the delay between resizing and when the triangle is recentered. This
delay is not present on Linux or macOS, as SDL supports continuous resizing
on those platforms (and mobile devices cannot be resized).

The solution to this problem is to move Vulkan rendering to a separate 
thread. However, this significantly alters the tutorial and comes with its
own challenges. Therefore, we do not do that here, and instead defer that
to a [later tutorial](../tutorial10/README.md).