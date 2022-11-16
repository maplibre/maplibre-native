# Expressions

Expressions are *domain specific language (DSL)* built by Mapbox for
vector styles. Mapbox Vector Style is used in Mapbox Vector Tiles.
Mapbox Vector Tiles is a vector tile specification initiated by Mapbox
which was later widely adopted by the geospatial community.

To recap, Mapbox Vector Styles have 3 components:

1.  Sources

2.  Layers

3.  Expressions

Sources contain geospatial features, in GeoJSON format. It's the data to
be visualized on the map. Layers contains data from a source. This is
where we expressions kick in. Expressions define how the data from a
source will be painted in a layer following a style. For example, a
heatmap, require the ability to paint features in different zoom levels
with different colors. Expressions facilitate that. The rendering
depends on the style of the layer along with pitch, bearing, and zoom of
the map. This was called *Data Driven Styling (DDS)*. Another option
that was used was to completely change the style in run time to achieve
the same outcome[^17].

The desire of being able to render a layer in different zoom levels
differently based on data drove the birth of *expressions*. To
summarize, expressions do the following:

1.  Expressions filter features from a source layer. This allows to
    apply conditional styling on the full or parts of the feature.

2.  Expressions apply rendering transformations to filtered features.
    MapLibre offers expressions that can interpolate, and paint. An
    expression can be applied to a feature constantly, zoom dependent,
    property dependent, or in zoom and property dependent manner.

Expressions use a JSON-like syntax to define. For brevity, let's look at
an example expression below:

```
'^': [
NumberType,
NumberType, NumberType],
(ctx, [b, e]) => Math.pow(b.evaluate(ctx), e.evaluate(ctx))
]
```

This defines an expression named `^` that returns a number
expression, and takes two number expressions as input. The
implementation follows right after. Another part to notice here is the
implementation evaluates both inputs because they are expressions too.
Expressions can be nested.

**Although it looks like JavaScript, for MapLibre Native GL**, **the
parser for any expression is written in MapLibre GL Native Core**. Each
platform such as iOS, Android has their own Expression class which
provides builders to build an expression and add it to a layer. When an
expression is added to a layer, the rendering part of the code picks it
up in the MapLibre Native GL Core.

Also, inside MapLibre GL Core, this definition mechanism allows
extending expressions library with custom expressions if desired.

## Expression Types

In the example expression, we saw how one expression is defined. The
example also shows that expressions have types. Expression language can
accept input and output types of null, number, string, boolean, color,
object, value, array, error, collator, and formatted.

Expressions are of different *kinds* such as assertion, coalesce,
interpolate, distance etc. Each *kind* of expression performs a single
responsibility.

*Assertion expressions* assert the returning type from one expression is
asserted before putting into another. For example, a filter expression
of `["get", "feature_property"]`, returns a generic *value* type. To
use it on another expression that accepts a string type, an assertion
such as `["string", ["get", "feature_property"]]` is necessary.
Assertion throws an evaluation-time error if the types don't match
during evaluation.

*Coercion expressions* convert a return type to another type. It also
allows to define a fallback if the conversion fails. A good example of
this is *to-number* expression. For example, `["to-number", ["get",
"feature_property"], 0]` means that we are trying to cover the
feature-property to a number. If it fails, we will use 0 as a fallback.

*Camera expressions* are expressions that allow style property
manipulation based on zoom, pitch, and distance from center.

Expressions are used *paint* property of a Mapbox Vector Style starting
with a *layer* selector. Expressions are evaluated by following an
expression chain constructed from the root expression following the
input expressions recursively.

## Implementation

Implementation wise, expressions are divided into builders and parsers.
Each platform such as Android and iOS have dedicated builder classes for
different types of expressions. An example of interpolate expression
from Android will look like:

```
fillLayer.setProperties(
    fillColor(
        interpolate(
            exponential(0.5f), zoom(),
            stop(1.0f, color(Color.RED)),
            stop(5.0f, color(Color.BLUE)),
            stop(10.0f, color(Color.GREEN))
        )
    )
);
```

**To render the built expression, MapLibre GL Native uses expression
parsers.** **Expression parsers are written in MapLibre GL Native Core
(in C++).** Each expression outputs an *EvaluationResult* class.
Resolving an *EvaluationResult* can be deferred. As in, the result of an
expression can be computed only when its necessary to be computed in
runtime. A change induced by data or interaction in an expression
evaluation result will result in a new style load in the rendering loop.

___________________

[^17]: Sourced from Mapbox GL Native wiki:
    <https://github.com/mapbox/mapbox-gl-native/wiki/Expression-Architecture>
