# Predicates and expressions

Using `NSPredicate` with MapLibre iOS

Style layers use predicates and expressions to determine what to display and how
to format it. _Predicates_ are represented by the same `NSPredicate` class that
filters results from Core Data or items in an `NSArray` in Objective-C.
Predicates are based on _expressions_, represented by the `NSExpression` class.
Somewhat unusually, style layers also use expressions on their own.

This document discusses the specific subset of the predicate and expression
syntax supported by this SDK. For a more general introduction to predicates and
expressions, consult the
_[Predicate Programming Guide](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Predicates/AdditionalChapters/Introduction.html)_
in Apple developer documentation. For additional detail on how this SDK has
extended the `NSExpression` class, see the [`NSExpression+MLNAdditions.h`](https://github.com/maplibre/maplibre-native/blob/main/platform/darwin/src/NSExpression%2BMLNAdditions.h) header.

## Using predicates to filter vector data

Most style layer classes display `MLNFeature` objects that you can show or hide
based on the feature’s attributes. Use the `MLNVectorStyleLayer.predicate`
property to include only the features in the source layer that satisfy a
condition that you define.

### Operators

The following comparison operators are supported:

`NSPredicateOperatorType`                     | Format string syntax
----------------------------------------------|---------------------
`NSEqualToPredicateOperatorType`              | `key = value`<br />`key == value`
`NSGreaterThanOrEqualToPredicateOperatorType` | `key >= value`<br />`key => value`
`NSLessThanOrEqualToPredicateOperatorType`    | `key <= value`<br />`key =< value`
`NSGreaterThanPredicateOperatorType`          | `key > value`
`NSLessThanPredicateOperatorType`             | `key < value`
`NSNotEqualToPredicateOperatorType`           | `key != value`<br />`key <> value`
`NSBetweenPredicateOperatorType`              | `key BETWEEN { 32, 212 }`

To test whether a feature has or lacks a specific attribute, compare the
attribute to `NULL` or `NIL`. Predicates created using the
`+[NSPredicate predicateWithValue:]` method are also supported. String
operators and custom operators are not supported.

The following compound operators are supported:

`NSCompoundPredicateType` | Format string syntax
--------------------------|---------------------
`NSAndPredicateType`      | `predicate1 AND predicate2`<br />`predicate1 && predicate2`
`NSOrPredicateType`       | `predicate1 OR predicate2`<br /><code>predicate1 &vert;&vert; predicate2</code>
`NSNotPredicateType`      | `NOT predicate`<br />`!predicate`

The following aggregate operators are supported:

`NSPredicateOperatorType`         | Format string syntax
----------------------------------|---------------------
`NSInPredicateOperatorType`       | `key IN { 'iOS', 'macOS', 'tvOS', 'watchOS' }`
`NSContainsPredicateOperatorType` | `{ 'iOS', 'macOS', 'tvOS', 'watchOS' } CONTAINS key`

You can use the `IN` and `CONTAINS` operators to test whether a value appears in a collection, whether a string is a substring of a larger string, or whether the evaluated feature (`SELF`) lies within a given `MLNShape` or `MLNFeature`. For example, to show one delicious local chain of sandwich shops, but not similarly named steakhouses and pizzerias:

```objc
MLNPolygon *cincinnati = [MLNPolygon polygonWithCoordinates:cincinnatiCoordinates count:sizeof(cincinnatiCoordinates) / sizeof(cincinnatiCoordinates[0])];
deliLayer.predicate = [NSPredicate predicateWithFormat:@"class = 'food_and_drink' AND name CONTAINS 'Izzy' AND SELF IN %@", cincinnati];
```

```swift
let cincinnati = MLNPolygon(coordinates: &cincinnatiCoordinates, count: UInt(cincinnatiCoordinates.count))
deliLayer.predicate = NSPredicate(format: "class = 'food_and_drink' AND name CONTAINS 'Izzy' AND SELF IN %@", cincinnati)
```

The following combinations of comparison operators and modifiers are supported:

`NSComparisonPredicateModifier` | `NSPredicateOperatorType`           | Format string syntax
--------------------------------|-------------------------------------|---------------------
`NSAllPredicateModifier`        | `NSNotEqualToPredicateOperatorType` | `ALL haystack != needle`
`NSAnyPredicateModifier`        | `NSEqualToPredicateOperatorType`    | `ANY haystack = needle`<br />`SOME haystack = needle`

The following comparison predicate options are supported for comparison and
aggregate operators that are used in the predicate:

`NSComparisonPredicateOptions`          | Format string syntax
----------------------------------------|---------------------
`NSCaseInsensitivePredicateOption`      | `'QUEBEC' =[c] 'Quebec'`
`NSDiacriticInsensitivePredicateOption` | `'Québec' =[d] 'Quebec'`

Other comparison predicate options are unsupported, namely `l`
(for locale sensitivity) and `n` (for normalization). A comparison is
locale-sensitive as long as it is case- or diacritic-insensitive. Comparison
predicate options are not supported in conjunction with comparison modifiers
like `ALL` and `ANY`.

### Operands

Operands in predicates can be [variables](#variables), [key paths](#key-paths),
or almost anything else that can appear
[inside an expression](#using-expressions-to-configure-layout-and-paint-attributes).

Automatic type casting is not performed. Therefore, a feature only matches a
predicate if its value for the attribute in question is of the same type as the
value specified in the predicate. Use the `CAST()` operator to convert a key
path or variable into a matching type:

* To cast a value to a number, use `CAST(key, 'NSNumber')`.
* To cast a value to a string, use `CAST(key, 'NSString')`.
* To cast a value to a color, use `CAST(key, 'UIColor')` on iOS and `CAST(key, 'NSColor')` on macOS.
* To cast an `NSColor` or `UIColor` object to an array, use `CAST(noindex(color), 'NSArray')`.

For details about the predicate format string syntax, consult the “Predicate
Format String Syntax” chapter of the
_[Predicate Programming Guide](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Predicates/AdditionalChapters/Introduction.html)_
in Apple developer documentation.

## Using expressions to configure layout and paint attributes

An expression can contain subexpressions of various types. Each of the supported
types of expressions is discussed below.

### Constant values

A constant value can be of any of the following types:

In Objective-C        | In Swift
----------------------|---------
`NSColor` (macOS)<br>`UIColor` (iOS) | `NSColor` (macOS)<br>`UIColor` (iOS)
`NSString`            | `String`
`NSString`            | `String`
`NSNumber.boolValue`  | `NSNumber.boolValue`
`NSNumber.doubleValue` | `NSNumber.doubleValue`
`NSArray<NSNumber>` | `[Float]`
`NSArray<NSString>` | `[String]`
`NSValue.CGVectorValue` (iOS)<br>`NSValue` containing `CGVector` (macOS) | `NSValue.cgVectorValue` (iOS)<br>`NSValue` containing `CGVector` (macOS)
`NSValue.UIEdgeInsetsValue` (iOS)<br>`NSValue.edgeInsetsValue` (macOS) | `NSValue.uiEdgeInsetsValue` (iOS)<br>`NSValue.edgeInsetsValue` (macOS)

For literal floating-point values, use `-[NSNumber numberWithDouble:]` instead
of `-[NSNumber numberWithFloat:]` to avoid precision issues.

### Key paths

A key path expression refers to an attribute of the `MLNFeature` object being
evaluated for display. For example, if a polygon’s `MLNFeature.attributes`
dictionary contains the `floorCount` key, then the key path `floorCount` refers
to the value of the `floorCount` attribute when evaluating that particular
polygon.

The following special attributes are also available on features that are produced
as a result of clustering multiple point features together in a shape source:

| Attribute   | Type   | Meaning                                                                                                                                  |
|-------------|--------|------------------------------------------------------------------------------------------------------------------------------------------|
| cluster     | Bool   | True if the feature is a point cluster. If the attribute is false (or not present) then the  feature should not be considered a cluster. |
| cluster_id  | Number | Identifier for the point cluster.                                                                                                        |
| point_count | Number | The number of point features in a given cluster.                                                                                         |

Some characters may not be used directly as part of a key path in a format
string. For example, if a feature’s attribute is named `ISO 3166-1:2006`, an
expression format string of `lowercase(ISO 3166-1:2006)` or a predicate format
string of `ISO 3166-1:2006 == 'US-OH'` would raise an exception. Instead, use a
`%K` placeholder or the `+[NSExpression expressionForKeyPath:]` initializer:

```objc
[NSPredicate predicateWithFormat:@"%K == 'US-OH'", @"ISO 3166-1:2006"];
[NSExpression expressionForFunction:@"lowercase:"
                          arguments:@[[NSExpression expressionForKeyPath:@"ISO 3166-1:2006"]]]
```

```swift
NSPredicate(format: "%K == 'US-OH'", "ISO 3166-1:2006")
NSExpression(forFunction: "lowercase:",
             arguments: [NSExpression(forKeyPath: "ISO 3166-1:2006")])
```

### Functions

Of the
[functions predefined](https://developer.apple.com/documentation/foundation/nsexpression/1413747-init#discussion)
by the
[`+[NSExpression expressionForFunction:arguments:]` method](https://developer.apple.com/documentation/foundation/nsexpression/1413747-init),
the following subset is supported in layer attribute values:

Initializer parameter | Format string syntax
----------------------|---------------------
`average:`            | `average({1, 2, 2, 3, 4, 7, 9})`
`sum:`                | `sum({1, 2, 2, 3, 4, 7, 9})`
`count:`              | `count({1, 2, 2, 3, 4, 7, 9})`
`min:`                | `min({1, 2, 2, 3, 4, 7, 9})`
`max:`                | `max({1, 2, 2, 3, 4, 7, 9})`
`add:to:`             | `1 + 2`
`from:subtract:`      | `2 - 1`
`multiply:by:`        | `1 * 2`
`divide:by:`          | `1 / 2`
`modulus:by:`         | `modulus:by:(1, 2)`
`sqrt:`               | `sqrt(2)`
`log:`                | `log(10)`
`ln:`                 | `ln(2)`
`raise:toPower:`      | `2 ** 2`
`exp:`                | `exp(0)`
`ceiling:`            | `ceiling(0.99999)`
`abs:`                | `abs(-1)`
`trunc:`              | `trunc(6378.1370)`
`floor:`              | `floor(-0.99999)`
`uppercase:`          | `uppercase('Elysian Fields')`
`lowercase:`          | `lowercase('DOWNTOWN')`
`noindex:`            | `noindex(0 + 2 + c)`
`length:`             | `length('Wapakoneta')`
`castObject:toType:`  | `CAST(ele, 'NSString')`<br>`CAST(ele, 'NSNumber')`

A number of [MapLibre-specific functions](#MapLibre-specific-functions) are also
available.

The following predefined functions are **not** supported:

Initializer parameter | Format string syntax
----------------------|---------------------
`median:`             | `median({1, 2, 2, 3, 4, 7, 9})`
`mode:`               | `mode({1, 2, 2, 3, 4, 7, 9})`
`stddev:`             | `stddev({1, 2, 2, 3, 4, 7, 9})`
`random`              | `random()`
`randomn:`            | `randomn(10)`
`now`                 | `now()`
`bitwiseAnd:with:`    | `bitwiseAnd:with:(5, 3)`
`bitwiseOr:with:`     | `bitwiseOr:with:(5, 3)`
`bitwiseXor:with:`    | `bitwiseXor:with:(5, 3)`
`leftshift:by:`       | `leftshift:by:(23, 1)`
`rightshift:by:`      | `rightshift:by:(23, 1)`
`onesComplement:`     | `onesComplement(255)`
`distanceToLocation:fromLocation:` | `distanceToLocation:fromLocation:(there, here)`

### Conditionals

Conditionals are supported via the built-in
`+[NSExpression expressionForConditional:trueExpression:falseExpression:]`
method and `TERNARY()` operator. If you need to express multiple cases
(“else-if”), you can either nest a conditional within a conditional or use the
[`MLN_IF()`](#code-mgl_if-code) or [`MLN_MATCH()`](#code-mgl_match-code) function.

### Aggregates

Aggregate expressions can contain arrays of expressions. In some cases, it is
possible to use the array itself instead of wrapping the array in an aggregate
expression.

### Variables

The following variables are defined by this SDK for use with style layers:

| Variable | Type | Meaning |
| --- | --- | --- |
| `$featureIdentifier` | Any GeoJSON data type | A value that uniquely identifies the feature in the containing source. This variable corresponds to the `NSExpression.featureIdentifierVariableExpression` property. |
| `$geometryType` | String | The type of geometry represented by the feature. A feature’s type is one of the following strings:<br><br>*   `Point` for point features, corresponding to the `MLNPointAnnotation` class<br>*   `LineString` for polyline features, corresponding to the ``MLNPolyline`` class<br>*   `Polygon` for polygon features, corresponding to the ``MLNPolygon`` class<br><br>This variable corresponds to the `NSExpression.geometryTypeVariableExpression` property. |
| `$heatmapDensity` | Number | The [kernel density estimation](https://en.wikipedia.org/wiki/Kernel_density_estimation) of a screen point in a heatmap layer; in other words, a relative measure of how many data points are crowded around a particular pixel. This variable can only be used with the `heatmapColor` property. This variable corresponds to the `NSExpression.heatmapDensityVariableExpression` property. |
| `$zoomLevel` | Number | The current zoom level. In style layout and paint properties, this variable may only appear as the target of a top-level interpolation or step expression. This variable corresponds to the `NSExpression.zoomLevelVariableExpression` property. |
| `$lineProgress` | Number | A number that indicates the relative distance along a line at a given point along the line. This variable evaluates to 0 at the beginning of the line and 1 at the end of the line. It can only be used with the ``MLNLineStyleLayer/lineGradient`` property. It corresponds to the `NSExpression.lineProgressVariableExpression` property. |

In addition to these variables, you can define your own variables and refer to
them elsewhere in the expression. The syntax for defining a variable makes use
of a [MapLibre-specific function](#MapLibre-specific-functions) that takes an
`NSDictionary` as an argument:

```objc
[NSExpression expressionWithFormat:@"MLN_LET('floorCount', 2, $floorCount + 1)"];
```

```swift
NSExpression(format: "MLN_LET(floorCount, 2, $floorCount + 1)")
```

## MapLibre-specific functions

> Warning: Due to a change in iOS 15.5, some of these stopped working. See [#331](https://github.com/maplibre/maplibre-native/issues/331) for more information and workarounds.

For compatibility with the MapLibre Style Spec, the following functions
are defined by this SDK. When setting a style layer property, you can call these
functions just like the predefined functions above, using either the
`+[NSExpression expressionForFunction:arguments:]` method or a convenient format
string syntax:



### mgl_does:have:

**Selector:** `mgl_does:have:`

**Format string syntax:** `mgl_does:have:(SELF, '🧀🍔')` or `mgl_does:have:(%@, '🧀🍔')`

Returns a Boolean value indicating whether the dictionary has a value for the
key or whether the evaluated object (`SELF`) has a value for the feature
attribute. Compared to the [`mgl_has:`](#code-mgl_has-code) custom function,
that function's target is instead passed in as the first argument to this
function. Both functions are equivalent to the syntax `key != NIL` or
`%@[key] != NIL` but can be used outside of a predicate.

### mgl_interpolate:withCurveType:parameters:stops:

**Selector:** `mgl_interpolate:withCurveType:parameters:stops:`

**Format string syntax:** `mgl_interpolate:withCurveType:parameters:stops:(x, 'linear', nil, %@)`

Produces continuous, smooth results by interpolating between pairs of input and
output values ("stops"). Compared to the
[`mgl_interpolateWithCurveType:parameters:stops:`](#code-mgl_interpolatewithcurvetype-parameters-stops-code)
custom function, the input expression (that function's target) is instead passed
in as the first argument to this function.

### mgl_step:from:stops:

**Selector:** `mgl_step:from:stops:`

**Format string syntax:** `mgl_step:from:stops:(x, 11, %@)`

Produces discrete, stepped results by evaluating a piecewise-constant function
defined by pairs of input and output values ("stops"). Compared to the
[`mgl_stepWithMinimum:stops:`](#code-mgl_stepwithminimum-stops-code) custom
function, the input expression (that function's target) is instead passed in as
the first argument to this function.

### mgl_join:

**Selector:** `mgl_join:`

**Format string syntax:** `mgl_join({'Old', 'MacDonald'})`

Returns the result of concatenating together all the elements of an array in
order. Compared to the
[`stringByAppendingString:`](#code-stringbyappendingstring-code) custom
function, this function takes only one argument, which is an aggregate
expression containing the strings to concatenate.

### mgl_acos:

**Selector:** `mgl_acos:`

**Format string syntax:** `mgl_acos(1)`

Returns the arccosine of the number.

This function corresponds to the
[`acos`](https://maplibre.org/maplibre-style-spec/expressions/#acos)
operator in the MapLibre Style Spec.

### mgl_asin:

**Selector:** `mgl_asin:`

**Format string syntax:** `mgl_asin(0)`

Returns the arcsine of the number.

This function corresponds to the
[`asin`](https://maplibre.org/maplibre-style-spec/expressions/#asin)
operator in the MapLibre Style Spec.

### mgl_atan:

**Selector:** `mgl_atan:`

**Format string syntax:** `mgl_atan(20)`

Returns the arctangent of the number.

This function corresponds to the
[`atan`](https://maplibre.org/maplibre-style-spec/expressions/#atan)
operator in the MapLibre Style Spec.

### mgl_cos:

**Selector:** `mgl_cos:`

**Format string syntax:** `mgl_cos(0)`

Returns the cosine of the number.

This function corresponds to the
[`cos`](https://maplibre.org/maplibre-style-spec/expressions/#cos)
operator in the MapLibre Style Spec.

### mgl_log2:

**Selector:** `mgl_log2:`

**Format string syntax:** `mgl_log2(1024)`

Returns the base-2 logarithm of the number.

This function corresponds to the
[`log2`](https://maplibre.org/maplibre-style-spec/expressions/#log2)
operator in the MapLibre Style Spec.

### mgl_round:

**Selector:** `mgl_round:`

**Format string syntax:** `mgl_round(1.5)`

Returns the number rounded to the nearest integer. If the number is halfway
between two integers, this function rounds it away from zero.

This function corresponds to the
[`round`](https://maplibre.org/maplibre-style-spec/expressions/#round)
operator in the MapLibre Style Spec.

### mgl_sin:

**Selector:** `mgl_sin:`

**Format string syntax:** `mgl_sin(0)`

Returns the sine of the number.

This function corresponds to the
[`sin`](https://maplibre.org/maplibre-style-spec/expressions/#sin)
operator in the MapLibre Style Spec.

### mgl_tan:

**Selector:** `mgl_tan:`

**Format string syntax:** `mgl_tan(0)`

Returns the tangent of the number.

This function corresponds to the
[`tan`](https://maplibre.org/maplibre-style-spec/expressions/#tan)
operator in the MapLibre Style Spec.

### mgl_distanceFrom:

**Selector:** `mgl_distanceFrom:`

**Format string syntax:** `mgl_distanceFrom(%@)` with an `MLNShape`

Returns the straight-line distance from the evaluated object to the given shape.

This function corresponds to the
[`distance`](https://maplibre.org/maplibre-style-spec/expressions/#distance)
operator in the MapLibre Style Spec.

### mgl_coalesce:

**Selector:** `mgl_coalesce:`

**Format string syntax:** `mgl_coalesce({x, y, z})`

Returns the first non-`nil` value from an array of expressions.

This function corresponds to the
[`coalesce`](https://maplibre.org/maplibre-style-spec/expressions/#coalesce)
operator in the MapLibre Style Spec.

### mgl_attributed:

**Selector:** `mgl_attributed:`

**Format string syntax:** `mgl_attributed({x, y, z})`

Concatenates and returns the array of `MLNAttributedExpression` objects, for use
with the `MLNSymbolStyleLayer.text` property.

`MLNAttributedExpression.attributes` valid attributes.

 Key | Value Type
 --- | ---
 `MLNFontNamesAttribute` | An `NSExpression` evaluating to an `NSString` array.
 `MLNFontScaleAttribute` | An `NSExpression` evaluating to an `NSNumber` value.
 `MLNFontColorAttribute` | An `NSExpression` evaluating to an `UIColor` (iOS) or `NSColor` (macOS).

This function corresponds to the
[`format`](https://maplibre.org/maplibre-style-spec/expressions/#types-format)
operator in the MapLibre Style Spec.

### MLN_LET

**Selector:** `MLN_LET:`

**Format string syntax:** `MLN_LET('age', uppercase('old'), 'name', uppercase('MacDonald'), mgl_join({$age, $name}))`

**Arguments:** Any number of variable names interspersed with their assigned
`NSExpression` values, followed by an `NSExpression`
that may contain references to those variables.

Returns the result of evaluating an expression with the given variable values.
Compared to the
[`mgl_expressionWithContext:`](#code-mgl_expressionwithcontext-code) custom
function, this function takes the variable names and values inline before the
expression that contains references to those variables.

### MLN_MATCH

**Selector:** `MLN_MATCH:`

**Format string syntax:** `MLN_MATCH(x, 0, 'zero match', 1, 'one match', 2, 'two match', 'default')`

**Arguments:** An input expression, then any number of argument pairs, followed by a default
expression. Each argument pair consists of a constant value followed by an
expression to produce as a result of matching that constant value.
If the input value is an aggregate expression, then any of the constant values within
that aggregate expression result in the following argument. This is shorthand for
specifying an argument pair for each of the constant values within that aggregate
expression. It is not possible to match the aggregate expression itself.

Returns the result of matching the input expression against the given constant
values.

This function corresponds to the
`+[NSExpression(MLNAdditions) mgl_expressionForMatchingExpression:inDictionary:defaultExpression:]`
method and the
[`match`](https://maplibre.org/maplibre-style-spec/expressions/#match)
operator in the MapLibre Style Spec.

### MLN_IF

**Selector:** `MLN_IF:`

**Format string syntax:** `MLN_IF(1 = 2, YES, 2 = 2, YES, NO)`

**Arguments:** Alternating `NSPredicate` conditionals and resulting expressions,
followed by a default expression.

Returns the first expression that meets the condition; otherwise, the default
value. Unlike
`+[NSExpression expressionForConditional:trueExpression:falseExpression:]` or
the `TERNARY()` syntax, this function can accept multiple "if else" conditions
and is supported on iOS 8._x_ and macOS 10.10._x_; however, each conditional
passed into this function must be wrapped in a constant expression.

This function corresponds to the
`+[NSExpression(MLNAdditions) mgl_expressionForConditional:trueExpression:falseExpresssion:]`
method and the
[`case`](https://maplibre.org/maplibre-style-spec/expressions/#case)
operator in the MapLibre Style Spec.

### MLN_FUNCTION

**Selector:** `MLN_FUNCTION:`

**Format string syntax:** `MLN_FUNCTION('typeof', mystery)`

**Arguments:** Any arguments required by the expression operator.

An expression exactly as defined by the
[MapLibre Style Spec](https://maplibre.org/maplibre-style-spec/expressions/).

## Custom functions

The following custom functions are also available with the
`+[NSExpression expressionForFunction:selectorName:arguments:]` method or the
`FUNCTION()` format string syntax.

Some of these functions are defined as methods on their respective target
classes, but you should not call them directly outside the context of an
expression, because the result may differ from the evaluated expression's result
or may result in undefined behavior.

The MapLibre Style Spec defines some operators for which no custom
function is available. To use these operators in an `NSExpression`, call the
[`MLN_FUNCTION()`](#code-mgl_function-code) function with the same arguments
that the operator expects.

### boolValue

**Selector:** `boolValue`

**Format string syntax:** `FUNCTION(1, 'boolValue')`

**Target:** An `NSExpression` that evaluates to a number or string.

**Arguments:** None.

A Boolean representation of the target: `FALSE` when then input is an
empty string, 0, `FALSE`, `NIL`, or `NaN`, otherwise `TRUE`.

### mgl_has:

**Selector:** `mgl_has:`

**Format string syntax:** `FUNCTION($featureAttributes, 'mgl_has:', '🧀🍔')`

**Target:** An `NSExpression` that evaluates to an `NSDictionary`
or the evaluated object (`SELF`).

**Arguments:** An `NSExpression` that evaluates to an `NSString`
representing the key to look up in the dictionary or the feature attribute to
look up in the evaluated object (see `MLNFeature.attributes`).

`true` if the dictionary has a value for the key or if the evaluated
object has a value for the feature attribute.

This function corresponds to the
[`has`](https://maplibre.org/maplibre-style-spec/expressions/#has)
operator in the MapLibre Style Spec. See also the
[`mgl_does:have:`](#code-mgl_does-have-code) function, which is used on its own
without the `FUNCTION()` operator. You can also check whether an object has an
attribute by comparing the key path to `NIL`, for example `cheeseburger != NIL`
or `burger.cheese != NIL`

### mgl_expressionWithContext:

**Selector:** `mgl_expressionWithContext:`

**Format string syntax:** `FUNCTION($ios + $macos, 'mgl_expressionWithContext:', %@)` with
a dictionary containing `ios` and `macos` keys

**Target:** An `NSExpression` that may contain references to the variables
defined in the context dictionary.

**Arguments:** An `NSDictionary` with `NSString`s as keys and
`NSExpression`s as values. Each key is a variable name and each
value is the variable's value within the target expression.

The target expression with variable subexpressions replaced with the values
defined in the context dictionary.

This function corresponds to the
[`let`](https://maplibre.org/maplibre-style-spec/expressions/#let)
operator in the MapLibre Style Spec. See also the
[`MLN_LET`](#code-mgl_let-code) function, which is used on its own without the
`FUNCTION()` operator.

### mgl_interpolateWithCurveType:parameters:stops:

**Selector:** `mgl_interpolateWithCurveType:parameters:stops:`

**Format string syntax:** `FUNCTION($zoomLevel, 'mgl_interpolateWithCurveType:parameters:stops:', 'linear', NIL, %@)`
with a dictionary containing zoom levels or other constant values as keys

**Target:** An `NSExpression` that evaluates to a number and contains a
variable or key path expression.

**Arguments:** The first argument is one of the following strings denoting curve types:
`linear`, `exponential`, or `cubic-bezier`.

The second argument is an expression providing parameters for the curve:

* If the curve type is `linear`, the argument is `NIL`.
* If the curve type is `exponential`, the argument is an
  expression that evaluates to a number, specifying the base of the
  exponential interpolation.
* If the curve type is `cubic-bezier`, the argument is an
  array or aggregate expression containing four expressions, each
  evaluating to a number. The four numbers are control points for the
  cubic Bézier curve.

The third argument is an `NSDictionary` object representing the
interpolation's stops, with numeric zoom levels as keys and expressions as
values.

A value interpolated along the continuous mathematical function defined by the
arguments, with the target as the input to the function.

The input expression is matched against the keys in the stop dictionary. The
keys may be feature attribute values, zoom levels, or heatmap densities. The
values may be constant values or `NSExpression` objects. For example, you can
use a stop dictionary with the zoom levels 0, 10, and 20 as keys and the colors
yellow, orange, and red as the values.

This function corresponds to the
`+[NSExpression(MLNAdditions) mgl_expressionForInterpolatingExpression:withCurveType:parameters:stops:]`
method and the
[`interpolate`](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
operator in the MapLibre Style Spec. See also the
[`mgl_interpolate:withCurveType:parameters:stops:`](#code-mgl_interpolate-withcurvetype-parameters-stops-code)
function, which is used on its own without the `FUNCTION()` operator.

### mgl_numberWithFallbackValues:

**Selector:** `mgl_numberWithFallbackValues:`,
`doubleValue`,
`floatValue`, or
`decimalValue`

**Format string syntax:** `FUNCTION(ele, 'mgl_numberWithFallbackValues:', 0)`

**Target:** An `NSExpression` that evaluates to a Boolean value, number, or
string.

**Arguments:** Zero or more `NSExpression`s, each evaluating to a Boolean value
or string.

A numeric representation of the target:

* If the target is `NIL` or `FALSE`, the result is 0.
* If the target is true, the result is 1.
* If the target is a string, it is converted to a number as specified by the

**Selector:**
`mgl_numberWithFallbackValues:`,
`doubleValue`,
`floatValue`, or
`decimalValue`

**Format string syntax:**
`FUNCTION(ele, 'mgl_numberWithFallbackValues:', 0)`

**Target:**
An `NSExpression` that evaluates to a Boolean value, number, or
string.

**Arguments:**
Zero or more `NSExpression`s, each evaluating to a Boolean value
or string.

A numeric representation of the target:

* If the target is `NIL` or `FALSE`, the result is 0.
* If the target is true, the result is 1.
  * If the target is a string, it is converted to a number as specified by the
    "[ToNumber Applied to the String Type](https://tc39.github.io/ecma262/#sec-tonumber-applied-to-the-string-type)"
    algorithm of the ECMAScript Language Specification.
  * If multiple values are provided, each one is evaluated in order until the
    first successful conversion is obtained.

This function corresponds to the
[`to-number`](https://maplibre.org/maplibre-style-spec/expressions/#types-to-number)
operator in the MapLibre Style Spec. You can also cast a value to a
number by passing the value and the string `NSNumber` into the `CAST()`
operator.

### mgl_stepWithMinimum:stops:

**Selector:**
`mgl_stepWithMinimum:stops:`

**Format string syntax:**
`FUNCTION($zoomLevel, 'mgl_stepWithMinimum:stops:', 0, %@)` with
a dictionary with zoom levels or other constant values as keys

**Target:**
An `NSExpression` that evaluates to a number and contains a
variable or key path expression.

**Arguments:**
The first argument is an expression that evaluates to a number, specifying
the minimum value in case the target is less than any of the stops in the
second argument.

The second argument is an `NSDictionary` object representing the
interpolation's stops, with numeric zoom levels as keys and expressions as
values.

The output value of the stop whose key is just less than the evaluated target,
or the minimum value if the target is less than the least of the stops' keys.

The input expression is matched against the keys in the stop dictionary. The
keys may be feature attribute values, zoom levels, or heatmap densities. The
values may be constant values or `NSExpression` objects. For example, you can
use a stop dictionary with the zoom levels 0, 10, and 20 as keys and the colors
yellow, orange, and red as the values.

This function corresponds to the
`+[NSExpression(MLNAdditions) mgl_expressionForSteppingExpression:fromExpression:stops:]`
method and the
[`step`](https://maplibre.org/maplibre-style-spec/expressions/#step)
operator in the MapLibre Style Spec.

### stringByAppendingString:

**Selector:**
`stringByAppendingString:`

**Format string syntax:**
`FUNCTION('Old', 'stringByAppendingString:', 'MacDonald')`

**Target:**
An `NSExpression` that evaluates to a string.

**Arguments:**
One or more `NSExpression`s, each evaluating to a string.

The target string with each of the argument strings appended in order.

This function corresponds to the
`-[NSExpression(MLNAdditions) mgl_expressionByAppendingExpression:]`
method and is similar to the
[`concat`](https://maplibre.org/maplibre-style-spec/expressions/#concat)
operator in the MapLibre Style Spec. See also the
[`mgl_join:`](#code-mgl_join-code) function, which concatenates multiple
expressions and is used on its own without the `FUNCTION()` operator.

### stringValue

**Selector:**
`stringValue`

**Format string syntax:**
`FUNCTION(ele, 'stringValue')`

**Target:**
An `NSExpression` that evaluates to a Boolean value, number, or
string.

**Arguments:**
None.

A string representation of the target:

* If the target is <code>NIL</code>, the result is the empty string.
* If the target is a Boolean value, the result is the string `true` or `false`.
* If the target is a number, it is converted to a string as specified by the
  “[NumberToString](https://tc39.github.io/ecma262/#sec-tostring-applied-to-the-number-type)”
  algorithm of the ECMAScript Language Specification.
* If the target is a color, it is converted to a string of the form
  `rgba(r,g,b,a)`, where <var>r</var>, <var>g</var>, and <var>b</var> are
  numerals ranging from 0 to 255 and <var>a</var> ranges from 0 to 1.
* Otherwise, the target is converted to a string in the format specified by the
  [`JSON.stringify()`](https://tc39.github.io/ecma262/#sec-json.stringify)
  function of the ECMAScript Language Specification.

This function corresponds to the
[`to-string`](https://maplibre.org/maplibre-style-spec/expressions/#types-to-string)
operator in the MapLibre Style Spec. You can also cast a value to a
string by passing the value and the string `NSString` into the `CAST()`
operator.
