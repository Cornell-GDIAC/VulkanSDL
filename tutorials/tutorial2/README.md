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

Most of this code is presented without comments to make it easier to diff 
against the original (as any comments would appear in the diff).

### SDL Window Management

In SDL2, there was a known problem where moving or resizing a window on 
Windows would cause the application to freeze until the operating was 
complete. That is because operations like `SDL_PollEvent` may block on 
window operations, preventing the code from that frame from executing.
This is most visible in [later tutorials](../tutorial3/README.md) where
the image is being animated.

This demo uses SDL3 which is able to reduce, but not entirely eliminate,
this problem by separating event consumption and the draw loop into 
separate callback functions. There is still some noticeable pause in
resizing (particularly at the start of the resize), but it is not
a hard freeze like would appear in SDL2. More information about this
issue can be found in the 
[official documentation](https://wiki.libsdl.org/SDL3/AppFreezeDuringDrag).

A true solution to this problem would be to move Vulkan rendering to a 
separate thread. However, this significantly alters the tutorial and 
comes with itsown challenges. Therefore, we do not do that here, and 
instead defer that to the [final tutorial](../tutorial10/README.md).

As part of this window refactor, we also needed to make some important
changes to the semaphores code.  First of all, `renderFinishedSemaphores`
needs to be the same size as the number of images, not the number of
frames in flight. While earlier versions of Vulkan allowed this behavior,
the Vulkan validation layers will report an error here as of Vulkan 1.4.
We have fixed this issue in the tutorial, but it complicates the semphore 
allocation and cleanup a bit because now the vectors have different 
lengths.

In addition, the original tutorial does not include the semaphores in the 
swap chain clean-up. A race condition can cause these semaphores to be 
stuck waiting in a signaled state if this happens. Therefore, window 
resizing requires that we include the semaphores in the clean up.