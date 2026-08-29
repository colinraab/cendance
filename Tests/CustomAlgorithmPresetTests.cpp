// P2: Custom algorithm preset tests — stub for now
// These require deeper integration with the algorithm registry
#include "../Source/App/CustomAlgorithmPreset.h"
#include "../Source/App/AlgorithmPresetRegistry.h"

#include <cassert>
#include <iostream>

void testCustomAlgorithmIdBase() {
    // Verify the custom algorithm ID base constant
    assert(AlgorithmPresetRegistry::kCustomAlgorithmIdBase == 2048);
}

void testCustomAlgorithmIdDetection() {
    AlgorithmPresetRegistry registry;
    // Custom IDs should be >= 2048
    assert(registry.isCustomAlgorithmId(2048));
    assert(registry.isCustomAlgorithmId(2049));
    assert(registry.isCustomAlgorithmId(3000));
    // Builtin IDs should be < 2048
    assert(!registry.isCustomAlgorithmId(0));
    assert(!registry.isCustomAlgorithmId(1));
    assert(!registry.isCustomAlgorithmId(2047));
}

void testCustomAlgorithmIdSanitize() {
    // Valid IDs are normalized to the registry's stable lowercase/underscore form.
    assert(sanitizeAlgorithmId("my-algo-123") == "my_algo_123");
    assert(sanitizeAlgorithmId("Algo_456") == "algo_456");

    // Invalid characters should be replaced
    std::string sanitized = sanitizeAlgorithmId("my algo!@#");
    assert(sanitized.find(' ') == std::string::npos);
    assert(sanitized.find('!') == std::string::npos);

    // Empty should produce a default
    std::string emptySanitized = sanitizeAlgorithmId("");
    assert(!emptySanitized.empty());
}

void testCustomAlgorithmIdBaseConstant() {
    // The base offset for custom algorithm IDs
    assert(AlgorithmPresetRegistry::kCustomAlgorithmIdBase == 2048);
}

int main() {
    testCustomAlgorithmIdBase();
    std::cout << "  testCustomAlgorithmIdBase passed\n";

    testCustomAlgorithmIdDetection();
    std::cout << "  testCustomAlgorithmIdDetection passed\n";

    testCustomAlgorithmIdSanitize();
    std::cout << "  testCustomAlgorithmIdSanitize passed\n";

    testCustomAlgorithmIdBaseConstant();
    std::cout << "  testCustomAlgorithmIdBaseConstant passed\n";

    std::cout << "CustomAlgorithmPreset tests passed!\n";
    return 0;
}
