/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/Entity.h>

#include <gmock/gmock.h>
#include <LyShine/Bus/UiDynamicScrollBoxBus.h>

// the following was generated using google's python script to autogenerate mocks.
// however, it needed some hand-editing to make it work, so if you add functions to IRenderer,
// it will probably be better to just manually add them here than try to run the script again
// hand-edits are marked with 'hand-edit'.  Everything else was autogenerated.

using ::testing::_;

class UiDynamicScrollBoxDataBusHandlerMock
    : public AZ::Component
    , public UiDynamicScrollBoxDataBus::Handler

{
public:
    AZ_COMPONENT(UiDynamicScrollBoxDataBusHandlerMock, "{5CBD9592-FD20-492B-BC24-4AEA8935AEA4}")

    // Just a mock object, no reflection necessary
    static void Reflect(AZ::ReflectContext*) {}

    void Activate() override
    {
        UiDynamicScrollBoxDataBus::Handler::BusConnect(GetEntityId());
    }

    void Deactivate() override
    {
        UiDynamicScrollBoxDataBus::Handler::BusDisconnect();
    }


    MOCK_METHOD0(GetNumElements, int ());
    MOCK_METHOD1(GetElementWidth, float (int));
    MOCK_METHOD1(GetElementHeight, float (int));
    MOCK_METHOD0(GetNumSections, int ());
};
