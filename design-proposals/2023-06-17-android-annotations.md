# Design Proposal for Android Annotations API

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

Here we can observe a negative leak of internal implementation details – that one manager equals one layer in style – onto the user-facing API. Much more straight-forward is the following:

1. Create `Symbol`
2. Add `Symbol` to map

This proposal facilitates exactly the second, easier version →(G5 **learnability**) →(G6 **ease**), without compromising the ability to build upon our `*Manager` classes →(G2 **frictionless migration**) →(G4 **power**).

Users should be able to set click, drag and long-click listeners directly on annotations and not only on managers. Any manager listeners that consume the event should have priority.

A second advantage is that the user need not worry about multiple managers for different types of objects. For instance, when adding a polygon as well as a marker, they would need to handle a `SymbolManger` as well as a `FillManager`.

Note that the direction in which things are added is reversed: in this proposal, we add `Symbol` to map, and not map creates `Symbol`. If a user decides to use `SymbolManager` directly, they add the `SymbolManager` to the map instead of passing the map to the `SymbolManager` during instantiation. This is possible only because we are implementing the API as a part of `maplibre-native` (G1).

This reversed approach allows us to rewrite similar calls to the ancient, Google Maps-style API that is currently part of `maplibre-native` to redirect them to the new implementation. This way, we can remove the legacy implementation and deduplicate the codebase.

A dependency injection pattern should be used to add a reference to the map to the annotation, such that it can notify the map on updates of itself. No annotation can be added to multiple maps, otherwise an exception is thrown.

### State consistency and learnability

To avoid inconsistent states →(G7 **consistent state**), and also to be easier to learn →(G5 **learnability**), the API should wrap variables that belong together in logical object classes – for instance `symbol.text?.size` instead of `symbol.textSize`. See the API modifications below for a more exact specification of the proposed changes in this regard.

### `*Option` classes and why we don't need them

Option classes like `SymbolOptions` and `FillOptions` provide a setter syntax of `withProperty(p)` calls, each of which return `this` to allow chaining. With a Kotlin-first (G3) codebase, this is not necessary because the `.apply` extension function together with Kotlin setters results in more idiomatic and readable code.

As stated above, we want to be able to instantiate annotation classes directly. This means that the corresponding `*Option` class is not necessary and should be replaced with code like this:

```
Symbol(latLng).apply {
    icon = Icon(bitmap)
    text = Text("label", color = Color.RED)
}.let { map.add(it) }
```

→(G3 **Kotlin-first**) →(G6 **ease**)

### Z layers

Once we imagine that users need not interact with `*Manager` classes, and that the map object has an overview over all annotations that it contains, it becomes obvious that users might want to assign different annotations different Z layer heights, without manually adding the objects to different managers (which would be needed to mix objects of different types) →(G6 **ease**).

They might also want to be able to set properties that are not data-driven on a per-item basis, even if this causes multiple layers to become necessary. The map object should automatically group the objects it receives into layers of compatible items and create appropriate `*Manager` objects for the user →(G4 **power**).

If a user whishes to create a `*Manager` class themselves, they should be provided with the ability to insert their layer above or below a specific existing layer, or on top of a specific Z index →(G4 **power**). All other annotations should be placed on top of all existing style layers.

**Runtime considersations**:

* The naive algorithm for grouping annotations runs in O(n²) with n as the total amount of annotations. However, through clever use of sorting algorithms, an implementation in O(n log n) is possible.
* The current implementation runs in O(n) with n as the amount of items in a layer for each update.
* In case a user needs this high efficiency (for displaying tens of thousands of markers), they should group their annotations in a manager themselves. Adding a manager on top should run in O(m) with m as the total amount of managers, then in O(n) with n as the amount of markers inside this manager's layer. So if only a constant amount of managers is used, the total is O(n).

### Icons

Users should not worry about manually adding icons to a layer's style →(G6 **ease**). For this reason, the API should take `Bitmap` objects (or, by extension, `Drawable`s that can be converted to `Bitmap`s) directly. Adding them to the style should happen automatically. Identical `Bitmap` objects should only be added once; the comparison is done using `Bitmap.sameAs` and an ID is generated automatically.


