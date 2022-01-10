/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityBus.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <Multiplayer/Components/NetworkHierarchyRootComponent.h>

namespace Multiplayer
{
    struct AuditTrailInput
    {
        AuditTrailInput(MultiplayerAuditCategory category, ClientInputId inputId, HostFrameId hostFrameId,
            const AZStd::string name, const AZStd::vector<MultiplayerAuditingElement>&& children)
            : m_category(category)
            , m_inputId(inputId)
            , m_hostFrameId(hostFrameId)
            , m_name(name)
            , m_children(children)
        {
        }

        MultiplayerAuditCategory m_category;
        ClientInputId m_inputId;
        HostFrameId m_hostFrameId;
        AZStd::string m_name;
        AZStd::vector<MultiplayerAuditingElement> m_children;
    };

    //! Buffer size for ImGui Search bar in Audit Trail UI
    const int AUDIT_SEARCH_BUFFER_SIZE = 1024;

    /**
     * /brief Provides ImGui driven UX for multiplayer audit trail
     */
    class MultiplayerDebugAuditTrail
    {
    public:
        static constexpr char DESYNC_TITLE[] = "Desync on %s";
        static constexpr char INPUT_TITLE[] = "%s Inputs";
        static constexpr char EVENT_TITLE[] = "DevEvent on %s";

        MultiplayerDebugAuditTrail();
        ~MultiplayerDebugAuditTrail();

        //! Main update loop.
        void OnImGuiUpdate(AZStd::deque<AuditTrailInput> auditTrailElems);

        //! Draws hierarchy information over hierarchy root entities.
        void UpdateDebugOverlay();

        //! Checks if the audit trail can be pumped and resets the pump flag
        bool CanPumpAuditTrail();

        //! Gets string filter for the audit trail
        AZStd::string_view GetAuditTrialFilter();

        //! Sets string filter for the audit trail
        void SetAuditTrailFilter(AZStd::string_view filter);
    private:
        AZ::ScheduledEvent m_updateDebugOverlay;

        AZStd::string_view m_filter;
        AzFramework::DebugDisplayRequests* m_debugDisplay = nullptr;
        char m_inputBuffer[AUDIT_SEARCH_BUFFER_SIZE] = {};
        bool m_canPumpTrail = false;
    };
}
