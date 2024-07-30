Hello Triangle
--------

This code is the Drawing a Triangle tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com) up to and including the section 
on [swap chain recreation](https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation). 
Note that most of the differences in this code from the example stem from 
that recreation since window handling is different in SDL (see below).

The code uses one nonstandard SDL function: `App_GetAssetPath`. This is one 
of the extensions added by `SDL_app`. This function is an alternative to 
`SDL_GetBasePath` which does not need to be freed (it has static allocation).
It also gives us a little more control in the Visual Studio debugger (e.g. 
we not have to copy the assets into the output folder). If you do not like 
this function, you can switch to `SDL_GetBasePath`, but you will need to 
manually copy the asset files into the Visual Studio output directory (or 
modify the Visual Studio project to copy these files for you).

To build this tutorial, you will first need to compile the shaders in the
`assets/shaders` directory using the right script for your platform. Then
link this tutorial to VulkanSDL using the python script. No additional
configuration is necessary.

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
to freeze until the operating is complete. This can be seen in this demo
with the delay between resizing and when the triangle is recentered. This
delay is not present on Linux or macOS, as SDL supports continuous resizing
on those platforms (and mobile devices cannot be resized).

The solution to this problem is to move Vulkan rendering to a separate 
thread. However, this significantly alters the tutorial and comes with its
own challenges. Therefore, we do not do that here, and instead defer that
to a [later tutorial](../tutorial10/README.md).

Finally, we will note that the presence of continuous resizing on Linux and
macOS lead to a discovery of a problem with this tutorial. The tutorial does
not include the semaphores in the swap chain clean-up. A race condition
can cause these semaphores to be stuck waiting in a signaled state if this
happens. Therefore, window resizing requires that we include the semaphores
in the clean up. This change is present in this code.