### `ClusterOptions` and symbol collisions

Two more questions are to be raised: how would users access the capabilities of `ClusterOptions`, and how can we set symbols to collide with each other (i.e. `setIconAllowOverlap(false)`)?

The proposed solution is to create a `ClusterGroup` or `CollisionGroup` that can be inserted to the at a specific Z index. It is forbidden to add two objects with conflicting, non-data-driven properties to the same `*Group`.

### `*Manager`

What about the `*Manager` API? It is no longer a user-facing forefront and primarily for advanced users. Much of the functionality that had previously been set on a `*Manager` level should be moved to either to the annotation objects directly (through automatic grouping into `*Manager` objects) or, in the case of `SymbolManager`, to `CollisionGroup`. It should also keep its functions that offer expert functionality like `create` with GeoJSON as a parameter →(G4 **power**). Advanced users have a better understanding of the underlying layer operations and the style spec. In this sense, they are mostly for internal use and are only exposed to facilitate special-need features.

For this reason, we shall not revise its API at this time, and instead only Kotlinify its getters and setters.

### Non-null by default

Object properties that have a default value should not be set to `null`, but to their default value instead →(G3 **Kotlin-first**). This makes it easier for users to find out what value is set by default →(G5 **learnability**).

The `setUsedDataDrivenProperties` mechanism should check for variations to the default values instead of for `null`. The default values should be set as constants and be kept in sync with any changes to default values in the style specifications.

`null` values should be used to signal to users that a property is not set and therefore not used (e.g. `halo` value unset = no halo; but `color` of a `Text` is never unset, instead it defaults to `Color.BLACK` per spec).

### On code generation

The annotations API currently uses code generation to generate the annotation objects. This should no longer be done in the future for the following reasons.

* Code generation incentivises non-specific, flat APIs – even where hiearchical objects or other specific constructs make more sense.
* In a similar way, it makes it harder to assign documentation to specific method →(G5 **learnability**). The current generated docs don't read like they were written with the Android implementation in mind, because they are copied from the spec.
* When switching to Kotlin and when removing the `*Options` classes (G3 **Kotlin-first**), boilerplate code is reduced by a significant amount.

Due to these reasons, it is simpler to reach our goals without code generation.

## API Modifications

The following list shows concrete public properties or functions that should be available to users. All properties are to be understood as settable and gettable by appropriate (G3 **Kotlin-first**) getters and setters. Non-nullable variables are either the provided default value from the spec, or (optional) constructor parameters. Nullable `val`s are optional constructor parameters with default value `null`.

(NDD) represents a property that is not data-driven, such that two items that do not match in all (NDD) properties must not be handled by the same manager and can not be placed in the same layer.

* `AnnotationManager`, `SymbolManager`, `FillManager`, `CircleManager`, `LineManager`
    * Kotlinified using Kotlin getters and setters, but essentially unchanged
* `Annotation`
    * `var zLayer: Int` (default `0`)
    * `var draggable: Boolean` (default `false`)
    * `var data: JsonElement?`
    * `var clickListener: OnAnnotationClickListener?`
    * `var dragListener: OnAnnotationDragListener?`
    * `var longClickListener: OnAnnotationLongClickListener?`
* `Symbol extends Annotation`
    * `var position: LatLng` (mandatory constructor parameter)
    * `var icon: Icon?`
    * `var text: Text?`
* `Icon`
    * `val image: Bitmap` (constructor alternatively takes and converts `Drawable`)
    * `val size: Float` (default `1f`)
    * `val rotate: Float` (default 0)
    * `val offset: PointF` (default `PointF(0f, 0f)`)
    * `val anchor: Anchor` (default `CENTER`)
    * `val opacity: Float` (default `1f`)
    * `val fitText: FitText` (default `FitText(false, false, Padding.ZERO)`) (NDD)
    * `val keepUpright: Boolean` (default `false`) (NDD)
    * `val pitchAlignment: Alignment?` (`null` represents `"auto"`) (NDD)
