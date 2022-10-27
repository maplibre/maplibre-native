# MapLibre Rendering Modularization Design Proposal

Before we dive in, a bit about our process.  Stamen Design, with a sub-contract to Wet Dog Weather, has been contracted by Amazon (AWS) to develop two proposals to upgrade MapLibre Native.  The goal is Metal support and the first proposal is this one: Modularizing the rendering support.

We are starting with a very sparse proposal, laying out our goals first.  Our intent is for the MapLibre community members and interested parties to add their own goals, which we can then discuss in detail.

While this is happening, we will be digging into the toolkit itself to inform a very specific **Proposed Change** section.  We'll fill that in over the 2nd and 3rd weeks of our work (Nov 7th and Nov 14).  That is more than two weeks away, yes.  We're consultants.  Time works differently for us.

The mechanism for this will be a Pull Request, which we have obviously just opened.  This will result in as much discussion as the community would like here, on the OSM Slack and by email or video call (we're available, so reach out).  As we wrap up our specific proposal, that discussion will hopefully reach a consensus and we'll be ready for a Yes or No on the PR by the 21st of November.  

Then we do it again for Metal.

## Motivation

MapLibre Native is currently depending on a deprecated rendering SDK (OpenGL) for iOS. It needs to move to Metal for iOS in some form. Moreover, there are other graphics SDKs to consider, like Vulkan for Android or Direct3D for Windows. Metal is an existential crisis on iOS, the others are desired but not required.

Thus the graphics implementations must diverge, either by doing so within MapLibre Native or depending on another toolkit to do the rendering entirely. We are proposing the former approach, with support for multiple rendering SDKs in the MapLibre Native toolkit itself.

This is the MapLibre Rendering Modularization Design Plan and the biggest goal is to pave the way for Metal support on iOS. But while we’ve got the hood open, so to speak, we’d also like to fix a few other things. So let’s get to it!

## Goals

We’re adding this section to the standard design template. What we will do here is state and discuss the various goals for this effort and either incorporate them or reject them, with a discussion of why we’re doing that.

*Please add your own goals that are relevant to the renderer modularization and we will discuss them*

- Make it easier to implement Metal on iOS while continuing support of OpenGL ES for Android (and others)
    This is the primary goal and what’s driving the effort. We need to support Metal one way or another.
- Existing SDK interfaces should not change between the current version and this one.
    Existing developers that use the current version of MapLibre Native should be able to recompile with the new version without changing external level code. If they made changes within the toolkit itself, obviously that’s different.
- The visual rendering should not change between the current version and this one.
    The new version of MapLibre Native would match the old version pixel for pixel on all platforms.
- Shader code should be easily findable.
    Opening the project (Xcode or Android Studio) should reveal a logical layout of OpenGL ES shaders, easily modifiable.
- The toolkit should not get any (or much) bigger when compiled.
    Many developers are sensitive to binary size. If there are opportunities to lower it, those may be worth pursuing in other efforts.
- The toolkit should not be any slower after the upgrade.
    In the general case, we should not be rendering at a lower frame rate, causing slower data loading times, or startup times.
- Shaders should be easy to replace at the dev level.
    A developer should be able to provide their own shader to replace implementation of any shader based functionality within the toolkit. For example, if you’d like to implement wide lines, you should be able to replace the current wide lines shader implementation with the same input data.
- Any well defined piece of rendering logic should be replaceable at the dev level.
    An example serves better here.  Let’s say you’d like to change the way tile sets are rendered to screen.  Not the loading, not the way styles are set up, just the rendering.  Every part associated with the rendering of tiles sets should be replaceable.  Some of these will be complex and a little difficult to understand, but at the very least you should be able to copy out the existing files, rename their classes and register your new version for the system to use.
- Ability to associate specific styles with specific rendering 
    A style should be able to call out how it’s rendered.  This would allow for new implementations to make use of existing style definitions.  For example, if you’d like to render a tile set as data rather than visual pixels (e.g. for weather), you could call out a rendering type which hooks into your own implementation.

## Proposed Change

Modularize: We will fill this out as we dig deeper into the technical design, informed by discussion of the Goals above.

## API Modifications

At present we do not expect to change any existing APIs at the public level.  We do expect to add some new API methods and classes.  We will detail these as we proceed.

## Migration Plan and Compatibility

We do not intend for these changes to be incompatible with the existing version.

## Rejected Alternatives

*Discuss what alternatives to your proposed change you considered and why you rejected them.*
