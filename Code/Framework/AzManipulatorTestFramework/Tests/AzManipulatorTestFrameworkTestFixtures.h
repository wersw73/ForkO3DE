/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzTest/AzTest.h>
#include <AzToolsFramework/UnitTest/AzToolsFrameworkTestHelpers.h>
#include <AzManipulatorTestFramework/AzManipulatorTestFrameworkUtils.h>

namespace UnitTest
{
    class LinearManipulatorTestFixture
        : public ToolsApplicationFixture
    {
    protected:
        LinearManipulatorTestFixture(const AzToolsFramework::ManipulatorManagerId& manipulatorManagerId)
            : m_manipulatorManagerId(manipulatorManagerId) {}

        void SetUpEditorFixtureImpl() override
        {        
            m_linearManipulator = AzManipulatorTestFramework::CreateLinearManipulator(
                m_manipulatorManagerId,
                /*position=*/AZ::Vector3::CreateZero(),
                /*radius=*/1.0f);

            // default sanity check call backs
            m_linearManipulator->InstallLeftMouseDownCallback(
                [this](const AzToolsFramework::LinearManipulator::Action& /*action*/)
            {
                m_receivedLeftMouseDown = true;
            });

            m_linearManipulator->InstallMouseMoveCallback(
                [this](const AzToolsFramework::LinearManipulator::Action& /*action*/)
            {
                m_receivedMouseMove = true;
            });

            m_linearManipulator->InstallLeftMouseUpCallback(
                [this](const AzToolsFramework::LinearManipulator::Action& /*action*/)
            {
                m_receivedLeftMouseUp = true;
            });
        }

        void TearDownEditorFixtureImpl() override
        {
            m_linearManipulator->Unregister();
            m_linearManipulator.reset();
        }

        const AzToolsFramework::ManipulatorManagerId m_manipulatorManagerId;
        AZStd::shared_ptr<AzToolsFramework::LinearManipulator> m_linearManipulator;

        // sanity flags for manipulator mouse callbacks
        bool m_receivedLeftMouseDown = false;
        bool m_receivedMouseMove = false;
        bool m_receivedLeftMouseUp = false;

        // initial world space starting position for mouse interaction
        const AzToolsFramework::ViewportInteraction::MousePick m_mouseStartingPositionRay =
            AzManipulatorTestFramework::CreateMousePick(
                AZ::Vector3(0.0f, -2.0f, 0.0f), AZ::Vector3(0.0f, 1.0f, 0.0f), AzFramework::ScreenPoint( 0,0 ));

        // left mouse down ray in world space 2 units back from origin looking down +y axis with a null interaction
        // id and no keyboard modifiers
        AzToolsFramework::ViewportInteraction::MouseInteraction m_interaction =
            AzManipulatorTestFramework::CreateMouseInteraction(
                m_mouseStartingPositionRay,
                AzManipulatorTestFramework::CreateMouseButtons(AzToolsFramework::ViewportInteraction::MouseButton::Left),
                AzToolsFramework::ViewportInteraction::InteractionId(AZ::EntityId(0), 0),
                AzToolsFramework::ViewportInteraction::KeyboardModifiers(0));
    };
} // namespace UnitTest
