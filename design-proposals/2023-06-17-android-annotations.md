# Design Proposal Template

## Motivation

`maplibre-native` contains a deprecated variant of annotation methods that work with objects such as `Marker`, `Polygon` and more that imitate the Google Maps API. On the other hand, `maplibre-plugins-android` repository contains an implementation that works with the objects `Symbol`, `Fill` and more. This situation is confusing to developers and users and should be cleaned up.

A first approach was to simply move code from the plugins repository to `maplibre-native` (https://github.com/maplibre/maplibre-native/issues/1154), but the codebases had derived too much to be feasible on the go (there is no obvious path towards moving the new code whilst un-deprecating the deprecated methods; these two goals are incompatible).

A better idea is to create a new and improved annotations API that combines the best of both worlds. 

## Proposed Change

### Design goals

The following ideas drive the design proposal for the improved API.

* (G1 **`maplibre-native`**) The annotation API should be a part of `maplibre-native`.
* (G2 **frictionless migration**) The new API should create as few friction as necessary for existing users of the annotations plugin during migrations.
* (G3 **Kotlin-first**) The API should facilitate Kotlin langauge features where appropriate.
* (G4 **power**) The annotations API should be as powereful as it was before. No functionality should be lost.
* (G5 **learnability**) At the same time, it should have a high learnability, such that new users find their way around the API with ease.
* (G6 **ease**) The API should be user-friendly and easy to use.
* (G7 **consistent state**) The API should prevents users from setting objects into an inconsistent state by design. We consider a state to be inconsistent if a set property does not have an effect because another property is unset. An example is that there can be no text size for a `Symbol` if the `Symbol` has no associated text.

### Changes

What conceptual changes facilitate these goals?

### On `AnnotationManager`s and why users usually don't want to see them

For a beginner, the concept of `AnnotationManager`s is hard to grasp. The following steps are necessary to add one single marker a given map:

1. Create `SymbolManager` using the map
2. Create `SymbolOptions`
3. Create `Symbol` using `SymbolOptions` on `SymbolManager`

Here we can observe a negative leak of internal implementation details – that one manager equals one layer in style – onto the user-facing API.Much more straight-forward is the following:

1. Create `Symbol`
2. Add `Symbol` to map

This proposal facilitates exactly the second, easier version →(G5 **learnability**) →(G6 **ease**), without compromising the ability to build upon our `*Manager` classes →(G2 **frictionless migration**) →(G4 **power**).

Users should be able to set click, drag and long-click listeners directly on annotations and not only on managers. Any manager listeners that consume the event should have priority.

Note that the direction in which things are added is reversed: in this proposal, we add `Symbol` to map, and not map creates `Symbol`. If a user decides to use `SymbolManager` directly, they add the `SymbolManager` to the map instead of passing the map to the `SymbolManager` during instantiation. This is possible only because we are implementing the API as a part of `maplibre-native` (G1).

A second advantage is that the user need not worry about multiple managers for different types of objects. For instance, when adding a polygon as well as a marker, they would need to handle a `SymbolManger` as well as a `FillManager`.

### State consistency and learnability

To avoid inconsistent states →(G7 **consistent state**), and also to be easier to learn →(G5 **learnability**), the API should wrap variables that belong together in logical object classes – for instance `symbol.text?.size` instead of `symbol.textSize`. See the API modifications below for a more exact specification of the proposed changes in this regard.

### `*Option` classes and why we don't need them

Option classes like `SymbolOptions` and `FillOptions` provide a setter syntax of `withProperty(p)` calls, each of which return `this` to allow chaining. With a Kotlin-first (G3) codebase, this is not necessary because the `.apply` extension function together with Kotlin setters results in more idiomatic and readable code.

As stated above, we want to be able to instantiate annotation classes directly. This means that the corresponding `*Option` class is not necessary and should be replaced with code like this:

```
Symbol(latLng).apply {
    icon = Icon(img)
    text = "label"
}.let { map.add(it) }
```

→(G3 **Kotlin-first**) →(G6 **ease**)

### Z layers

Once we imagine that users need not interact with `*Manager` classes, and that the map object has an overview over all annotations that it contains, it becomes obvious that users might want to assign differnt annotations different Z layer heights, without manually adding the objects to different managers (which would be needed to mix objects of different types). →(G6 **ease**)

They might also want to be able to set properties that are not data-driven on a per-item basis, even if this causes multiple layers to become necessary. The map object should automatically group the objects it receives into layers of compatible items and create appropriate `*Manager` objects for the user. →(G4 **power**)

If a user whishes to create a `*Manager` class themselves, they should be provided with the ability to insert their layer above or below a specific existing layer, or on top of a specific Z index. →(G4 **power**) All other annotations should be placed on top of all existing style layers.

### `ClusterOptions` and symbol collisions

This directly raises two more questions: how would users access the capabilities of `ClusterOptions`, and how can we set symbols to collide with each other (i.e. `setIconAllowOverlap(false)`)?

The proposed solution is to create a `ClusterGroup` or `CollisionGroup` that can be inserted to the at a specific Z index. It is forbidden to add two objects with conflicting, non-data-driven properties to the same `*Group`.

### On code generation

The annotations API currently uses code generation to generate the annotation objects. This should no longer be done in the future for the following reasons.

* Code generation incentivises non-specific, flat APIs where hiearchical objects or other specific constructs may make more sense.
* In a similar way, it makes it harder to assign documentation to specific methods →(G5 **learnability**).
* When switching to Kotlin and when removing the `*Options` classes (G3 **Kotlin-first**), boilerplate code is reduced by a significant amount.

Due to these reasons, it is simpler to reach our goals without code generation.

## API Modifications

The following lists concrete class defintions that should be available to users. All properties are to be understood as settable and gettable by appropriate (G3 **Kotlin-first**) getters and setters. Non-nullable variables are either the default value that is currently in the code or constructor parameters. Nullable `val`s are optional constructor parameters with default value `null`.

(NDD) represents a property that is not data-driven, such that two items that do not match in all (NDD) properties must not be placed in the same layer (and are therefore not handled by the same manager).

* `AnnotationManager`
    * to be specified
* `Annotation`
    * `var zLayer: Int`
    * `var draggable: Boolean`
    * `var data: JsonElement?`
* `Symbol extends Annotation`
    * `var latLng: LatLng`
    * `var icon: Icon?`
    * `var text: Text?`
* `Icon`
    * `val image: Image`
    * `val size: Float?`
    * `val rotate: Float?`
    * `val offset: PointF?`
    * `val anchor: Anchor?`
    * `val opacity: Float?`
    * `val fitText: FitText?` (NDD)
    * `val keepUpright: Boolean?` (NDD)
    * `val pitchAlignment: PitchAlignment?` (NDD)
* `enum Anchor`: `LEFT`, `RIGHT`, `CENTER`, `TOP_LEFT`, `TOP_RIGHT`, `BOTTOM_LEFT`, `BOTTOM_RIGHT` (inner class of `Icon` and `Text`)
* `FitText` (inner class of `Icon`)
    * `val width: Boolean`
    * `val height: Boolean`
    * `val padding: Padding?`
* `Padding` (inner class of FitText)
    * `val top: Float`
    * `val right: Float`
    * `val bottom: Float`
    * `val left: Float`
* `enum PitchAlignment`: `MAP`, `VIEWPORT`, `AUTO` (inner class of `Icon` and `Text`)
* `SdfIcon`
    * `val color: @ColorInt Int?`
    * `val halo: Halo?`
* `Image`: to be specified
* `Halo`
    * `val color: @ColorInt Int`
    * `val width: @ColorInt Int?`
    * `val blur: Float?`
* `Text`
    * `val string: String`
    * `val font: String[]?` TODO
    * `val size: Float?`
    * `val maxWidth: Float?`
    * `val letterSpacing: Float?`
    * `val justify: Justify?`
    * `val radialOffset: Float?`
    * `val anchor: Anchor?`
    * `val rotate: Float?`
    * `val transform: Transform?`
    * `val offset: PointF?`
    * `val opacity: Float?`
    * `val color: @ColorInt Int?`
    * `val halo: Halo?`
    * `val pitchAlignment: PitchAlignment?` (NDD)
    * `val lineHeight: Float?` (NDD)
    * `val variableAnchor: Anchor[]?` (NDD)
    * `val maxAngle: Float?` (NDD)
    * `val keepUpright: Boolean?` (NDD)
* `enum Justify`: `AUTO`, `LEFT`, `CENTER`, `RIGHT` (inner class of `Text`)
* `enum Transform`: `NONE`, `UPPERCASE`, `LOWERCASE` (inner class of `Text`)

## Migration Plan and Compatibility

<!-- If your change is incompatible with existing APIs, draft a migration plan to help users adapting to the new version once your change is completed. -->

## Rejected Alternatives

<!-- Discuss what alternatives to your proposed change you considered and why you rejected them.-->
