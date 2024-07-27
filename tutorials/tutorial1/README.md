Hello Triangle
--------

This code is the Drawing a Triangle tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com) up to and including the section 
on [swap chain recreation](https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation). 
Note that most of the differences in this code from the example stem from that 
recreation since window handling is different in SDL.

The code uses one nonstandard SDL function: `App_GetAssetPath`. This is one 
of the extensions added by `SDL_app`. This function is an alternative to 
`SDL_GetBasePath` which does not need to be freed (it has static allocation).
It also gives us a little more control in the Visual Studio debugger. If you 
do not like this function, you can switch to `SDL_GetBasePath`, but you will 
need to manually copy the asset files into the Visual Studio output directory
(or modify the Visual Studio project to copy these files for you).

To build this tutorial, you will first need to compile the shaders in the
`assets/shaders` directory using the right script for your platform. Then
link this tutorial to VulkanSDL using the python script. No additional
configuration is necessary.

This code is presented without comments to make it easier to diff against the 
original.  People who know me are aware of how painful it is for me to let code 
go without spec comments.