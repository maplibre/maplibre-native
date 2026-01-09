# Complex, Animatable, Interactive Annotations Proposal

An annotation as described in this proposal is a piece of UI that is drawn on the map at a single coordinate. In Apple MapKit this would be an MKAnnotationView, and in Google world this would be similar to a Marker. Annotations are expected to be user interactable, animatable and allow for complex ui to be represented.

## Motivation

Most mobile map experiences display some sort of custom content on the map, often in the form of annotations. MapLibre’s current annotation system is difficult to use, performs poorly, and lacks many of the features developers expect from modern mapping libraries.

This proposal introduces the concept of annotations that are complex, animatable and interactive to MapLibre’s core. The first phase allows bitmap backed annotations to be rendered. The second phase will enable native platform views (iOS, Android) to be rendered directly by MapLibre Native.

These new annotations will provide a more flexible tool for drawing user content on the map that is more in line with what developers expect from the built-in mobile map toolkits. The focus on animations and interactivity allows developers to create differentiated map experiences that feel made for mobile.

By introducing a specialized rendering path for annotations, we bypass some of the limitations of the current layer-based system. This leads to better performance, especially in scenarios with many dynamic elements, where full layout re-calculations would otherwise cause noticeable lag and degrade the user experience.

The customization options that these annotations will provide, together with the existing MapLibre stack, aim to offer a system that is more adaptable and robust than those from Google or Apple.

## Proposed Change

### Phase 1

For this phase we would like to just build our support for bitmaps. The idea being that all native views eventually become bitmaps and supporting them robustly will eventually allow the renderer to draw native views correctly.

* Introduce “Annotation” as a first class citizen of the MapLibre Native C++ Core.
* Annotations rely on their own bitmap handling and not on the symbol layer texture atlas. Bitmaps should be able to be transformed and quickly swappable to support animations.
* Annotations can define a label (reuse symbol layer logic).
* Annotations provide callbacks and can trigger full map re-renders. These callbacks need to be in sync with the native render loop so that animations can be synchronized properly.
* Annotations participate in label and symbol layer collisions.
* [Stretch] Annotations support clustering, like symbol layers do.
* Platform layers allow annotations to participate in hit tests and “onTap” events.
* Layout, styling and positioning of Annotations can happen outside of the style’s layout and paint loops. However they should still participate in the rendering process.

### Phase 2

Once we have a system in place that can handle bitmaps we would like to take it one step further and support native platform views. The changes described here would likely be done inside the individual platforms.

For inspiration we can look at Flutter and their rendering engine (see https://github.com/flutter/flutter/wiki/Texture-Layer-Hybrid-Composition and https://github.com/flutter/flutter/wiki/Hybrid-Composition-iOS).

The big challenges here would be:
1. Going from Native View -> bitmap
2. Correctly handling touch events
3. Keeping the views in sync with map and the system

#### Some potential challenges (ios centric):
1. UIViews animate using a framework called CoreAnimation, much of which actually happens on a inaccessible rendering backend. However, with Metal it is now possible to synchronize with Core Animation via https://developer.apple.com/documentation/quartzcore/cametallayer/1478157-presentswithtransaction. Core Animation will animate your view either through transforms or any number of techniques. We need to capture each frame of the animation and the state of the bitmap so that maplibre-gl can render it.
2. Touch events are notoriously tricky to get right (even the Flutter team points it out as a difficult problem). We need to come up with a way to correctly proxy the touch events from the MapView to the UIView backed annotations (which may have complex gesture recognizers attached to them). We may want to settle for limited touch support (select/deselect via single tap), but full touch will be complex.
3. The view content itself may not represent what is actually being drawn, for example imagine an annotation backed by a UI view that has parts of the view hidden but can be shown through some touch event. Which parts of this view should participate? Should the whole view (based on the frame) or the visible elements. And lastly, back to (1) how do these get sync'd with the map in case of animations. We can start with the simpler case of just looking at bounding boxes as well as checking for visibility of the superview.


## API Modifications

APIs should largely stay the same as this is an additive change. We can discuss the possibility of removing the existing “Annotation Manager” style code in the platforms as a Phase 3.

## Migration Plan and Compatibility

No migration plan needed unless we introduce a Phase 3 to cleanup existing platform implementations.

## Challenges With Current System

Historically the gap has been somewhat filled by the different platform layers. On the iOS platform there is a MapKit style “AnnotationDelegate”, which in some cases draws directly to the mglMap core and in others will add views as subviews to the MGLMapView. On Android there is a whole proposal for updating the Annotations API (design-proposals/2023-06-17-android-annotations.md). Note: This proposal is for purely additive code and we do not plan to change any of these existing systems at this time. Those who have attempted to make complex map applications using these workarounds quickly run into performance issues and other problems like incompatibility with clustering or label collisions.

If one were to attempt to create such a system using SymbolLayers, they will quickly find that they need to build out a whole system to handle touch events (usually involving querying a screen rect for map features). They will find that if they want to draw many different high resolution images they now need to deal with manually adding images to the stylesheet and the performance implications of doing so. They will also find that any attempts at animating the content can cause full layout recalculations of the map and will rarely animate as expected.
