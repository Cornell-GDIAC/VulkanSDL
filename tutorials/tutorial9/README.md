Compute Shaders
--------

This code is the compute shader tutorial from the 
[Vulkan Tutorial](https://vulkan-tutorial.com). Most of the modifications made
to this tutorial are similar to those made to the uniform buffer tutorial.
However, there is also a minor change to the compute shader to eliminate an
unfortunate interaction with the SDL window management (see below).

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

More than any other tutorial, this one shows off the problems with SDL and
Windows. This tutorial behaves perfectly fine on Linux and macOS, as well as
the mobile platforms Android and iOS. But on Windows, the particle simulation
freeze any time the window is moved or resized. That is because of a 
[well-known issue](https://github.com/libsdl-org/SDL/issues/1059) where 
(on Windows only) these actions block the main thread until they complete.

In this tutorial, this issue has an even worse effect than freezing the frame. 
The simulation measures the time between frames to figure out how far to move 
the particles. If the window is frozen, this could be for a very long time, 
sending the particles far offscreen. And because of an assumption in the compute 
shader (see below), these particles will never make it back on screen. As a 
result, it is very possible to end up with a blank window with no particles.

### Particle Simulation

In the original particle system from the 
[Vulkan Tutorial](https://vulkan-tutorial.com/Compute_Shader)
made an assumption about particle movement in order to keep the code as simple
as possible. This assumption was that, if a particle ever went offscreen, it 
would go back onscreen in a single animation frame. This assumption is 
apparent in the following code:

```
// Flip movement at window border
if ((particlesOut[index].position.x <= -1.0) || (particlesOut[index].position.x >= 1.0)) {
	particlesOut[index].velocity.x = -particlesOut[index].velocity.x;
}    
if ((particlesOut[index].position.y <= -1.0) || (particlesOut[index].position.y >= 1.0)) {
	particlesOut[index].velocity.y = -particlesOut[index].velocity.y;
}
```

As you can see, the way we handle an offscreen particle is to reverse the 
velocity to put it back on screen. As long as that velocity puts the particle
back on screen in the next frame, all is okay. But if that velocity is not
sufficient to put the particle back on the screen in one frame, the velocity
will forever flip back-and-forth, trapping the particle in place.

This is exactly what happens when the window freezes. The position of the
particle is the velocity times the time passed. The window freeze causes a long
period of time to pass, putting the particle well offscreen. But once it 
unfreezes, the frames are once again short, meaning that the particle does not
have enough time to get back on screen.

The only way to solve this problem is to add a new attribute to the class
`Particle` that tracks whether or not the particle is offscreen. For each
axis (x and y), we have a boolean that tracks whether or not the particle is
offscreen. Once it goes offscreen, we reverse the velocity and set the 
appropriate boolean to true. This keeps us from reseting the velocity until
the particle is back on screen. When the particle final returns onscreen, we
reset the boolean to false to continue normal behavior.

For technical reasons, we represent these booleans as a `vec2` with 1 being
true and -1 being false. The result is a slightly altered shader:

```
// Flip movement at window border
if ((particlesOut[index].position.x <= -1.0) || (particlesOut[index].position.x >= 1.0)) {
    if (particlesOut[index].offsides.x < 0) {
        particlesOut[index].velocity.x = -particlesOut[index].velocity.x;
        particlesOut[index].offsides.x = 1.0;
    }
} else if (particlesOut[index].offsides.x > 0) {
    particlesOut[index].offsides.x = -1.0;
}
    
if ((particlesOut[index].position.y <= -1.0) || (particlesOut[index].position.y >= 1.0)) {
    if (particlesOut[index].offsides.y < 0) {
        particlesOut[index].velocity.y = -particlesOut[index].velocity.y;
        particlesOut[index].offsides.y = 1.0;
    }
} else if (particlesOut[index].offsides.y > 0) {
    particlesOut[index].offsides.y = -1.0;
}
```

Of course, this means that we need to add this `offsides` attribute to the 
`Particle` struct as well.  Naively this would be done as follows:

```
struct Particle {
	vec2 position;
	vec2 velocity;
    vec4 color;
    vec2 offsides;
};
```

Unfortunately, this does not work. That is because the shader uses 
[std140](https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159) 
to represent the list of particles. This format requires that the size of
`Particle` be a multiple of the size of `vec4`. In the original tutorial, this
was easy, because `position` and `velocity` add together to be a `vec4` and
`color` is a `vec4` by itself.  But now we have added an additional `vec2`,
messing up the alignment.

The solution is to add an unused attribute for padding, like this:

```
struct Particle {
	vec2 position;
	vec2 velocity;
    vec4 color;
    vec2 offsides;
    // Padding required for std140 alignment
    vec2 padding;
};
```

Now our particle system works, and is resilient against window freezes. As you
can see, this is the only change that we have made to the original tutorial,
beyond what is required for window management.