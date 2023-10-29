import XCTest
import MapLibre

class CustomAnnotationView: MLNAnnotationView {
    
    override init(reuseIdentifier: String?) {
        super.init(reuseIdentifier: reuseIdentifier)
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
}

class MLNAnnotationViewIntegrationTests: XCTestCase {
    
    func testCreatingCustomAnnotationView() {
        let customAnnotationView = CustomAnnotationView(reuseIdentifier: "resuse-id")
        XCTAssertNotNil(customAnnotationView)
    }
    
}
