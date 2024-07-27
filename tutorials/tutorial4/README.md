Texture Mapping
--------

This code is the texture mapping tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com) up to and including the section 
on the [combined image sampler](https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler). 

This code replaces `stb_image.h` with an implementation using SDL_Image. This
is partly for portability reasons, but mainly because SDL_Image provides a 
wider array of file formats.

To build this tutorial, you will first need to compile the shaders in the
`assets/shaders` directory using the right script for your platform. Then
link this tutorial to VulkanSDL using the python script. No additional
configuration is necessary.

This code is presented without comments to make it easier to diff against the 
original.  People who know me are aware of how painful it is for me to let code 
go without spec comments.