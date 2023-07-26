//
//  XCTestCase+Extensions.swift
//  iosapp UITests
//
//  Copyright © 2021 MapLibre. All rights reserved.
//

import XCTest

extension XCTestCase {

    /// `screenshot` — Save a PNG screenshot to the UITest results
    /// - Parameter name: friendly name to include.
    ///  Associate an image with the Activity - See [add()](https://developer.apple.com/documentation/xctest/xctactivity/2887222-add)
    /// - returns: XCTAttachment, which will need to be `add`ed to the `XCTActivity`
/**
```
/// rely on the calling test functions name
add(screenshot())

/// Add by name
add(screenshot(name: "Example"))
```
*/
    func screenshot(name: String = #function) -> XCTAttachment {
        let attachmentName = "MapLibre-Screenshot-\(name)-\(UIDevice.current.name)-\(UIDevice.current.systemVersion).png"
        let screenshot = XCUIScreen.main.screenshot()
        let attachment = XCTAttachment(uniformTypeIdentifier: "public.png",
                                       name: attachmentName,
                                       payload: screenshot.pngRepresentation,
                                       userInfo: nil)
        attachment.lifetime = .keepAlways

        return attachment
    }
}
