/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/std/containers/vector.h>
#include <Viewport/MaterialCanvasViewportNotificationBus.h>
#include <Viewport/MaterialCanvasViewportRequestBus.h>
#include <Viewport/MaterialCanvasViewportSettings.h>
#include <Window/ToolBar/LightingPresetComboBox.h>
#include <Window/ToolBar/MaterialCanvasToolBar.h>
#include <Window/ToolBar/ModelPresetComboBox.h>

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <AzQtComponents/Components/Widgets/ToolBar.h>
#include <QAbstractItemView>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QToolButton>
AZ_POP_DISABLE_WARNING

namespace MaterialCanvas
{
    MaterialCanvasToolBar::MaterialCanvasToolBar(QWidget* parent)
        : QToolBar(parent)
    {
        AzQtComponents::ToolBar::addMainToolBarStyle(this);

        AZStd::intrusive_ptr<MaterialCanvasViewportSettings> viewportSettings =
            AZ::UserSettings::CreateFind<MaterialCanvasViewportSettings>(AZ::Crc32("MaterialCanvasViewportSettings"), AZ::UserSettings::CT_GLOBAL);

        // Add toggle grid button
        m_toggleGrid = addAction(QIcon(":/Icons/grid.svg"), "Toggle Grid");
        m_toggleGrid->setCheckable(true);
        connect(m_toggleGrid, &QAction::triggered, [this]() {
            MaterialCanvasViewportRequestBus::Broadcast(&MaterialCanvasViewportRequestBus::Events::SetGridEnabled, m_toggleGrid->isChecked());
        });
        m_toggleGrid->setChecked(viewportSettings->m_enableGrid);

        // Add toggle shadow catcher button
        m_toggleShadowCatcher = addAction(QIcon(":/Icons/shadow.svg"), "Toggle Shadow Catcher");
        m_toggleShadowCatcher->setCheckable(true);
        connect(m_toggleShadowCatcher, &QAction::triggered, [this]() {
            MaterialCanvasViewportRequestBus::Broadcast(
                &MaterialCanvasViewportRequestBus::Events::SetShadowCatcherEnabled, m_toggleShadowCatcher->isChecked());
        });
        m_toggleShadowCatcher->setChecked(viewportSettings->m_enableShadowCatcher);

        // Add toggle alternate skybox button
        m_toggleAlternateSkybox = addAction(QIcon(":/Icons/skybox.svg"), "Toggle Alternate Skybox");
        m_toggleAlternateSkybox->setCheckable(true);
        connect(m_toggleAlternateSkybox, &QAction::triggered, [this]() {
            MaterialCanvasViewportRequestBus::Broadcast(
                &MaterialCanvasViewportRequestBus::Events::SetAlternateSkyboxEnabled, m_toggleAlternateSkybox->isChecked());
        });
        m_toggleAlternateSkybox->setChecked(viewportSettings->m_enableAlternateSkybox);

        // Add mapping selection button
        QToolButton* toneMappingButton = new QToolButton(this);
        QMenu* toneMappingMenu = new QMenu(toneMappingButton);

        m_operationNames = {
            {AZ::Render::DisplayMapperOperationType::Reinhard, "Reinhard"},
            {AZ::Render::DisplayMapperOperationType::GammaSRGB, "GammaSRGB"},
            {AZ::Render::DisplayMapperOperationType::Passthrough, "Passthrough"},
            {AZ::Render::DisplayMapperOperationType::AcesLut, "AcesLut"},
            {AZ::Render::DisplayMapperOperationType::Aces, "Aces"}};

        for (auto operationNamePair : m_operationNames)
        {
            m_operationActions[operationNamePair.first] = toneMappingMenu->addAction(operationNamePair.second, [operationNamePair]() {
                MaterialCanvasViewportRequestBus::Broadcast(
                    &MaterialCanvasViewportRequestBus::Events::SetDisplayMapperOperationType, operationNamePair.first);
            });
            m_operationActions[operationNamePair.first]->setCheckable(true);
            m_operationActions[operationNamePair.first]->setChecked(
                operationNamePair.first == viewportSettings->m_displayMapperOperationType);
        }

        toneMappingButton->setMenu(toneMappingMenu);
        toneMappingButton->setText("Tone Mapping");
        toneMappingButton->setIcon(QIcon(":/Icons/toneMapping.svg"));
        toneMappingButton->setPopupMode(QToolButton::InstantPopup);
        toneMappingButton->setVisible(true);
        addWidget(toneMappingButton);

        // Add lighting preset combo box
        auto lightingPresetComboBox = new LightingPresetComboBox(this);
        lightingPresetComboBox->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
        lightingPresetComboBox->view()->setMinimumWidth(200);
        addWidget(lightingPresetComboBox);

        // Add model combo box
        auto modelPresetComboBox = new ModelPresetComboBox(this);
        modelPresetComboBox->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
        modelPresetComboBox->view()->setMinimumWidth(200);
        addWidget(modelPresetComboBox);

        MaterialCanvasViewportNotificationBus::Handler::BusConnect();
    }

    MaterialCanvasToolBar::~MaterialCanvasToolBar()
    {
        MaterialCanvasViewportNotificationBus::Handler::BusDisconnect();
    }

    void MaterialCanvasToolBar::OnGridEnabledChanged(bool enable)
    {
        m_toggleGrid->setChecked(enable);
    }

    void MaterialCanvasToolBar::OnAlternateSkyboxEnabledChanged(bool enable)
    {
        m_toggleAlternateSkybox->setChecked(enable);
    }

    void MaterialCanvasToolBar::OnDisplayMapperOperationTypeChanged(AZ::Render::DisplayMapperOperationType operationType)
    {
        for (auto operationActionPair : m_operationActions)
        {
            operationActionPair.second->setChecked(operationActionPair.first == operationType);
        }
    }

    void MaterialCanvasToolBar::OnShadowCatcherEnabledChanged(bool enable)
    {
        m_toggleShadowCatcher->setChecked(enable);
    }

} // namespace MaterialCanvas

#include <Window/ToolBar/moc_MaterialCanvasToolBar.cpp>
