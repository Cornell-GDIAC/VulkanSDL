Compute Shader Revisited
--------

This code is our own addition to the tutorial sequence, highlighting the
challenges of working with SDL, particularly in a cross-platform setting.
A common theme throughout all the tutorials was the problem with window 
management in SDL. Application windows behave differently on different 
platforms. On Linux and macOS they can be resized and moved continuously. 
However, on Windows, resizing or moving the window causes the animation 
to freeze. And, of course, on mobile platforms the window cannot be moved 
or resized at all.

In the classic OpenGL setting there would not be to much that we could do
here. That is because OpenGL commands always have to be executed on the same
thread as their "context", which is the thread that created the window.
So if the window does anything to freeze that thread, it is going to freeze
the OpenGL calls as well.

But Vulkan is not OpenGL. We are no longer limited to rendering on the same
thread as the window manager. So even if the window manager thread is blocked,
the animation can keep going. This is an obvious solution to our problem,
and is indeed why many games put even handling and animation on different
threads.

In this tutorial, we take you through the process of separating these two
threads. As we shall see, there are challenges with this approach, and some
problems are simply insolvable. But we can use this technique to come up with
a compromise solution that is good enough.

## The Main Thread

The main thread is simply the one that invokes the `main` function in your 
application. A classic application has just one main thread, and all code is 
executed on this thread. Both SDL and C++ allow for additional threads.
However, some operating systems have some restrictions about what kind of code
can be executed off the main thread.

For example, macOS and iOS will **not** allow you make a window off the main 
thread. On these platforms, SDL initialization and window creation must happen 
on the main thread. Furthermore, on **all** platforms (not just macOS and iOS) 
event management (`SDL_PollEvents` and `SDL_PumpEvents`) has to occur on the 
same thread that the window was created. As a result, if you are writing a 
truly cross-platform SDL application (and this is the entire point of SDL), you 
should always initialize SDL and handle SDL events on the main thread.

Fortunately, this does not include rendering. So we can move the Vulkan 
`drawFrame` off to its own thread. But again there are some restrictions here.
The `VkSurfaceKHR` is created from the window. Its creation is platform
specific and it does not have the thread-safe guarantees that the other
`VkCreate` methods do. So, to be safe, we need to create it on the same
thread as the window. The same is not true of `VkInstance`; we can create
that on any thread that we want. But since the surface depends on the instance,
it makes sense to create all of these on the same thread.

As a result, you can see that our `Main.cpp` starts with the first four steps 
from the compute shader tutorial:

- `initWindow`
- `createInstance`
- `setupDebugMessenger`
- `createSurface`

Once that is done, we move to our second thread, which is represented by the
class `RenderThread`.

## The Render Thread

The methods in the render thread are all of the remaining methods from the
compute shader tutorial. This includes setting up the swapchain, the 
pipelines, and the command buffers. We have our own `mainLoop` which just 
includes `drawFrame` and the simulation timer. The event management is all 
kept in the main thread.

