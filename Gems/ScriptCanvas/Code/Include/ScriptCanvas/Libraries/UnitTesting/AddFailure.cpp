/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "AddFailure.h"

#include "UnitTestBus.h"

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace UnitTesting
        {
            void AddFailure::OnInputSignal([[maybe_unused]] const SlotId& slotId)
            {
                const auto report = FindDatum(GetSlotId("Report"))->GetAs<Data::StringType>();
                ScriptCanvas::UnitTesting::Bus::Event(GetOwningScriptCanvasId(), &ScriptCanvas::UnitTesting::BusTraits::AddFailure, *report);

                SignalOutput(GetSlotId("Out"));
            }
        }
    }
}
