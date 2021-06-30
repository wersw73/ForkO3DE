/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>

class QGraphicsLayout;
class QGraphicsLayoutItem;

namespace GraphCanvas
{
    static const AZ::Crc32 NodeLayoutServiceCrc = AZ_CRC("GraphCanvas_NodeLayoutService", 0x3dc121b7);
    static const AZ::Crc32 NodeSlotsServiceCrc = AZ_CRC("GraphCanvas_NodeSlotsService", 0x28f0a117);
    static const AZ::Crc32 NodeLayoutSupportServiceCrc = AZ_CRC("GraphCanvas_NodeLayoutSupportService", 0xa8b639be);

    //! NodeLayoutRequests
    //! Requests that are serviced by a node layout implementation.
    class NodeLayoutRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        
        //! Obtain the layout component as a \code QGraphicsLayout*. 
        virtual QGraphicsLayout* GetLayout() { return{}; }
    };

    using NodeLayoutRequestBus = AZ::EBus<NodeLayoutRequests>;

    //! NodeSlotRequestBus
    //! Used for making requests of a particular slot.
    class NodeSlotsRequests
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual QGraphicsLayoutItem* GetGraphicsLayoutItem() = 0;
    };

    using NodeSlotsRequestBus = AZ::EBus<NodeSlotsRequests>;
}