* `enum Anchor`: `LEFT`, `RIGHT`, `CENTER`, `TOP_LEFT`, `TOP_RIGHT`, `BOTTOM_LEFT`, `BOTTOM_RIGHT` (inner class of `Icon` and `Text`)
* `FitText` (inner class of `Icon`)
    * `val width: Boolean`
    * `val height: Boolean`
    * `val padding: Padding` (default `Padding.ZERO`)
* `Padding` (inner class of FitText)
    * `val top: Float`
    * `val right: Float`
    * `val bottom: Float`
    * `val left: Float`
    * constant `ZERO = Padding(0, 0, 0, 0)`
* `enum Alignment`: `MAP`, `VIEWPORT`
* `SdfIcon`
    * `val color: @ColorInt Int` (non-optional constructor parameter)
    * `val halo: Halo?`
* `Image`: to be specified
* `Halo`
    * `val width: @ColorInt Int` (default `0` would represent no halo, so non-optional constructor parameter; throw if a number <= 0 is provided)
    * `val color: @ColorInt Int` (non-optional constructor parameter)
    * `val blur: Float?` (throw if <= 0)
* `Text`
    * `val string: String`
    * `val font: String[]?` (throws if empty array is provided or if added to a map which does not have all the fonts in the font stack)
    * `val size: Float` (default `16f`)
    * `val maxWidth: Float` (default `10f`)
    * `val letterSpacing: Float` (default `0f`)
    * `val justify: Justify` (default `CENTER`)
    * `val anchor: Anchor` (default `CENTER`)
    * `val rotate: Float` (default `0f` in degrees)
    * `val transform: Transform?` (`null` represents `NONE`)
    * `val offset: Offset?`
    * `val opacity: Float` (default `0f`)
    * `val color: @ColorInt Int` (default `Color.BLACK`)
    * `val halo: Halo?`
    * `val pitchAlignment: Alignment?` (`null` represents `"auto"`) (NDD)
    * `val lineHeight: Float` (default `1.2f`) (NDD)
* `enum Justify`: `AUTO`, `LEFT`, `CENTER`, `RIGHT` (inner class of `Text`)
* `enum Transform`: `UPPERCASE`, `LOWERCASE` (inner class of `Text`)
* `sealed class Offset` (inner class of `Text`)
* `RadialOffset extends Offset` (inner class of `Text`)
    * `val offset: Float`
* `AbsoluteOffset extends Offset` (inner class of `Text`)
    * `val offset: PointF`
* `Line extends Annotation`
    * `var path: List<LatLng>`
    * `var join: Join` (default `MITER`)
    * `var opacity: Float` (default `1f`)
    * `var color: @ColorInt Int` (default `Color.BLACK`)
    * `var width: Float` (default `1f`)
    * `var gap: Float?` (throws if <= 0)
    * `var offset: Float` (default `0f`)
    * `var blur: Float?` (throws if <= 0)
    * `var pattern: Bitmap?`
    * `var cap: Cap` (default `BUTT`) (NDD)
    * `var translate: Translate?` (NDD)
    * `var dashArray: Float[]?` (throws if `.length % 2 != 0`) (NDD)
* `enum Join` (inner class of `Line`): `BEVEL`, `ROUND`, `MITER`
* `enum Cap` (inner class of `Line`): `BUTT`, `ROUND`, `SQUARE`
* `Translate` (inner class of `Line`, `Circle` and `Fill`)
    * `val offset: PointF` (maps to `line-translate`)
    * `val anchor: Translate.Anchor`
* `enum Anchor` (inner class of `Translate`): `MAP`, `VIEWPORT`
* `Fill extends Annotation`
    * `var paths: List<List<LatLng>>`
    * `var opacity: Float` (default `1f`)
    * `var color: @ColorInt Int` (default `Color.BLACK`)
    * `var outlineColor: @ColorInt Int?`
    * `var pattern: Bitmap?` (note: if set, `outlineColor` is ignored)
    * `var antialias: Boolean` (default `true`) (NDD)
    * `var translate: Translate?` (NDD)
