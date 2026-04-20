#pragma once

#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Simple 3D vector for presentation internal logic.
 */
struct Vec3 {
    float x, y, z;
};

/**
 * @brief High-level presentation mode for the project or scene.
 */
enum class PresentationMode {
    Classic2D,      // Standard 2D sprite/tile rendering
    Spatial,        // Enhanced 2.5D/Spatial rendering
    Spatial3D = Spatial, // Alias for testing compatibility
    Count
};

/**
 * @brief Platform/Device capability tiers.
 */
enum class CapabilityTier {
    Tier0_Baseline,    // Guaranteed on all hardware (2D only)
    Tier0_Basic = Tier0_Baseline,
    Tier1_Standard,    // Basic Spatial features
    Tier2_Enhanced,    // Advanced lighting/particles
    Tier3_Full,        // Maximum fidelity
    Count
};

/**
 * @brief Diagnostic severity levels (Section 15).
 */
enum class DiagnosticSeverity {
    Notice,
    Warning,
    Error,
    Debug
};

/**
 * @brief A single diagnostic entry.
 */
struct DiagnosticEntry {
    DiagnosticSeverity severity;
    std::string message;
    std::string context; // e.g. "MapScene" or "Asset: Tree01"
};

/**
 * @brief Helper to resolve presentation mode logic.
 * ADR-002: Mode resolution order is Scene Override > Project Default.
 */
struct PresentationResolver {
    static PresentationMode Resolve(PresentationMode projectDefault, PresentationMode sceneOverride) {
        // If scene override is Classic2D or Spatial, it wins.
        // We assume 'Count' or a special 'None' would mean "no override".
        if (sceneOverride != PresentationMode::Count) {
             return sceneOverride;
        }
        return projectDefault;
    }
};

} // namespace urpg::presentation
