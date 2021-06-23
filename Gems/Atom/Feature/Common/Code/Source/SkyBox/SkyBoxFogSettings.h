/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#pragma once

#include <AzCore/Math/Color.h>
#include <AzCore/RTTI/RTTI.h>

namespace AZ
{
    namespace Render
    {
        struct SkyBoxFogSettings final
        {
            AZ_RTTI(AZ::Render::SkyBoxFogSettings, "{DB13027C-BA92-4E46-B428-BB77C2A80C51}");
            
            static void Reflect(ReflectContext* context);

            SkyBoxFogSettings() = default;

            bool IsFogDisabled() const;

            AZ::Color m_color = AZ::Color::CreateOne();
            bool m_enable = false;
            float m_topHeight = 0.01;
            float m_bottomHeight = 0.0;
        };
    }
}
