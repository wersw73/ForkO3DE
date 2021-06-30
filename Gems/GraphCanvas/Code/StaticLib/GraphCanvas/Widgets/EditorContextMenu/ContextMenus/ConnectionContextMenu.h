/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <GraphCanvas/Widgets/EditorContextMenu/EditorContextMenu.h>

#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentActionsMenuGroup.h>
#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditActionsMenuGroup.h>

namespace GraphCanvas
{
    class ConnectionContextMenu
        : public EditorContextMenu
    {
    public:
        ConnectionContextMenu(EditorId editorId, QWidget* parent = nullptr);
        ~ConnectionContextMenu() override = default;
        
    protected:
    
        void OnRefreshActions(const GraphId& graphId, const AZ::EntityId& targetMemberId) override;

    private:

        EditActionsMenuGroup m_editActionsGroup;
        AlignmentActionsMenuGroup m_alignmentActionsGroup;
    };
}
