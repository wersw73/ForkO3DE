/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Artifact/Static/TestImpactBuildTargetDescriptor.h>

namespace TestImpact
{
    //! Artifact produced by the target artifact compiler that represents a production build target in the repository.
    struct ProductionTargetDescriptor
        : public BuildTargetDescriptor
    {
        ProductionTargetDescriptor(BuildTargetDescriptor&& buildTarget);
    };
} // namespace TestImpact