Note that the simulation timer no longer uses `SDL_GetTicks`. Like events, this
function has to be called on the same thread that initialized SDL. Fortunately, 
C++ has its own clocks, so we can do without. Indeed 
[steady_clock](https://en.cppreference.com/w/cpp/chrono/steady_clock) is a
very reliable clock that can give measurements in the microseconds or even
nanoseconds on some platforms.

The only methods new to `RenderThread` are those for communicating with the
main thread. Communication in our tutorial is always top-down. That is, the 
main thread communicates with the render thread by calling its methods, and 
never vice versa. The reason for this is synchronization. Communication between 
threads means shared state, and shared state means critical sections. To
simplify critical section support, it is best to keep communication in
just one direction if at all possible.

In our tutorial we have three ways of handling this communication

- promise variables
- atomic variables
- mutex variables

Each of these serve a different purpose.

### Promise Variables

A [promise](https://en.cppreference.com/w/cpp/thread/promise) variable is the 
C++ equivalent of a Vulkan fence. You want some part of your code to block 
until a task has been carried out. The promise signals that the task is done.

In our case, we do not want to start processing SDL events until the Vulkan
environment is completely initialized. We could have done this by putting these
into the constructor of `RenderThread`. While the `run` method of `RenderThread`
is executed on a second thread, the constructor is executed on the main thread.
However, we elected to push these into the thread simply to show that it could
be done.

This means that we have to wait until that part of the thread is complete.
So when we call the `start` method, we provide it a promise variable to
synchronize with. We have created a [future](https://en.cppreference.com/w/cpp/thread/future) 
from this promise, and block until the promise is signalled. This signal happens
at the start of `mainLoop` in `RenderThread`.

### Atomic Variables

[Atomic](https://en.cppreference.com/w/cpp/atomic/atomic) variables are 
designed to prevent simple data races. Reading from or writing to a variable
might seem like a simple operation, but it is not always. Sometimes it may 
involve multiple steps. So if one thread writes to a variable while another 
is reading from it, the result could be a corrupted value.

One way to resolve this problem is with mutex locks, but that is a very 
heavyweight solution if we are only looking at a single variable. When you
just have one variable, it is sufficient to make it atomic.

In our case, this is how we handle thread lifespan. Both `RenderThread` and
the main thread have `mainLoop` methods that essentially run forever until
something happens to shut them down. In both cases this is managed by a 
variable named `running`. If the SDL gets a quit event, it sets this to false,
shutting down the main thread. This in turn sets `running` in the render
thread to false so it can shutdown. We make this variable atomic so that 
the main thread can set it safely.

### Mutex Variables

[Mutex](https://en.cppreference.com/w/cpp/thread/mutex) variables are the
big guns. They are designed to solve critical section issues that you cannot
manage with atomics. In our case, we use it to manage window resizing. Window
resizing involves multiple variables (`framebufferResized` and `newExtent`)
and we do not want race conditions where they have inconsistent values. So
we use mutexes to make sure these two variables are read and written as a
transactional unit. Technically it is possible to do that with atomics,
but we also do not want these values to change during the entire execution
of `drawFrame`.

A mutex does not do anything by itself. Instead a mutex is used to create
[locks guards](https://en.cppreference.com/w/cpp/thread/lock_guard). No two
blocks of code with a lock guard around the same mutex can execute at the
same time. So in our code, the methods `resizeSwapChain`, `resizeWindow` and 
`drawFrame` all have lock guards that keep them from being executed 
simultaneously.

## SDL Window Management

If you compile and run the application, you will find that window movement is
no longer an issue.  On all platforms, including Windows, the particle 
simulation continues normally while the window is being moved. We did not 
have to do anything special to achieve this. While window movement blocks
the event handler on windows, the main thread does not hold any locks when
this happens, so `drawFrame` can execute normally.

The more interesting challenge is resizing. This is where we run into problems.
You will notice that our `Main.cpp` creates the window without the
`SDL_WINDOW_RESIZABLE` flag enabled. You are welcome to add this flag, but
the issues will be apparent very soon.

The problem is that window resizing is a continuous process. Suppose we receive
an event for a window resize in the main thread. We call `resizeSwapChain` 
in the main thread, which cannot be executed while `drawFrame` is in the middle
of its progress.  When it does execute, it communicates its information to the
render thread and releases its lock.  The `drawFrame` is now free to execute
with this new information, informing it to recreate the swap chain. But while
that is happening, the user continues to resize the window, making this 
information obsolete. While we have locks around `resizeSwapChain`, we do not
have locks around the resizing process itself.

At first glance, this might not be so bad. We are drawing to a stale swapchain,
but so what? The problem is that the swapchain is defined by an image 
attachment that we got from a surface (that surface that we created on the
main thread). When you change the window size, you change the extent of that
surface. If the swapchain and image attachment do not match up, the program 
might crash.

In practice, the only problems happen when the swapchain extent is larger than
that of the image attachement. If it is smaller, Vulkan will just draw to a
smaller section of the image attachment. If you have a Linux or macOS setup, 
enable `SDL_WINDOW_RESIZABLE` and watch what happens. You can increase the size 
of the window with no ill affects at all. But the second you start to shrink the
window, both platforms crash. In Xcode, macOS hits an assert in MoltenVK telling 
you that the swapchain extent is too large, and it stops executing. On Linux the 
window just locks up completely.

The only way to prevent this problem is to create a critical section between 
window resizing and `drawFrame`, preventing any resizing from taking place 
while that method is being executed. But as window resizing happens at the 
OS level, there is not much hope here. It might be possible with some very 
heavyweight locking, but the end result would just fuse these two threads 
into a single execution unit, eliminating any advantage we got from 
multithreading.

Interestingly enough, this problem does not exist on Windows. On Windows 
resizing causes a slight pause in the window animation, but after a second 
or two the window responds to your actions in real time. You can then grow 
or shrink the window with no issues. And when you let go, there is another 
pause before animation resumes. This suggests that Windows is performing 
some Vulkan synchronization for you under the hood. But none of this is 
documented, which means it is to be avoided at all costs. Undefined behavior
is bad.

The take away from all this is that **real time window resizing is impossible** 
when writing cross-platform SDL code. Linux and macOS need `drawFrame` to be 
on the main thread to handle this properly, while Windows does not want this
on the main thread at all.

### The Resizing Compromise

Fortunately, this does not mean that resizing is impossible. Games support
resizing all the time. What they do not support, however is *continuous*
resizing. Typically the user has some small selection of window sizes and
they choose one of the two. The game pauses whenever the user switches 
between window sizes.

This problem is a lot easier to manage because our software manually controls
the window resizing process. We can put a lock guard around the window resizing 
code, ensuring that it is never executed at the same time as `drawFrame`. This 
is exactly what we do in `resizeWindow`. 

This function is driven by keyboard controls. To try it out, press the keys
`"="` and `"-"`.  If you press `"="` the window will grow in size, while on
`"-"`, it will revert to the original size. Of course, these actions have
no affect on mobile devices. But they will behave correctly on all desktop
platforms.
