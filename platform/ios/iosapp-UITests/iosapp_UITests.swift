//
//  iosapp_UITests.swift
//  iosapp UITests
//
//  Copyright © 2021 MapLibre. All rights reserved.
//

import XCTest

class iosapp_UITests: XCTestCase {
    
    let app = XCUIApplication()

    override func setUpWithError() throws {
        // Put setup code here. This method is called before the invocation of each test method in the class.
        // UI tests must launch the application that they test.
        app.launchEnvironment.updateValue("YES", forKey: "UITesting")
        app.launch()

        // In UI tests it is usually best to stop immediately when a failure occurs.
        continueAfterFailure = false

        // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
    }

    override func tearDownWithError() throws {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    /// Launch `iosapp`, reset to XYZ = (0, 0, 0) and take a screenshot
    func test_iosappScheme() throws {
        /// Launch and take a screenshot
        app.navigationBars["MapLibre Basic"].buttons["Map settings"].tap()
        app.tables/*@START_MENU_TOKEN@*/.staticTexts["Reset position"]/*[[".cells.staticTexts[\"Reset position\"]",".staticTexts[\"Reset position\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        sleep(1)
        add(screenshot())
    }
    
    /// Turn on Debug tile boundaries, tile info and FPS ornaments
    /// Demonstrates how the Tile Boundaries look when rendered
    func testDebugBoundaryTiles() {
        app.windows.children(matching: .other).element.children(matching: .other).element.children(matching: .other).element.doubleTap()
        
        let mapSettingsButton = app.navigationBars["MapLibre Basic"].buttons["Map settings"]
        mapSettingsButton.tap()
        
        /// setup initial conditions for position (0,0) and debug settings
        let tablesQuery = app.tables
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show zoom level ornament"]/*[[".cells.staticTexts[\"Show zoom level ornament\"]",".staticTexts[\"Show zoom level ornament\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show tile boundaries"]/*[[".cells.staticTexts[\"Show tile boundaries\"]",".staticTexts[\"Show tile boundaries\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show tile info"]/*[[".cells.staticTexts[\"Show tile info\"]",".staticTexts[\"Show tile info\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show tile timestamps"]/*[[".cells.staticTexts[\"Show tile timestamps\"]",".staticTexts[\"Show tile timestamps\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery.staticTexts["Add Test Shapes"].tap()
        
        sleep(1)
        add(screenshot(name: "Add Test Shapes"))

        mapSettingsButton.tap()
        tablesQuery.staticTexts["Reset position"].tap()
        
        sleep(1)
        add(screenshot(name: "Null Island, Zoom=0"))
    }

    var mapSettingsButton: XCUIElement { get { app.navigationBars["MapLibre Basic"].buttons["Map settings"] } }

    /// Open and close the secondary map view a few times to ensure that the dynamic layout adjustment doesn't crash
    func testSecondMap() {
        let showTimeout = 1.0
        let hideTimeout = 10.0
        let iterations = 3

        let secondMapQuery = app.otherElements["Second Map"]
        XCTAssert(!secondMapQuery.exists)

        for _ in 0..<iterations {
            mapSettingsButton.tap()
            app.tables.staticTexts["Show Second Map"].tap()
            XCTAssert(secondMapQuery.waitForExistence(timeout: showTimeout))

            sleep(1)

            mapSettingsButton.tap()
            app.tables.staticTexts["Hide Second Map"].tap()

            expectation(for: NSPredicate(format: "exists == 0"), evaluatedWith: secondMapQuery)
            waitForExpectations(timeout: hideTimeout, handler: nil)
        }
    }

    func testAll() {
        let allItems = [
        "Reset position",
        "Show tile boundaries",
        "Hide tile boundaries",
        "Show tile info",
        "Hide tile info",
        "Show tile timestamps",
        "Hide tile timestamps",
        "Show collision boxes",
        "Hide collision boxes",
        "Show overdraw visualization",
        "Hide overdraw visualization",
        "Show zoom level ornament",
        "Hide zoom level ornament",
        "Show frame time graph",
        "Hide frame time graph",
        "Show reuse queue stats",
        "Hide reuse queue stats",
        "Add 100 Views",
        //"Add 1,000 Views",    // These cause the test to time out
        //"Add 10,000 Views",
        "Add 100 Sprites",
        //"Add 1,000 Sprites",
        //"Add 10,000 Sprites",
        "Animate an Annotation View",
        "Add Test Shapes",
        "Add 10x Test Shapes",
        "Add Point With Custom Callout",
        "Query Annotations",
        "Enable Custom User Dot",
        "Disable Custom User Dot",
        "Remove Annotations",
        "Select an offscreen point annotation",
        "Center selected annotation",
        "Add visible area polyline",
        "Add Building Extrusions",
        "Style Water With Function",
        "Style Roads With Function",
        "Add Raster & Apply Function",
        "Add Shapes & Apply Fill",
        "Style Symbol Color",
        "Style Building Fill Color",
        "Style Ferry Line Color",
        "Remove Parks",
        "Style Fill With Filter",
        "Style Lines With Filter",
        "Style Fill With Numeric Filter",
        "Query and Style Features",
        "Style Feature",
        "Style Dynamic Point Collection",
        "Update Shape Source: Data",
        "Update Shape Source: URL",
        "Update Shape Source: Features",
        "Style Vector Tile Source",
        "Style Raster Tile Source",
        "Style Image Source",
        "Add Route Line",
        "Dynamically Style Polygon",
        "Add Custom Lat/Lon Grid",
        "Style Route line with gradient",
        "Start World Tour",
        "Random Tour",
        "Show Second Map",
        "Hide Second Map",
        "Missing Icon",
        "Limit Camera Changes",
        "Unlimit Camera Changes",
        "Turn On Content Insets",
        "Turn Off Content Insets",
        "Lat Long bounds with padding",
        "Show Labels in Default Language",
        ]

        for label in allItems {
            mapSettingsButton.tap()
            app.tables.staticTexts[label].tap()
            sleep(1)
        }

        // Only one of these will show up, run whichever one is there
        mapSettingsButton.tap()
        if let customLayer = staticItemIfExists("Add Custom Triangle Layer (OpenGL)") ??
                             staticItemIfExists("Add Custom Triangle Layer (Metal)") {
            customLayer.tap()
            sleep(1)
            mapSettingsButton.tap()
        }

        if let customLayer = staticItemIfExists("Add Custom Drawable Layer") {
            customLayer.tap()
            sleep(1)
            mapSettingsButton.tap()
        }

        // See `bestLanguageForUser`
        for loc in [ "ar", "de", "en", "es", "fr", "ja", "ko", "pt", "ru", "zh", "zh-Hans", "zh-Hant" ] {
            if let name = NSLocale(localeIdentifier: loc).displayName(forKey: .identifier, value: loc) {
                if let item = staticItemIfExists("Show Labels in " + name) {
                    item.tap()
                    sleep(1)
                    mapSettingsButton.tap()
                }
            }
        }

        // These open another view controller that then needs to be closed

        app.tables.staticTexts["View Route Simulation"].tap()
        XCTAssert(tapBackButton("MBXCustomLocationView", timeout: 5))

        mapSettingsButton.tap()
        app.tables.staticTexts["Ornaments Placement"].tap()
        XCTAssert(tapBackButton("Ornaments", timeout: 5))

        mapSettingsButton.tap()
        app.tables.staticTexts["Show Snapshots"].tap()
        XCTAssert(tapBackButton("MBXSnapshotsView", timeout: 5))

        mapSettingsButton.tap()
        app.tables.staticTexts["Embedded Map View"].tap()
        XCTAssert(tapBackButton("MBXEmbeddedMapView", timeout: 5))
    }

    private func staticItemIfExists(_ ident: String) -> XCUIElement? {
        let item = app.staticTexts[ident]
        return item.exists ? item : nil
    }

    private func tapBackButton(_ label: String, timeout: TimeInterval) -> Bool {
        let bar = app.navigationBars[label]
        if (bar.waitForExistence(timeout: timeout)) {
            let button = bar.buttons["Back"]
            if (button.waitForExistence(timeout: 1)) {
                button.tap()
                return true
            }
        }
        return false
    }

    func testRecord() {
        /// Use recording to get started writing UI tests.
        ///   Use `Editor` > `Start Recording UI Test` while your cursor is in this `func`
        /// Use XCTAssert and related functions to verify your tests produce the correct results.
    }

    func testLaunchPerformance() throws {
        if #available(macOS 10.15, iOS 13.0, tvOS 13.0, *) {
            // This measures how long it takes to launch your application.
            measure(metrics: [XCTApplicationLaunchMetric()]) {
                XCUIApplication().launch()
            }
        }
    }
    
    /// Make sure the Custom Drawable Layer does not crash
    func testCustomDrawableLayer() {
        app.windows.children(matching: .other).element.children(matching: .other).element.children(matching: .other).element.doubleTap()
        
        let mapSettingsButton = app.navigationBars["MapLibre Basic"].buttons["Map settings"]
        mapSettingsButton.tap()
        
        let tablesQuery = app.tables
        tablesQuery.staticTexts["Add Custom Drawable Layer"].tap()
        sleep(5)
    }
}