* `Circle extends Annotation`
    * `var center: LatLng`
    * `var radius: Float` (default `5f`, in pixels)
    * `var color: @ColorInt Int` (default `Color.BLACK`)
    * `var blur: Float?` (throw if <= 0)
    * `var opacity: Float` (default `1f`)
    * `var stroke: Stroke?`
    * `var translate: Translate?` (NDD)
    * `var pitchScale: Alignment` (default `MAP`) (NDD)
    * `var pitchAlignment: Alignment` (default `VIEWPORT`) (NDD)
* `Stroke` (inner class of `Circle`)
    * `val width: Float` (throws if <= 0)
    * `val color: @ColorInt Int` (default `Color.BLACK`)
    * `val opacity: Float` (default `1f`)
* `enum PitchScale` (inner class of `Circle`): `MAP`, `VIEWPORT`
* `ClusterGroup` (defaults match former `ClusterOptions`)
    * `val symbols: List<Symbol> by Delegates.observable(mutableListOf(), onChange = { _, _, new -> … })`
    * `val colorLevels: Map<Int, /* @ColorInt */ Int> by Delegates.observable(mutableMapOf(Pair(0, Color.BLUE)), onChange = {_, _, new -> … })
`
    * `var radius: Int` (default `50`)
    * `var maxZoom: Int` (default `14`)
    * `var textColor: Expression` (default `Expression.color(Color.WHITE)`)
    * `var textSize: Expression` (default `Expression.literal(12)`)
    * `var textField: Expression` (default `Expression.toNumber(Expression.get("point_count"))`)
    * provides list-like functionality (`add`, `addAll`, `remove`, `removeAll`, `get`, `getAll`) to add new `Symbol`s
* `CollisionGroup`
    * `val symbols: List<Symbol> by Delegates.observable(mutableListOf(), onChange = { _, _, new -> … })`
    * `var symbolSpacing: Float` (default `250f`)
    * `var symbolAvoidEdges: Boolean` (default `false`)
    * `var iconAllowOverlap: Boolean` (default `false`)
    * `var iconIgnorePlacement: Boolean` (default `false`)
    * `var iconOptional: Boolean` (default `false`)
    * `var iconPadding: Float` (default `2f`)
    * `var textPadding: Float` (default `2f`)
    * `var textAllowOverlap: Boolean` (default `false`)
    * `var textIgnorePlacement: Boolean` (default `false`)
    * `var textOptional: Boolean` (default `false`)
    * `var textVariableAnchor: Anchor[]?` (throws on empty array)
* `MapLibreMap` (in addition to its other methods)
    * `add(Annotation...)`
    * `remove(Annotation...)`
    * `removeAllAnnotations()`
    * `add(AnnotationManager)` (added as highest z layer)
    * `add(manager: AnnotationManager, zLayer: Int)` (added on top of this z layer)
    * `add(manager: AnnotationManager, layerAbove: String)` (added below the layer specified by the parameter)
    * `add(manager: AnnotationManager, layerBelow: String)` (added above the layer specified by the parameter)
    * `remove(AnnotationManager...)`
    * `removeAllManagers()`
    * analog methods for `ClusterGroup` and `CollisionGroup`: `add, remove, removeAllGroups`

## Migration Plan and Compatibility

Users who do not wish to migrate can continue using the old Plugins Annotation API, which is to be deprecated.

Long-term, we aim for a removal of duplicate API from `maplibre-native`, as it had been deprecated for a while and we are to offer a similarly user-friendly alternative. As a migration path, we can rewrite existing old code to map to new code. This also allows us to deduplicate the internal implementation.

One remaining task is to consider supporting an `InfoWindow`-like feature with our new API. This should be the last piece missing for feature parity with the very old version.

## Rejected Alternatives

* Moving the existing code to `maplibre-native` without further considerations leads to chaos, because it would leave users with the choice between two rather mediocre APIs – one being mediocre by design, the other because it is deprecated. Additionally, it would leave the codebase with two confusingly similar codebases. With this proposal, we can offer one new and clean API, and redirect calls to the old API to the new one.
