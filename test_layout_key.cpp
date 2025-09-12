#include <iostream>
#include <string>
#include <mbgl/renderer/group_by_layout.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>

using namespace mbgl;
using namespace mbgl::style;

int main() {
    // Create test layers
    auto fillLayer = std::make_unique<FillLayer>("test-fill", "test-source");
    fillLayer->setSourceLayer("countries");
    
    auto lineLayer = std::make_unique<LineLayer>("test-line", "test-source");
    lineLayer->setSourceLayer("countries");
    
    // Generate layout keys
    std::string fillKey = layoutKey(*fillLayer->baseImpl);
    std::string lineKey = layoutKey(*lineLayer->baseImpl);
    
    std::cout << "Fill layer key: " << fillKey << std::endl;
    std::cout << "Line layer key: " << lineKey << std::endl;
    
    // Check that keys don't contain raw pointer addresses
    // They should now contain layer type names instead
    if (fillKey.find("fill") != std::string::npos) {
        std::cout << "SUCCESS: Fill layer key contains 'fill' type name" << std::endl;
    } else {
        std::cout << "ERROR: Fill layer key doesn't contain expected type name" << std::endl;
        return 1;
    }
    
    if (lineKey.find("line") != std::string::npos) {
        std::cout << "SUCCESS: Line layer key contains 'line' type name" << std::endl;
    } else {
        std::cout << "ERROR: Line layer key doesn't contain expected type name" << std::endl;
        return 1;
    }
    
    // Verify no large numbers (pointer addresses) in keys
    bool hasLargeNumber = false;
    for (size_t i = 0; i < fillKey.length() - 9; i++) {
        if (std::isdigit(fillKey[i])) {
            std::string numStr = fillKey.substr(i, 10);
            try {
                long long num = std::stoll(numStr);
                if (num > 1000000000) { // Likely a pointer address
                    hasLargeNumber = true;
                    std::cout << "WARNING: Found large number in key: " << num << std::endl;
                    break;
                }
            } catch (...) {
                // Not a valid number, continue
            }
        }
    }
    
    if (!hasLargeNumber) {
        std::cout << "SUCCESS: No pointer addresses found in keys" << std::endl;
    } else {
        std::cout << "ERROR: Keys still contain pointer addresses" << std::endl;
        return 1;
    }
    
    return 0;
}