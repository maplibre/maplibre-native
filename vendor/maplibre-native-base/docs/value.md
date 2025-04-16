

# mapbox::base values

*Value* offers a fast, flexible and uncomplicated way to build easy-to-use APIs where strong types are not required.

A mapbox::base::Value holds a value of a type. Supported types are `int`, `uint`, `bool`, `double`, `array`, `object` and `string`. The type and the value of a mapbox::base::Value [can change](#Type).

Values are used widely in Mapbox libraries, for instance when working with geometries or data in JSON format. They can be nested to create complex structures.

- [mapbox::base::Value](#Value)
- [mapbox::base::ValueArray](#ValueArray)
- [mapbox::base::ValueObject](#ValueObject)
- [Introspect Values](#Introspect-Values)
- [Example](#A-real-world-example)

## Value

```C++
    mapbox::base::Value num(-20);
    mapbox::base::Value pi(3.14159265359);
    mapbox::base::Value name("John Smith");
    mapbox::base::Value codingIsFun(true);
```

### get

Use `get<T>()` to get a copy.

```C++
double value = pi.get<double>();
```

It throws an exception if the type doesn't match the current type.

```C++
    double value;
    try {
        value = pi.get<double>();
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    std::cout << value << std::endl;
```

If you don't want exceptions, use `get_unchecked<T>()`:

```C++
    pi.get_unchecked<std::string>();
```
<!--
or
```C++
    double value2 = *pi.getDouble();
    std::cout << value << std::endl;
```
//-->

Get a pointer:
```C++
    double* value = pi.getDouble();
    std::cout << *value << std::endl;
```

outputs 3.14159265359

### set, update

```C++
    pi = 0.456;
    pi.set<double>(0.123);

    std::cout << *pi.getDouble() << std::endl;
```

outputs 0.123

### Type

Assigning a value of an allowed type changes the type of the [mapbox::base::Value](#Value).

You can use `which()` to get the type index.

```C++
    mapbox::base::Value val;

    val = "a string";
    std::cout << val.which() << std::endl;

    val = 0.5;
    std::cout << val.which() << std::endl;
```

output:
```
5
4
```

To ask for a specific type, use `is<T>()`:

```C++
std::cout << "is double: " << val.is<double>() << ", is string: " << val.is<std::string>() << std::endl;
```

```
is double: 1, is string: 0
```

Use `valid()` to see if the type index is within the allowed range.

## ValueArray

The ValueArray can hold an arbitrary number of [values](#Value) and is based on a [std::vector](https://en.cppreference.com/w/cpp/container/vector). You can use all its methods to work with the array.

```C++
    mapbox::base::ValueArray array = {num, pi, name, codingIsFun};
```

or more programatically

```C++
    array.push_back(num);
    array.push_back(pi);
    array.push_back(name);
    array.push_back(codingIsFun);
```

Our array is: [-20,3.14159265359,"John Smith",true]

### get an item

```C++
    mapbox::base::Value myValue = array[1];
    double piV = myValue.get<double>();

    std::cout << piV << std::endl;>>
```

### and insert

```C++
    auto it = array.begin() + 2;
    array.insert(it, "inserting a string");
```
array is now [-20,3.14159265359,"inserting a string","John Smith",true]

### update an item

```C++
    array[1].set<double>(0.45);
```
array is now [-20,0.45,"inserting a string","John Smith",true]. Here we can also [change](#Type) the type and assign a value of a different type.

## ValueObject

The ValueObject holds key-value pairs and can be used to model structures similar to json objects.
It is based on an [std::unordered_map](https://de.cppreference.com/w/cpp/container/unordered_map)<std::string, [value](#value)>. You can use their methods to work with ValueObject.

```C++
    mapbox::base::ValueObject object;
    object.insert({"text-color", "yellow"});
```

### get an item:

```C++
    mapbox::base::Value v = object["text-color"];

    std::string color = *object["text-color"].getString();
    std::cout << color << std::endl;
```

outputs yellow;

### update the value of an item:

```C++
    object["text-color"].set<std::string>("green");

    std::cout << object["text-color"].get<std::string>() << std::endl;
```

outputs green.

## Introspect Values

Lets assume you get a value from a component and want to peek inside to see whats 'in the box'.

We don't know beforehand how deep the value is nested. Therefor we want to recursively visit the values, and we can use `match()` for the types:

```C++
    std::function<void(const mapbox::base::Value&)> visit = [&](const mapbox::base::Value& value) {

        value.match([&](const mapbox::base::NullValue) {},
                    [&](const std::string& str) {},
                    [&](bool b) {},
                    [&](uint64_t u64) {},
                    [&](int64_t i64) {},
                    [&](double f64) {},
                    [&](const mapbox::base::ValueArray& array) {
                        for (const auto& v : array) {
                            visit(v);
                        }
                    },
                    [&](const mapbox::base::ValueObject& object) {
                        for (const auto& kv : object) {
                            visit(kv.second)
                        }
                    });

    };

    visit(value);
```

And now with a bit of extra code to output the value in a json-like fashion:

```C++
std::string toString(const mapbox::base::Value& value) {
    std::stringstream buffer;

    std::function<void(const mapbox::base::Value&, int)> visit = [&](const mapbox::base::Value& value, int depth) {
        std::string indent = "";
        for(int i = 0; i < depth; ++i) {
            indent += "  ";
        }

        value.match([&](const mapbox::base::NullValue)  { buffer << indent << "nullptr"; },
                    [&](const std::string& str)         { buffer << "\"" << str << "\""; },
                    [&](bool b)                         { buffer << b; },
                    [&](uint64_t u64)                   { buffer << indent << u64; },
                    [&](int64_t i64)                    { buffer << i64; },
                    [&](double f64)                     { buffer << f64; },
                    [&](const mapbox::base::ValueArray& array) {
                        buffer << indent << "[" << std::endl;
                        for (const auto& v : array) {
                            buffer << indent << "  ";
                            visit(v, depth+1);
                            buffer << "," << std::endl;
                        }
                        if (array.size() > 1) buffer.seekp(-2, std::ios_base::end);
                        buffer << std::endl << indent << "]" << std::endl;
                    },
                    [&](const mapbox::base::ValueObject& object) {
                        buffer << indent << "{" << std::endl;
                        for (const auto& kv : object) {
                            buffer << indent << "  " << kv.first << ": ";
                            visit(kv.second, depth+1);
                            buffer << "," << std::endl;
                        }
                        if (object.size() > 1) buffer.seekp(-2, std::ios_base::end);
                        buffer << std::endl << indent << "}"  << std::endl;
                    });
    };

    visit(value, 0);

    return buffer.str();
}

//! lets add our array to our object to create a nested structure:
object.insert({"list", array});

std::cout << toString(object) << std::endl;
```

outputs

```JSON
{
  text-color: "green",
  list:   [
    -20,
    0.45,
    "inserting a string",
    "John Smith",
    1
  ]
}
```

## A real world example

Let's assume you want to create a line gradient to be used in MapLibre Native. We can programatically construct the data structure as we would do it if the data comes from a backend data source:

```C++
    using A = mapbox::base::ValueArray;

    A lineGradient = {"interpolate", A{"linear"}, A{"line-progress"}};

    lineGradient.push_back(0);
    lineGradient.push_back("blue");

    lineGradient.push_back(0.1);
    lineGradient.push_back("royalblue");

    lineGradient.push_back(0.3);
    lineGradient.push_back("cyan");

    lineGradient.push_back(0.5);
    lineGradient.push_back("lime");

    lineGradient.push_back(0.7);
    lineGradient.push_back("yellow");

    lineGradient.push_back(1);
    lineGradient.push_back("red");

    //! which we can now give to the map:
    map->setStyleLayerProperty("my-line", "line-gradient", lineGradient);

    std::cout << toString(lineGradient) << std::endl;
```

results in:

```JSON
[
  "interpolate",
  ["linear"],
  ["line-progress"],
  0,
  "blue",
  0.1,
  "royalblue",
  0.3,
  "cyan",
  0.5,
  "lime",
  0.7,
  "yellow",
  1,
  "red"
]
```

See https://maplibre.org/maplibre-style-spec/layers/#line-gradient to learn more about line gradients.
