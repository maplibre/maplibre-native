# Fill Extrusion Ambient Occlusion

Darken the bottom of the walls of a fill extrusion layer and the ground around it, and adjust it with sliders.

> Note: This example uses UIKit.

This example loads a style with 3D buildings and sets ``MLNFillExtrusionStyleLayer/fillExtrusionAmbientOcclusionWallIntensity``, ``MLNFillExtrusionStyleLayer/fillExtrusionAmbientOcclusionWallRadius``, ``MLNFillExtrusionStyleLayer/fillExtrusionAmbientOcclusionGroundIntensity`` and ``MLNFillExtrusionStyleLayer/fillExtrusionAmbientOcclusionGroundRadius`` on every ``MLNFillExtrusionStyleLayer`` in the style.

The intensity controls how much of the ambient light the walls lose where they meet the ground, and the radius is the height in meters over which the effect fades out. Walls facing away from the light get darker than sunlit ones. The ground intensity and radius do the same for the ground around the footprints, which needs an opaque background layer. A switch turns the effect off by setting both intensities to 0.

<!-- include-example(AmbientOcclusionExample) -->

```swift
/// Fill-extrusion ambient occlusion: darkens the base of the walls and the ground around the
/// buildings. Every fill-extrusion layer in the style gets the values of the sliders.
class AmbientOcclusionExample: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    private struct Control {
        let title: String
        let range: ClosedRange<Float>
        let initial: Float
        let format: String
        /// Set to 0 while ambient occlusion is switched off
        let isIntensity: Bool
        let apply: (MLNFillExtrusionStyleLayer, NSExpression) -> Void
    }

    private let controls: [Control] = [
        Control(title: "Wall intensity", range: 0 ... 1, initial: 0.6, format: "%.2f", isIntensity: true) {
            $0.fillExtrusionAmbientOcclusionWallIntensity = $1
        },
        Control(title: "Wall radius", range: 0 ... 20, initial: 4, format: "%.1f m", isIntensity: false) {
            $0.fillExtrusionAmbientOcclusionWallRadius = $1
        },
        Control(title: "Ground intensity", range: 0 ... 1, initial: 0.6, format: "%.2f", isIntensity: true) {
            $0.fillExtrusionAmbientOcclusionGroundIntensity = $1
        },
        Control(title: "Ground radius", range: 0 ... 30, initial: 8, format: "%.1f m", isIntensity: false) {
            $0.fillExtrusionAmbientOcclusionGroundRadius = $1
        },
    ]

    private var sliders: [UISlider] = []
    private var valueLabels: [UILabel] = []
    private let enabledSwitch = UISwitch()
    private let panel = UIVisualEffectView(effect: UIBlurEffect(style: .systemMaterial))

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds, styleURL: OPENFREEMAP_LIBERTY_STYLE)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        mapView.delegate = self

        // Midtown Manhattan, New York, NY
        mapView.camera = MLNMapCamera(lookingAtCenter: CLLocationCoordinate2D(latitude: 40.7549, longitude: -73.9840),
                                      altitude: 900, pitch: 60, heading: 30)

        view.addSubview(mapView)

        addControls()
    }

    func mapView(_: MLNMapView, didFinishLoading style: MLNStyle) {
        if fillExtrusionLayers(in: style).isEmpty {
            addFillExtrusionLayer(style: style)
        }
        applyAll()
    }

    private func fillExtrusionLayers(in style: MLNStyle) -> [MLNFillExtrusionStyleLayer] {
        style.layers.compactMap { $0 as? MLNFillExtrusionStyleLayer }
    }

    /// Used only if the style has no 3D buildings of its own
    private func addFillExtrusionLayer(style: MLNStyle) {
        guard let source = style.source(withIdentifier: "openmaptiles") else {
            print("Could not find source openmaptiles")
            return
        }
        let layer = MLNFillExtrusionStyleLayer(identifier: "ambient-occlusion-buildings", source: source)
        layer.sourceLayerIdentifier = "building"
        layer.fillExtrusionBase = NSExpression(forKeyPath: "render_min_height")
        layer.fillExtrusionHeight = NSExpression(forKeyPath: "render_height")
        layer.fillExtrusionColor = NSExpression(forConstantValue: UIColor(white: 0.92, alpha: 1))
        style.addLayer(layer)
    }

    // MARK: - Controls

    private func addControls() {
        let stack = UIStackView()
        stack.axis = .vertical
        stack.spacing = 6
        stack.translatesAutoresizingMaskIntoConstraints = false

        let header = UIStackView()
        header.axis = .horizontal
        let headerLabel = UILabel()
        headerLabel.text = "Ambient occlusion"
        headerLabel.font = .preferredFont(forTextStyle: .headline)
        enabledSwitch.isOn = true
        enabledSwitch.addTarget(self, action: #selector(controlChanged), for: .valueChanged)
        header.addArrangedSubview(headerLabel)
        header.addArrangedSubview(UIView())
        header.addArrangedSubview(enabledSwitch)
        stack.addArrangedSubview(header)

        for (index, control) in controls.enumerated() {
            let titleLabel = UILabel()
            titleLabel.text = control.title
            titleLabel.font = .preferredFont(forTextStyle: .footnote)
            titleLabel.widthAnchor.constraint(equalToConstant: 110).isActive = true

            let valueLabel = UILabel()
            valueLabel.font = .monospacedDigitSystemFont(ofSize: 13, weight: .regular)
            valueLabel.textAlignment = .right
            valueLabel.widthAnchor.constraint(equalToConstant: 56).isActive = true

            let slider = UISlider()
            slider.minimumValue = control.range.lowerBound
            slider.maximumValue = control.range.upperBound
            slider.value = control.initial
            slider.isContinuous = true
            slider.tag = index
            slider.addTarget(self, action: #selector(controlChanged), for: .valueChanged)

            let row = UIStackView(arrangedSubviews: [titleLabel, slider, valueLabel])
            row.axis = .horizontal
            row.spacing = 8
            row.alignment = .center

            stack.addArrangedSubview(row)
            sliders.append(slider)
            valueLabels.append(valueLabel)
        }

        panel.layer.cornerRadius = 12
        panel.clipsToBounds = true
        panel.translatesAutoresizingMaskIntoConstraints = false
        panel.contentView.addSubview(stack)
        view.addSubview(panel)

        NSLayoutConstraint.activate([
            panel.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 12),
            panel.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -12),
            panel.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -12),
            stack.leadingAnchor.constraint(equalTo: panel.contentView.leadingAnchor, constant: 14),
            stack.trailingAnchor.constraint(equalTo: panel.contentView.trailingAnchor, constant: -14),
            stack.topAnchor.constraint(equalTo: panel.contentView.topAnchor, constant: 10),
            stack.bottomAnchor.constraint(equalTo: panel.contentView.bottomAnchor, constant: -10),
        ])

        updateLabels()
    }

    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        // Keep the logo and the attribution button visible above the panel
        let bottom = view.bounds.maxY - panel.frame.minY - view.safeAreaInsets.bottom + 8
        mapView.logoViewMargins = CGPoint(x: mapView.logoViewMargins.x, y: bottom)
        mapView.attributionButtonMargins = CGPoint(x: mapView.attributionButtonMargins.x, y: bottom)
    }

    @objc private func controlChanged() {
        applyAll()
    }

    private func updateLabels() {
        for (index, control) in controls.enumerated() {
            valueLabels[index].text = String(format: control.format, sliders[index].value)
            sliders[index].isEnabled = enabledSwitch.isOn
        }
    }

    private func applyAll() {
        updateLabels()
        guard let style = mapView.style else { return }
        for layer in fillExtrusionLayers(in: style) {
            for (index, control) in controls.enumerated() {
                let value = control.isIntensity && !enabledSwitch.isOn ? 0 : sliders[index].value
                control.apply(layer, NSExpression(forConstantValue: value))
            }
        }
    }
}
```
