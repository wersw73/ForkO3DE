/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Allocators.h>
#include <QueryVector.h>

namespace EMotionFX::MotionMatching
{
    AZ_CLASS_ALLOCATOR_IMPL(QueryVector, MotionMatchAllocator, 0)

    void QueryVector::SetVector2(const AZ::Vector2& value, size_t offset)
    {
        operator[](offset + 0) = value.GetX();
        operator[](offset + 1) = value.GetY();
    }

    void QueryVector::SetVector3(const AZ::Vector3& value, size_t offset)
    {
        operator[](offset + 0) = value.GetX();
        operator[](offset + 1) = value.GetY();
        operator[](offset + 2) = value.GetZ();
    }

    AZ::Vector2 QueryVector::GetVector2(size_t offset) const
    {
        return AZ::Vector2(operator[](offset + 0), operator[](offset + 1));
    }

    AZ::Vector3 QueryVector::GetVector3(size_t offset) const
    {
        return AZ::Vector3(operator[](offset + 0), operator[](offset + 1), operator[](offset + 2));
    }
} // namespace EMotionFX::MotionMatching
