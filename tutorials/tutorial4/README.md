Texture Mapping
--------

This code is the texture mapping tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com) up to and including the section 
on the [combined image sampler](https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler). 

This code replaces `stb_image.h` with an implementation using SDL_Image. This
is for portability reasons, as Android does not have a regular filesystem.

This code is presented without comments to make it easier to diff against the 
original.  People who know me are aware of how painful it is for me to let code 
go without spec comments.