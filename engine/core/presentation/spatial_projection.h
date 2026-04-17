#pragma once

#include "presentation_camera.h"
#include "presentation_schema.h"
#include "engine/core/math/vector2.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace urpg::presentation {

struct ViewportRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 1.0f;
    float height = 1.0f;
};

struct WorldRay {
    Vec3 origin{0.0f, 0.0f, 0.0f};
    Vec3 direction{0.0f, -1.0f, 0.0f};
};

struct ResolvedCameraPose {
    Vec3 eye{0.0f, 0.0f, 0.0f};
    Vec3 target{0.0f, 0.0f, 0.0f};
    Vec3 forward{0.0f, 0.0f, 1.0f};
    Vec3 right{1.0f, 0.0f, 0.0f};
    Vec3 up{0.0f, 1.0f, 0.0f};
};

class SpatialProjection {
public:
    static constexpr float kEpsilon = 1e-5f;
    static constexpr float kMaxTraceDistance = 10000.0f;
    static constexpr float kTraceStep = 0.05f;

    static ResolvedCameraPose ResolveCameraPose(
        const CameraState& state,
        const CameraProfile& profile) {
        ResolvedCameraPose pose;
        pose.target = {
            state.targetPos.x + profile.lookAtOffset.x,
            state.targetPos.y + profile.lookAtOffset.y,
            state.targetPos.z + profile.lookAtOffset.z
        };

        const float pitch = DegreesToRadians(std::clamp(state.currentPitch, profile.minPitch, profile.maxPitch));
        const float yaw = DegreesToRadians(state.currentYaw);
        const float distance = std::clamp(state.currentDistance, profile.minDistance, profile.maxDistance);

        pose.forward = Normalize({
            std::cos(pitch) * std::sin(yaw),
            -std::sin(pitch),
            std::cos(pitch) * std::cos(yaw)
        });

        pose.eye = {
            pose.target.x - pose.forward.x * distance,
            pose.target.y - pose.forward.y * distance,
            pose.target.z - pose.forward.z * distance
        };

        const Vec3 worldUp{0.0f, 1.0f, 0.0f};
        pose.right = Normalize(Cross(pose.forward, worldUp));
        if (LengthSq(pose.right) < kEpsilon) {
            pose.right = {1.0f, 0.0f, 0.0f};
        }
        pose.up = Normalize(Cross(pose.right, pose.forward));
        return pose;
    }

    static WorldRay ScreenPointToWorldRay(
        const Vector2f& screenPoint,
        const ViewportRect& viewport,
        const CameraState& state,
        const CameraProfile& profile) {
        const ResolvedCameraPose pose = ResolveCameraPose(state, profile);
        const Vector2f ndc = ScreenToNdc(screenPoint, viewport);
        const float aspect = SafeAspect(viewport);

        if (state.isOrtho) {
            const float halfHeight = std::max(profile.orthoSize, kEpsilon);
            const float halfWidth = halfHeight * aspect;
            return {
                {
                    pose.eye.x + pose.right.x * (ndc.x * halfWidth) + pose.up.x * (ndc.y * halfHeight),
                    pose.eye.y + pose.right.y * (ndc.x * halfWidth) + pose.up.y * (ndc.y * halfHeight),
                    pose.eye.z + pose.right.z * (ndc.x * halfWidth) + pose.up.z * (ndc.y * halfHeight)
                },
                pose.forward
            };
        }

        const float halfFovTan = std::tan(DegreesToRadians(profile.fov) * 0.5f);
        Vec3 direction = Normalize({
            pose.forward.x + pose.right.x * (ndc.x * aspect * halfFovTan) + pose.up.x * (ndc.y * halfFovTan),
            pose.forward.y + pose.right.y * (ndc.x * aspect * halfFovTan) + pose.up.y * (ndc.y * halfFovTan),
            pose.forward.z + pose.right.z * (ndc.x * aspect * halfFovTan) + pose.up.z * (ndc.y * halfFovTan)
        });
        return {pose.eye, direction};
    }

    static std::optional<Vec3> IntersectGroundPlane(const WorldRay& ray, float groundY = 0.0f) {
        if (std::fabs(ray.direction.y) < kEpsilon) {
            return std::nullopt;
        }

        const float t = (groundY - ray.origin.y) / ray.direction.y;
        if (t < 0.0f) {
            return std::nullopt;
        }

        return Vec3{
            ray.origin.x + ray.direction.x * t,
            groundY,
            ray.origin.z + ray.direction.z * t
        };
    }

    static std::optional<Vec3> IntersectElevationGrid(
        const WorldRay& ray,
        const ElevationGrid& elevation) {
        if (elevation.width == 0 || elevation.height == 0 || elevation.levels.empty()) {
            return IntersectGroundPlane(ray, 0.0f);
        }

        for (float distance = 0.0f; distance <= kMaxTraceDistance; distance += kTraceStep) {
            const Vec3 sample{
                ray.origin.x + ray.direction.x * distance,
                ray.origin.y + ray.direction.y * distance,
                ray.origin.z + ray.direction.z * distance
            };

            if (sample.x < 0.0f || sample.z < 0.0f) {
                continue;
            }

            const uint32_t tileX = static_cast<uint32_t>(std::floor(sample.x));
            const uint32_t tileY = static_cast<uint32_t>(std::floor(sample.z));
            if (tileX >= elevation.width || tileY >= elevation.height) {
                continue;
            }

            const float cellHeight = elevation.GetWorldHeight(tileX, tileY);
            if (sample.y <= cellHeight + kTraceStep) {
                return Vec3{sample.x, cellHeight, sample.z};
            }
        }

        return std::nullopt;
    }

private:
    static float DegreesToRadians(float degrees) {
        return degrees * 0.01745329251994329577f;
    }

    static float SafeAspect(const ViewportRect& viewport) {
        return viewport.height > kEpsilon ? viewport.width / viewport.height : 1.0f;
    }

    static Vector2f ScreenToNdc(const Vector2f& screenPoint, const ViewportRect& viewport) {
        const float width = std::max(viewport.width, 1.0f);
        const float height = std::max(viewport.height, 1.0f);
        return {
            ((screenPoint.x - viewport.x) / width) * 2.0f - 1.0f,
            1.0f - ((screenPoint.y - viewport.y) / height) * 2.0f
        };
    }

    static float LengthSq(const Vec3& v) {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }

    static Vec3 Normalize(const Vec3& v) {
        const float lenSq = LengthSq(v);
        if (lenSq < kEpsilon) {
            return {0.0f, 0.0f, 0.0f};
        }

        const float invLen = 1.0f / std::sqrt(lenSq);
        return {v.x * invLen, v.y * invLen, v.z * invLen};
    }

    static Vec3 Cross(const Vec3& a, const Vec3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }
};

} // namespace urpg::presentation
