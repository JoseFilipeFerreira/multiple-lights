// Copyright 2017-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ArcballCamera.h"
#include <ospcommon/math/AffineSpace.h>
#include <ospcommon/math/Quaternion.h>
#include <ospcommon/math/constants.h>
#include <ospcommon/math/vec.h>

#include <cmath>
#include <iostream>

template <typename T>
ospcommon::math::QuaternionT<T> qcast(const ospcommon::math::vec3f& vx,
                                      const ospcommon::math::vec3f& vy,
                                      const ospcommon::math::vec3f& vz) {
    T fourXSquaredMinus1       = vx.x - vy.y - vz.z;
    T fourYSquaredMinus1       = vy.y - vx.x - vz.z;
    T fourZSquaredMinus1       = vz.z - vx.x - vy.y;
    T fourWSquaredMinus1       = vx.x + vy.y + vz.z;
    int biggestIndex           = 0;
    T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourXSquaredMinus1;
        biggestIndex             = 1;
    }
    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourYSquaredMinus1;
        biggestIndex             = 2;
    }
    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourZSquaredMinus1;
        biggestIndex             = 3;
    }

    T biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<T>(1)) * static_cast<T>(0.5);
    T mult       = static_cast<T>(0.25) / biggestVal;

    switch (biggestIndex) {
        case 0:
            return ospcommon::math::QuaternionT<T>(
                biggestVal, (vy.z - vz.y) * mult, (vz.x - vx.z) * mult, (vx.y - vy.x) * mult);
        case 1:
            return ospcommon::math::QuaternionT<T>(
                (vy.z - vz.y) * mult, biggestVal, (vx.y + vy.x) * mult, (vz.x + vx.z) * mult);
        case 2:
            return ospcommon::math::QuaternionT<T>(
                (vz.x - vx.z) * mult, (vx.y + vy.x) * mult, biggestVal, (vy.z + vz.y) * mult);
        case 3:
            return ospcommon::math::QuaternionT<T>(
                (vx.y - vy.x) * mult, (vz.x + vx.z) * mult, (vy.z + vz.y) * mult, biggestVal);
        default
            :  // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
            assert(false);
            return ospcommon::math::QuaternionT<T>(1, 0, 0, 0);
    }
}

ArcballCamera::ArcballCamera(const ospcommon::math::vec3f& eye,
                             const ospcommon::math::vec3f& center,
                             const ospcommon::math::vec2i& windowSize)
    : zoomSpeed(1),
      invWindowSize(ospcommon::math::vec2f(1.0) / ospcommon::math::vec2f(windowSize)),
      centerTranslation(ospcommon::math::one),
      translation(ospcommon::math::one),
      rotation(ospcommon::math::quaternionf(0, 0, 1, 0)) {
    const ospcommon::math::vec3f up(0, 1, 0);
    const ospcommon::math::vec3f dir = center - eye;
    ospcommon::math::vec3f z_axis    = ospcommon::math::normalize(dir);
    ospcommon::math::vec3f x_axis =
        ospcommon::math::normalize(ospcommon::math::cross(z_axis, ospcommon::math::normalize(up)));
    ospcommon::math::vec3f y_axis = ospcommon::math::normalize(ospcommon::math::cross(x_axis, z_axis));
    x_axis                        = ospcommon::math::normalize(ospcommon::math::cross(z_axis, y_axis));

    centerTranslation = ospcommon::math::rcp(ospcommon::math::AffineSpace3f::translate(center));
    translation =
        ospcommon::math::AffineSpace3f::translate(ospcommon::math::vec3f(0.f, 0.f, ospcommon::math::length(dir)));

    // translation = ospcommon::math::AffineSpace3f::translate(dir);
    auto r   = ospcommon::math::LinearSpace3f(-x_axis, y_axis, z_axis).transposed();
    rotation = ospcommon::math::normalize(qcast<float>(-x_axis, y_axis, z_axis));
    updateCamera();
}

ArcballCamera::ArcballCamera(const ospcommon::math::box3f& worldBounds, const ospcommon::math::vec2i& windowSize)
    : zoomSpeed(1),
      invWindowSize(ospcommon::math::vec2f(1.0) / ospcommon::math::vec2f(windowSize)),
      centerTranslation(ospcommon::math::one),
      translation(ospcommon::math::one),
      rotation(ospcommon::math::one) {
    ospcommon::math::vec3f diag = worldBounds.size();
    zoomSpeed                   = ospcommon::math::max(length(diag) / 150.0, 0.001);
    diag                        = ospcommon::math::max(diag, ospcommon::math::vec3f(0.3f * length(diag)));

    centerTranslation = ospcommon::math::AffineSpace3f::translate(-worldBounds.center());
    translation       = ospcommon::math::AffineSpace3f::translate(ospcommon::math::vec3f(0, 0, length(diag)));
    updateCamera();
}

void ArcballCamera::rotate(const ospcommon::math::vec2f& from, const ospcommon::math::vec2f& to) {
    rotation = screenToArcball(to) * screenToArcball(from) * rotation;
    updateCamera();
}

void ArcballCamera::zoom(float amount) {
    amount *= zoomSpeed;
    translation = ospcommon::math::AffineSpace3f::translate(ospcommon::math::vec3f(0, 0, amount)) * translation;
    updateCamera();
}

void ArcballCamera::pan(const ospcommon::math::vec2f& delta) {
    const ospcommon::math::vec3f t = ospcommon::math::vec3f(-delta.x * invWindowSize.x, delta.y * invWindowSize.y, 0);
    const ospcommon::math::vec3f worldt = translation.p.z * xfmVector(invCamera, t);
    centerTranslation                   = ospcommon::math::AffineSpace3f::translate(worldt) * centerTranslation;
    updateCamera();
}

ospcommon::math::vec3f ArcballCamera::eyePos() const {
    return xfmPoint(invCamera, ospcommon::math::vec3f(0, 0, 1));
}

ospcommon::math::vec3f ArcballCamera::center() const {
    return -centerTranslation.p;
}

ospcommon::math::vec3f ArcballCamera::lookDir() const {
    return xfmVector(invCamera, ospcommon::math::vec3f(0, 0, 1));
}

ospcommon::math::vec3f ArcballCamera::upDir() const {
    return xfmVector(invCamera, ospcommon::math::vec3f(0, 1, 0));
}

void ArcballCamera::updateCamera() {
    const ospcommon::math::AffineSpace3f rot    = ospcommon::math::LinearSpace3f(rotation);
    const ospcommon::math::AffineSpace3f camera = translation * rot * centerTranslation;
    invCamera                                   = rcp(camera);
}

void ArcballCamera::setRotation(ospcommon::math::quaternionf q) {
    rotation = q;
    updateCamera();
}

void ArcballCamera::updateWindowSize(const ospcommon::math::vec2i& windowSize) {
    invWindowSize = ospcommon::math::vec2f(1) / ospcommon::math::vec2f(windowSize);
}

ospcommon::math::quaternionf ArcballCamera::screenToArcball(const ospcommon::math::vec2f& p) {
    const float dist = dot(p, p);
    // If we're on/in the sphere return the point on it
    if (dist <= 1.f) {
        return ospcommon::math::quaternionf(0, p.x, p.y, std::sqrt(1.f - dist));
    } else {
        // otherwise we project the point onto the sphere
        const ospcommon::math::vec2f unitDir = normalize(p);
        return ospcommon::math::quaternionf(0, unitDir.x, unitDir.y, 0);
    }
}
