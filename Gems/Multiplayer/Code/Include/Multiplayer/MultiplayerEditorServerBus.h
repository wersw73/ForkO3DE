/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>

namespace Multiplayer
{
    class MultiplayerEditorServerRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

        virtual void SendEditorServerInitPacket(AzNetworking::IConnection* connection) = 0;
    };
    using MultiplayerEditorServerRequestBus = AZ::EBus<MultiplayerEditorServerRequests>;
} // namespace Multiplayer
