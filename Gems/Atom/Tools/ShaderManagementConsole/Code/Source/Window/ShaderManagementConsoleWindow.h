/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Memory/SystemAllocator.h>
#include <Atom/Document/ShaderManagementConsoleDocumentNotificationBus.h>

#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Edit/Shader/ShaderVariantListSourceData.h>

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
//#include <AzQtComponents/Components/DockMainWindow.h>
#include <AzQtComponents/Components/FancyDocking.h>
#include <AzQtComponents/Components/StyledDockWidget.h>
#include <AzQtComponents/Components/Widgets/TabWidget.h>
#include <AzQtComponents/Application/Window/AzQtApplicationWindow.h>

#include <Window/ShaderManagementConsoleBrowserWidget.h>
#include <Window/ToolBar/ShaderManagementConsoleToolBar.h>

#include <QMenuBar>
#include <QToolBar>
#include <QStandardItemModel>
AZ_POP_DISABLE_WARNING
#endif

namespace AzToolsFramework
{
    class CScriptTermDialog;
}

namespace ShaderManagementConsole
{
    /**
     * ShaderManagementConsoleWindow is the main class. Its responsibility is limited to initializing and connecting
     * its panels, managing selection of assets, and performing high-level actions like saving. It contains...
     */
    class ShaderManagementConsoleWindow
        : public AzQtComponents::AzQtApplicationWindow
        , private ShaderManagementConsoleDocumentNotificationBus::Handler
    {
        Q_OBJECT
    public:
        AZ_CLASS_ALLOCATOR(ShaderManagementConsoleWindow, AZ::SystemAllocator, 0);

        ShaderManagementConsoleWindow(QWidget* parent = 0);
        ~ShaderManagementConsoleWindow();

    private:
        // ShaderManagementConsoleDocumentNotificationBus::Handler overrides...
        void OnDocumentOpened(const AZ::Uuid& documentId) override;
        void OnDocumentClosed(const AZ::Uuid& documentId) override;
        void OnDocumentModified(const AZ::Uuid& documentId) override;
        void OnDocumentUndoStateChanged(const AZ::Uuid& documentId) override;
        void OnDocumentSaved(const AZ::Uuid& documentId) override;

        void SetupMenu() override;
        void SetupTabs() override;

        void AddTabForDocumentId(const AZ::Uuid& documentId);
        void RemoveTabForDocumentId(const AZ::Uuid& documentId);
        void UpdateTabForDocumentId(const AZ::Uuid& documentId);
        AZ::Uuid GetDocumentIdFromTab(const int tabIndex) const;

        void OpenTabContextMenu() override;

        void SelectDocumentForTab(const int tabIndex);
        void CloseDocumentForTab(const int tabIndex);
        void CloseAllExceptDocumentForTab(const int tabIndex);

        void closeEvent(QCloseEvent* closeEvent) override;

        void CreateDocumentContent(const AZ::Uuid& documentId, QStandardItemModel* model);

        ShaderManagementConsoleBrowserWidget* m_assetBrowser = nullptr;
        ShaderManagementConsoleToolBar* m_toolBar = nullptr;
        AzToolsFramework::CScriptTermDialog* m_pythonTerminal = nullptr;

        AzQtComponents::StyledDockWidget* m_assetBrowserDockWidget = nullptr;
        AzQtComponents::StyledDockWidget* m_pythonTerminalDockWidget = nullptr;
    };
} // namespace ShaderManagementConsole
