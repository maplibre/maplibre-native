## Custom Drawable Layer vs Annotations

### Annotations

#### Interface

The current Annotations interface is exposed by the `Map` object;
```C++
    // Annotations
    void addAnnotationImage(std::unique_ptr<style::Image>);
    void removeAnnotationImage(const std::string&);
    double getTopOffsetPixelsForAnnotationImage(const std::string&);

    AnnotationID addAnnotation(const Annotation&);
    void updateAnnotation(AnnotationID, const Annotation&);
    void removeAnnotation(AnnotationID);
```

#### Usage

Point Annotation with image in Swift

![image](https://github.com/maplibre/maplibre-native/assets/4198736/c6c3001f-a3d9-4427-8893-ac0aefe9e4b9)

```Swift
    func makeUIView(context: Context) -> MGLMapView {
        // create the mapview
        let mapView = MGLMapView(frame: .zero, styleURL: MGLStyle.defaultStyleURL())
        ...
        mapView.delegate = context.coordinator

        // annotations
        // Create point to represent where the symbol should be placed
        let point1 = MGLPointAnnotation()
        point1.coordinate = CLLocationCoordinate2D(latitude: 46.76952032174447, longitude: 23.589856130996207)
        point1.title = "Statue1"
        mapView.addAnnotation(point1)
        
        // 46.751701289159485, 23.5981339859047
        let point2 = MGLPointAnnotation()
        point2.coordinate = CLLocationCoordinate2D(latitude: 46.751701289159485, longitude: 23.5981339859047)
        point2.title = "Statue2"
        mapView.addAnnotation(point2)
        
        return mapView
    }
        
    final class Coordinator: NSObject, MGLMapViewDelegate {
        var control: MapView
        
        init(_ control: MapView) {
            self.control = control
        }

        ...
                
        func mapView(_ mapView: MGLMapView, imageFor annotation: MGLAnnotation) -> MGLAnnotationImage? {
            let image = UIImage(named: "myImage")!
            let annotationImage = MGLAnnotationImage(image: image, reuseIdentifier: "myImage")
            return annotationImage
        }
    }
```




### Custom Drawable Layer 

#### Interface

The _draft_ interface to build Custom Drawable Layers is offered through `CustomDrawableLayerHost::Interface`, and offers the following methods:
```C++
    /**
     * @brief Get the drawable count
     *
     * @return std::size_t
     */
    std::size_t getDrawableCount() const;

    /**
     * @brief Set the Tile ID
     *
     * @param tileID
     */
    void setTileID(OverscaledTileID tileID);

    /**
     * @brief Set the line options
     *
     * @param options
     */
    void setLineOptions(const LineOptions& options);

    /**
     * @brief Set the fill options
     *
     * @param options
     */
    void setFillOptions(const FillOptions& options);

    /**
     * @brief Set the Symbol options
     *
     * @param options
     */
    void setSymbolOptions(const SymbolOptions& options);

    /**
     * @brief Add a polyline
     *
     * @param coordinates
     * @param options Polyline options
     */
    void addPolyline(const GeometryCoordinates& coordinates);

    /**
     * @brief Add a multipolygon area fill
     *
     * @param geometry a collection of rings with optional holes
     */
    void addFill(const GeometryCollection& geometry);

    /**
     * @brief Add a symbol
     *
     * @param point
     */
    void addSymbol(const GeometryCoordinate& point);

    /**
     * @brief Finish the current drawable building session
     *
     */
    void finish();
```

Supporting objects:
```C++
    struct LineOptions {
        Color color;
        float blur = 0.f;
        float opacity = 1.f;
        float gapWidth = 0.f;
        float offset = 0.f;
        float width = 1.f;
        gfx::PolylineGeneratorOptions geometry;
    };

    struct FillOptions {
        Color color;
        float opacity = 1.f;
    };

    struct SymbolOptions {
        Color color;
        Size size;
        gfx::Texture2DPtr texture;
    };
```
#### Example usage 

Refer to the example in [ExampleCustomDrawableStyleLayer.mm](https://github.com/maplibre/maplibre-native/blob/main/platform/darwin/app/ExampleCustomDrawableStyleLayer.mm) 

```C++
class ExampleCustomDrawableStyleLayerHost : public mbgl::style::CustomDrawableLayerHost {
public:
    ExampleCustomDrawableStyleLayerHost(ExampleCustomDrawableStyleLayer *styleLayer) {
      // ...
    }
    
    void initialize() override {
      // ...
    }
    
    void update(Interface& interface) override {
        
        // if we have built our drawable(s) already, either update or skip
        if (interface.getDrawableCount())
            return;
        
        // setup view 
        interface.setTileID({11, 327, 791});

        // add polylines
        {
            Interface::LineOptions options { /*...*/ };            
            GeometryCoordinates polyline { /*...*/ };
                
            // set options
            interface.setLineOptions(options);
                
            // add polyline
            interface.addPolyline(polyline);
        }

        // add fill polygon
        {
            GeometryCollection geometry{
                {
                    // ring 1
                },
                // ...
                {
                    // ring N
                },
            };

            // set options
            interface.setFillOptions({/*color=*/, /*opacity=*/});

            // add fill
            interface.addFill(geometry);
        }
        
        // add symbol
        {
            // load image
            UIImage *assetImage = [UIImage imageNamed:@"Image"];
            assert(assetImage.CGImage != NULL);
            std::shared_ptr<PremultipliedImage> image = std::make_shared<PremultipliedImage>(MLNPremultipliedImageFromCGImage(assetImage.CGImage));
            
            // set options
            Interface::SymbolOptions options { /*...*/ };
            options.texture->setImage(image);
            interface.setSymbolOptions(options);
            
            // add symbol
            GeometryCoordinate position { /*...*/ };
            interface.addSymbol(position);
        }
                
        // finish
        interface.finish();
    }
    
    void deinitialize() override {
      // ...
    }
};
```
