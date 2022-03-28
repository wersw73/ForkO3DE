/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzFramework/Spawnable/SpawnableMediator.h>
#include <ScriptCanvas/Libraries/Spawning/CreateSpawnTicketNodeable.h>

namespace ScriptCanvas::Nodeables::Spawning
{
    CreateSpawnTicketNodeable::CreateSpawnTicketNodeable([[maybe_unused]] const CreateSpawnTicketNodeable& rhs)
    {
        // this method is required by Script Canvas, left intentionally blank to avoid copying m_spawnableMediator
    }

    CreateSpawnTicketNodeable& CreateSpawnTicketNodeable::operator=([[maybe_unused]] const CreateSpawnTicketNodeable& rhs)
    {
        // this method is required by Script Canvas, left intentionally blank to avoid copying m_spawnableMediator
        return *this;
    }

    AzFramework::EntitySpawnTicket CreateSpawnTicketNodeable::CreateTicket(const AzFramework::SpawnableAssetRef& prefab)
    {
        using namespace AzFramework;
        
        return m_spawnableMediator.CreateSpawnTicket(prefab);
    }
}
