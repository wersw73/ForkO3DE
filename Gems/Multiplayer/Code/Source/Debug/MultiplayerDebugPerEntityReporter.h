/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include "MultiplayerDebugByteReporter.h"

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <AzCore/Interface/Interface.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <Multiplayer/MultiplayerStats.h>
#include <Multiplayer/MultiplayerTypes.h>

namespace Multiplayer
{
    /**
     * \brief GridMate network live analysis tool via ImGui.
     */
    class MultiplayerDebugPerEntityReporter
    {
    public:
        MultiplayerDebugPerEntityReporter();
        ~MultiplayerDebugPerEntityReporter();

        // main update loop
        void OnImGuiUpdate();

        //! Event handlers
        // @{
        void RecordEntitySerializeStart(AzNetworking::SerializerMode mode, AZ::EntityId entityId, const char* entityName);
        void RecordComponentSerializeEnd(AzNetworking::SerializerMode mode, Multiplayer::NetComponentId netComponentId);
        void RecordEntitySerializeStop(AzNetworking::SerializerMode mode, AZ::EntityId entityId, const char* entityName);
        void RecordPropertySent(Multiplayer::NetComponentId netComponentId, Multiplayer::PropertyIndex propertyId, uint32_t totalBytes);
        void RecordPropertyReceived(Multiplayer::NetComponentId netComponentId, Multiplayer::PropertyIndex propertyId, uint32_t totalBytes);
        void RecordRpcSent(AZ::EntityId entityId, const char* entityName, Multiplayer::NetComponentId netComponentId, Multiplayer::RpcIndex rpcId, uint32_t totalBytes);
        void RecordRpcReceived(AZ::EntityId entityId, const char* entityName, Multiplayer::NetComponentId netComponentId, Multiplayer::RpcIndex rpcId, uint32_t totalBytes);
        // }@

        void UpdateDebugOverlay();

    private:

        AZ::ScheduledEvent m_updateDebugOverlay;
        Multiplayer::MultiplayerStats::EventHandlers m_eventHandlers;

        AZStd::map<AZ::EntityId, MultiplayerDebugEntityReporter> m_sendingEntityReports{};
        MultiplayerDebugEntityReporter m_currentSendingEntityReport;

        AZStd::map<AZ::EntityId, MultiplayerDebugEntityReporter> m_receivingEntityReports{};
        MultiplayerDebugEntityReporter m_currentReceivingEntityReport;

        float m_replicatedStateKbpsWarn = 10.f;
        float m_replicatedStateMaxSizeWarn = 30.f;

        char m_statusBuffer[100] = {};

        struct NetworkEntityTraffic
        {
            const char* m_name = nullptr;
            float m_up = 0.f;
            float m_down = 0.f;
        };

        AZStd::fixed_unordered_map<AZ::EntityId, NetworkEntityTraffic, 5, 10> m_networkEntitiesTraffic;

        AzFramework::DebugDisplayRequests* m_debugDisplay = nullptr;
    };
}
