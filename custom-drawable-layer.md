## Custom Drawable Layer vs Annotations

### Annotations

#### Interface

The current Annotations interface is exposed by the _core_ through `Map`:
```C++
    // Annotations
    void addAnnotationImage(std::unique_ptr<style::Image>);
    void removeAnnotationImage(const std::string&);
    double getTopOffsetPixelsForAnnotationImage(const std::string&);

    AnnotationID addAnnotation(const Annotation&);
    void updateAnnotation(AnnotationID, const Annotation&);
    void removeAnnotation(AnnotationID);
```

#### Implementation

[Source folder](https://github.com/maplibre/maplibre-native/blob/main/src/mbgl/annotation/)

`AnnotationManager` [annotation_manager.hpp](https://github.com/maplibre/maplibre-native/blob/main/src/mbgl/annotation/annotation_manager.hpp)
```C++
class AnnotationManager ... {
public:
...
    AnnotationID addAnnotation(const Annotation&);
    bool updateAnnotation(const AnnotationID&, const Annotation&);
    void removeAnnotation(const AnnotationID&);

    void addImage(std::unique_ptr<style::Image>);
    void removeImage(const std::string&);
    double getTopOffsetPixelsForImage(const std::string&);

    void setStyle(style::Style&);
    void onStyleLoaded();

    void updateData();

    void addTile(AnnotationTile&);
    void removeTile(AnnotationTile&);

    static const std::string SourceID;
    static const std::string PointLayerID;
    static const std::string ShapeLayerID;

...
private:
    void add(const AnnotationID&, const SymbolAnnotation&);
    void add(const AnnotationID&, const LineAnnotation&);
    void add(const AnnotationID&, const FillAnnotation&);

    void update(const AnnotationID&, const SymbolAnnotation&);
    void update(const AnnotationID&, const LineAnnotation&);
    void update(const AnnotationID&, const FillAnnotation&);

    void remove(const AnnotationID&);

    void updateStyle();

    std::unique_ptr<AnnotationTileData> getTileData(const CanonicalTileID&);

    std::reference_wrapper<style::Style> style;

    std::mutex mutex;

    bool dirty = false;

    AnnotationID nextID = 0;

    using SymbolAnnotationTree = boost::geometry::index::rtree<std::shared_ptr<const SymbolAnnotationImpl>,
                                                               boost::geometry::index::rstar<16, 4>>;
    // Unlike std::unordered_map, std::map is guaranteed to sort by
    // AnnotationID, ensuring that older annotations are below newer
    // annotations. <https://github.com/mapbox/mapbox-gl-native/issues/5691>
    using SymbolAnnotationMap = std::map<AnnotationID, std::shared_ptr<SymbolAnnotationImpl>>;
    using ShapeAnnotationMap = std::map<AnnotationID, std::unique_ptr<ShapeAnnotationImpl>>;
    using ImageMap = std::unordered_map<std::string, style::Image>;

    SymbolAnnotationTree symbolTree;
    SymbolAnnotationMap symbolAnnotations;
    ShapeAnnotationMap shapeAnnotations;
    ImageMap images;

    std::unordered_set<AnnotationTile*> tiles;
    mapbox::base::WeakPtrFactory<AnnotationManager> weakFactory{this};
};
```

`LineAnnotation` `FillAnnotation` `SymbolAnnotation` [annotation.hpp](https://github.com/maplibre/maplibre-native/blob/main/include/mbgl/annotation/annotation.hpp)

`RenderOrchestrator::createRenderTree` calls `AnnotationManager::updateData()`

`AnnotationSource`

`AnnotationTile`



RenderLayer

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

Add Annotation Shapes

```ObjectiveC
- (void)addTestShapes:(NSUInteger)featuresCount
{
    for (int featureIndex = 0; featureIndex < featuresCount; ++featureIndex) {
        double deltaLongitude = featureIndex * 0.01;
        double deltaLatitude = -featureIndex * 0.01;
        
        // Pacific Northwest triangle
        //
        CLLocationCoordinate2D triangleCoordinates[3] =
        {
            CLLocationCoordinate2DMake(44 + deltaLatitude, -122 + deltaLongitude),
            CLLocationCoordinate2DMake(46 + deltaLatitude, -122 + deltaLongitude),
            CLLocationCoordinate2DMake(46 + deltaLatitude, -121 + deltaLongitude)
        };
        
        MLNPolygon *triangle = [MLNPolygon polygonWithCoordinates:triangleCoordinates count:3];
        
        [self.mapView addAnnotation:triangle];
        
        // West coast polyline
        //
        CLLocationCoordinate2D lineCoordinates[4] = {
            CLLocationCoordinate2DMake(47.6025 + deltaLatitude, -122.3327 + deltaLongitude),
            CLLocationCoordinate2DMake(45.5189 + deltaLatitude, -122.6726 + deltaLongitude),
            CLLocationCoordinate2DMake(37.7790 + deltaLatitude, -122.4177 + deltaLongitude),
            CLLocationCoordinate2DMake(34.0532 + deltaLatitude, -118.2349 + deltaLongitude)
        };
        MLNPolyline *line = [MLNPolyline polylineWithCoordinates:lineCoordinates count:4];
        [self.mapView addAnnotation:line];
        
        // Orcas Island, WA hike polyline
        //
        NSDictionary *hike = [NSJSONSerialization JSONObjectWithData:
                              [NSData dataWithContentsOfFile:
                               [[NSBundle mainBundle] pathForResource:@"polyline" ofType:@"geojson"]]
                                                             options:0
                                                               error:nil];
        
        NSArray *hikeCoordinatePairs = hike[@"features"][0][@"geometry"][@"coordinates"];
        
        CLLocationCoordinate2D *polylineCoordinates = (CLLocationCoordinate2D *)malloc([hikeCoordinatePairs count] * sizeof(CLLocationCoordinate2D));
        
        for (NSUInteger i = 0; i < [hikeCoordinatePairs count]; i++)
        {
            polylineCoordinates[i] = CLLocationCoordinate2DMake([hikeCoordinatePairs[i][1] doubleValue] + deltaLatitude, [hikeCoordinatePairs[i][0] doubleValue] + deltaLongitude);
        }
        
        MLNPolyline *polyline = [MLNPolyline polylineWithCoordinates:polylineCoordinates
                                                               count:[hikeCoordinatePairs count]];
        
        [self.mapView addAnnotation:polyline];
        
        free(polylineCoordinates);
        
        // PA/NJ/DE polygons
        //
        NSDictionary *threestates = [NSJSONSerialization JSONObjectWithData:
                                     [NSData dataWithContentsOfFile:
                                      [[NSBundle mainBundle] pathForResource:@"threestates" ofType:@"geojson"]]
                                                                    options:0
                                                                      error:nil];
        
        for (NSDictionary *feature in threestates[@"features"])
        {
            NSArray *stateCoordinatePairs = feature[@"geometry"][@"coordinates"];
            
            while ([stateCoordinatePairs count] == 1) stateCoordinatePairs = stateCoordinatePairs[0];
            
            CLLocationCoordinate2D *polygonCoordinates = (CLLocationCoordinate2D *)malloc([stateCoordinatePairs count] * sizeof(CLLocationCoordinate2D));
            
            for (NSUInteger i = 0; i < [stateCoordinatePairs count]; i++)
            {
                polygonCoordinates[i] = CLLocationCoordinate2DMake([stateCoordinatePairs[i][1] doubleValue] + deltaLatitude, [stateCoordinatePairs[i][0] doubleValue] + deltaLongitude);
            }
            
            MLNPolygon *polygon = [MLNPolygon polygonWithCoordinates:polygonCoordinates count:[stateCoordinatePairs count]];
            polygon.title = feature[@"properties"][@"NAME"];
            
            [self.mapView addAnnotation:polygon];
            
            free(polygonCoordinates);
        }
        
        // Null Island polygon with an interior hole
        //
        CLLocationCoordinate2D innerCoordinates[] = {
            CLLocationCoordinate2DMake(-5 + deltaLatitude, -5 + deltaLongitude),
            CLLocationCoordinate2DMake(-5 + deltaLatitude, 5 + deltaLongitude),
            CLLocationCoordinate2DMake(5 + deltaLatitude, 5 + deltaLongitude),
            CLLocationCoordinate2DMake(5 + deltaLatitude, -5 + deltaLongitude),
        };
        MLNPolygon *innerPolygon = [MLNPolygon polygonWithCoordinates:innerCoordinates count:sizeof(innerCoordinates) / sizeof(innerCoordinates[0])];
        CLLocationCoordinate2D outerCoordinates[] = {
            CLLocationCoordinate2DMake(-10 + deltaLatitude, -10 + deltaLongitude),
            CLLocationCoordinate2DMake(-10 + deltaLatitude, 10 + deltaLongitude),
            CLLocationCoordinate2DMake(10 + deltaLatitude, 10 + deltaLongitude),
            CLLocationCoordinate2DMake(10 + deltaLatitude, -10 + deltaLongitude),
        };
        MLNPolygon *outerPolygon = [MLNPolygon polygonWithCoordinates:outerCoordinates count:sizeof(outerCoordinates) / sizeof(outerCoordinates[0]) interiorPolygons:@[innerPolygon]];
        [self.mapView addAnnotation:outerPolygon];
    }
}
```

Add Annotation with custom callout

```ObjectiveC
- (void)addAnnotationWithCustomCallout
{
    [self.mapView removeAnnotations:self.mapView.annotations];

    MBXCustomCalloutAnnotation *firstAnnotation = [[MBXCustomCalloutAnnotation alloc] init];
    firstAnnotation.coordinate = CLLocationCoordinate2DMake(48.8533940, 2.3775439);
    firstAnnotation.title = @"Open anchored to annotation";
    firstAnnotation.anchoredToAnnotation = YES;
    firstAnnotation.dismissesAutomatically = NO;

    MBXCustomCalloutAnnotation *secondAnnotation = [[MBXCustomCalloutAnnotation alloc] init];
    secondAnnotation.coordinate = CLLocationCoordinate2DMake(48.8543940, 2.3775439);
    secondAnnotation.title = @"Open not anchored to annotation";
    secondAnnotation.anchoredToAnnotation = NO;
    secondAnnotation.dismissesAutomatically = NO;

    MBXCustomCalloutAnnotation *thirdAnnotation = [[MBXCustomCalloutAnnotation alloc] init];
    thirdAnnotation.coordinate = CLLocationCoordinate2DMake(48.8553940, 2.3775439);
    thirdAnnotation.title = @"Dismisses automatically";
    thirdAnnotation.anchoredToAnnotation = YES;
    thirdAnnotation.dismissesAutomatically = YES;

    NSArray *annotations = @[firstAnnotation, secondAnnotation, thirdAnnotation];
    [self.mapView addAnnotations:annotations];

    [self.mapView showAnnotations:annotations animated:YES];
}
```

Annotation image callback ObjectiveC

```ObjectiveC
- (MLNAnnotationImage *)mapView:(MLNMapView * __nonnull)mapView imageForAnnotation:(id <MLNAnnotation> __nonnull)annotation
{
    if ([annotation isKindOfClass:[MBXDroppedPinAnnotation class]] || [annotation isKindOfClass:[MBXCustomCalloutAnnotation class]])
    {
        return nil; // use default marker
    }

    NSAssert([annotation isKindOfClass:[MBXSpriteBackedAnnotation class]], @"Annotations should be sprite-backed.");

    NSString *title = [(MLNPointAnnotation *)annotation title];
    if (!title.length) return nil;
    NSString *lastTwoCharacters = [title substringFromIndex:title.length - 2];

    MLNAnnotationImage *annotationImage = [mapView dequeueReusableAnnotationImageWithIdentifier:lastTwoCharacters];

    if ( ! annotationImage)
    {
        UIColor *color;

        // make every tenth annotation blue
        if ([lastTwoCharacters hasSuffix:@"0"]) {
            color = [UIColor blueColor];
        } else {
            color = [UIColor redColor];
        }

        UIImage *image = [self imageWithText:lastTwoCharacters backgroundColor:color];
        annotationImage = [MLNAnnotationImage annotationImageWithImage:image reuseIdentifier:lastTwoCharacters];

        // don't allow touches on blue annotations
        if ([color isEqual:[UIColor blueColor]]) annotationImage.enabled = NO;
    }

    return annotationImage;
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
