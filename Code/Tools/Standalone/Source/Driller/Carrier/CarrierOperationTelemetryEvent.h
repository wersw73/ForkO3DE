/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#ifndef DRILLER_CARRIER_CARRIEROPERATIONTELEMETRYEVENT_H
#define DRILLER_CARRIER_CARRIEROPERATIONTELEMETRYEVENT_H

#include "Source/Telemetry/TelemetryEvent.h"

namespace Driller
{
    class CarrierOperationTelemetryEvent
        : public Telemetry::TelemetryEvent
    {
    public:
        CarrierOperationTelemetryEvent()
            : Telemetry::TelemetryEvent("CarrierDataViewOperation")
        {
        }
    };
}

#endif
