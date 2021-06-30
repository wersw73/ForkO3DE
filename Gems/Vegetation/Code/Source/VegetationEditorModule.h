/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <VegetationModule.h>

namespace Vegetation
{
    class VegetationEditorModule
        : public VegetationModule
    {
    public:
        AZ_RTTI(VegetationEditorModule, "{8BA356E4-A07D-46A4-ADE1-B17F3BA032BF}", VegetationModule);
        AZ_CLASS_ALLOCATOR(VegetationEditorModule, AZ::SystemAllocator, 0);

        VegetationEditorModule();

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override;
    };
}
