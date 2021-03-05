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
        app.launch()

        // In UI tests it is usually best to stop immediately when a failure occurs.
        continueAfterFailure = false

        // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
    }

    override func tearDownWithError() throws {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testExample() throws {
        /// Launch and take a screenshot
        add(screenshot())

        // Use recording to get started writing UI tests.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
    }
    
    /// Turn on Debug tile boundaries, tile info and FPS ornaments
    func testDebugBoundaryTiles() {
        let mapSettingsButton = app.navigationBars["Streets"].buttons["Map settings"]
        mapSettingsButton.tap()
        
        /// setup initial conditions for position (0,0) and debug settings
        let tablesQuery = app.tables
        tablesQuery.staticTexts["Reset position"].tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show zoom level ornament"]/*[[".cells.staticTexts[\"Show zoom level ornament\"]",".staticTexts[\"Show zoom level ornament\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show tile boundaries"]/*[[".cells.staticTexts[\"Show tile boundaries\"]",".staticTexts[\"Show tile boundaries\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show tile info"]/*[[".cells.staticTexts[\"Show tile info\"]",".staticTexts[\"Show tile info\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()
        tablesQuery/*@START_MENU_TOKEN@*/.staticTexts["Show tile timestamps"]/*[[".cells.staticTexts[\"Show tile timestamps\"]",".staticTexts[\"Show tile timestamps\"]"],[[[-1,1],[-1,0]]],[0]]@END_MENU_TOKEN@*/.tap()
        mapSettingsButton.tap()

        add(screenshot(name: "Null Island Tiles"))
    }

    func testLaunchPerformance() throws {
        if #available(macOS 10.15, iOS 13.0, tvOS 13.0, *) {
            // This measures how long it takes to launch your application.
            measure(metrics: [XCTApplicationLaunchMetric()]) {
                XCUIApplication().launch()
            }
        }
    }
}
