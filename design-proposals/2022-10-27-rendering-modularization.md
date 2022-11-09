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

## Goals / Evaluation metrics

This proposal contains details as to the design of a refactored library. If this design is implemented as detailed, we expect the following to be true:

1. Developers should be able to continue using the OpenGL renderer with this refactor.

2. This refactor will allow migration to Metal on IOS with less overhead than a migration without this explicit modularization. In other words, migration will be substantially simplified.

3. The resulting compiled binary from the modularized refactor should not increase more than 5% when compiled.

4. The modularization refactor should be visually identical to the current implementation when using OpenGL. All existing render tests should pass, and developers that continue to use this library with an OpenGL renderer should not experience any visual degradation such as artifacts.

5. After refactor, the startup time for applications build with this library should not increase. This will be measured by the startup time of [this](https://github.com/maplibre/maplibre-gl-native/blob/main/platform/ios/platform/ios/iosapp%20UITests/iosapp_UITests.swift) iOS test app.

6. When implemented with a given style and set of tilesets, the compiled library should not decrease the rendering frame rate.

7. This refactor will not introduce data loading or other bottlenecks that will negatively impact rendering performance.

8. Developers currently using this library will not see any breaking API changes, and will be able to seamlessly upgrade versions.


9. Optionally, shader code will no longer be compiled from the [webgl source](https://github.com/maplibre/maplibre-gl-js/tree/main/src/shaders), and should be organized within the `maplibre-gl-native` repo.

10. Shader code will implemented in a modular fashion, such that replacement of any given shader can happen without impacting other shaders

11. Layer rendering logic should be replaceable at the dev level.  All the necessary support should be exposed and a developer should be able to take over representation of any given layer.

12. Developers should be able to associate specific styles with specific layer rendering.  The style sheet should be able to call out the type of representation it would like beyond the default.

13. Developers should be able to add new visual representations fed by existing geometry types.  Data display driven by tile sets, for example.

14. We should be able to drop out unused layer types with a compile flag.

## Proposed Change

### Shader Representation

We need an external representation for shaders so they can be added or replaced by developers.

The shaders need a representation visible outside the toolkit and we need to change the way they’re handled inside the toolkit.  We need to be able to replace them, add brand new ones, and allow developers to control their shaders.

#### The way it is now

Individual shaders are represented by an object class with massive amounts of template logic and a minimum of in-line comments.  They’re opaque from the outside and only controllable through vector tile data and styles. 

The [render_raster_layer](https://github.com/maplibre/maplibre-gl-native/blob/main/src/mbgl/renderer/layers/render_raster_layer.cpp), as an example, asks for the instantiation of (eventually) RasterProgram.  Rather than having the source for the program, that then pokes into a compressed chunk of memory that contains the source, which is then uncompressed and fed into OpenGL for compilation.

All of this makes sense in context, but is completely inaccessible outside the toolkit and rather incomprehensible when making changes.

#### Required changes
To let developers control their own shaders we’ll need to:
- Allow shaders to be instantiated with shader source (one option).
- Allow shaders to be instantiated by reference to unique names in pre-compiled file (Metal does this).
- Set uniform values by name
- Set uniform structs by name (Metal uses structs)

#### Benefits
We'll be able to:
- Create entirely new shaders to do new and different things.
- Replace existing shaders without having to recompile the whole set.
- Let developers communicate with their own shaders.

#### For GLES we’ll need to:
- Push aside the existing Program hierarchy and rename it with a GLES extension.
- Allow for named uniforms for new Programs
- We can probably ignore that requirement for the existing shaders and just wrap them

### Shader Registry

Once developers can create their own shaders, we’ll need a way of managing them, thus the Shader Registry.  This registry will associate a given shader (Program) with a name.  Basic functionality will include:
- Add a new shader by name
- Replace an existing shader by name
- Query a shader by name
- Publish the well known names of the core shaders for the basic functionality

Layers already go through a level of indirection to instantiate or reference their associated programs.  The shader registry could be inserted into that get*LayerPrograms() pattern fairly easily.

The GLES version of the Shader Registry can encapsulate the simple (but fairly weird) logic of shader source lookup that exists now.

### Rendering Passes

Modern graphics pipelines use more than one pass to create a visual representation.  It's actually pretty unusual to have a single rendering pass in your toolkit.  These are used for a dizzying array of purposes, including lightning effects, shadows, atmosphere and many others.  We (Wet Dog Weather) use separate rendering passes to draw data intermingled with map elements.

Without getting too deep into specifics, early rendering passes allow the developer to use the power of the rasterizer for their own data.  Later rendering passes are typically using to decorate the map with effects.

#### The way it is now:
The [low level rendering logic](https://github.com/maplibre/maplibre-gl-native/blob/main/src/mbgl/renderer/renderer_impl.cpp) in the toolkit supports only one rendering pass with ordering as the main input.

#### Required changes:
To flexibly implement multiple rendering passes we will need to:
- Keep track of them by name and order
- Implement Render Targets (see below)
- Outputs from one pass as inputs to another
- Optimize out passes without geometry
- Expose all of this outside the toolkit

#### Benefits
Similar to what was discussed above:
- Using the rasterizer to sort out your data ahead of time.  We do this a lot with weather data.
- Reducing the size of a very costly shader.  The bigger your shader, the longer it takes.  If you have some fancy effect, it can probably be rendered at lower resolution and then laid over the top of the final scene.
- Doing post-process effects on the map.  In games you might think of lens warping, wipes, and such.  For mapping, this tends to be more merging data layers or things like glow.

### Off Screen Render Targets

The toolkit already has support for one offscreen render target.  You can either set things up “normally” where the toolkit draws to a window or you can set things up to draw to memory and hand you back the resulting image.  This is a little like that, but in real time.

Having an explicit off screen render target lets the developer do something like a snapshot in real time.  That is, run some sort of logic on the data being rendered before it gets its final visual representation.

#### Required Changes:
To support Rendering Passes we’ll need Render Targets we can access in a way similar to a texture.  They’ll be set up with:
- Settings such as bit depth and width/height (you don’t always want full screen resolution)
- Allow geometry to specify which one(s) they’ll render to as a “target”
- Query and snapshot

This may seem a bit of a detour, but it’s fairly easy to get the basics of it in place and allow for further development as developers might add new shaders and varieties of targets.

#### Benefits
It's a core idea in many rendering toolkits, but for MapLibre Native it will let us:
- Reach into the rendering pipeline to make use of processed data without adding a GPU<->CPU round trip.
- Make it easier to assemble things like videos in real time.
- Help define how the rendering passes work with fixed points of output.

*More changes to follow.*

## API Modifications

At present we do not expect to change any existing APIs at the public level.  We do expect to add some new API methods and classes.  We will detail these as we proceed.

## Migration Plan and Compatibility

We do not intend for these changes to be incompatible with the existing version.

## Rejected Alternatives

*Discuss what alternatives to your proposed change you considered and why you rejected them.*
