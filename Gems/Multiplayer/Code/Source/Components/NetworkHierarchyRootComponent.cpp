/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Console/ILogger.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Components/TransformComponent.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/Components/NetworkHierarchyChildComponent.h>
#include <Multiplayer/Components/NetworkHierarchyRootComponent.h>

AZ_CVAR(uint32_t, bg_hierarchyEntityMaxLimit, 16, nullptr, AZ::ConsoleFunctorFlags::Null,
    "Maximum allowed size of network entity hierarchies, including top level entity.");

static constexpr int CommonHierarchyEntityMaxLimit = 16; // Should match @bg_hierarchyEntityMaxLimit

namespace Multiplayer
{
    void NetworkHierarchyRootComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkHierarchyRootComponent, NetworkHierarchyRootComponentBase>()
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<NetworkHierarchyRootComponent>(
                    "Network Hierarchy Root", "Marks the entity as the root of an entity hierarchy.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }
        NetworkHierarchyRootComponentBase::Reflect(context);
    }

    void NetworkHierarchyRootComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkTransformComponent"));
    }

    void NetworkHierarchyRootComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkHierarchyRootComponent"));
    }

    void NetworkHierarchyRootComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkHierarchyChildComponent"));
        incompatible.push_back(AZ_CRC_CE("NetworkHierarchyRootComponent"));
    }

    NetworkHierarchyRootComponent::NetworkHierarchyRootComponent()
        : m_childChangedHandler([this](AZ::ChildChangeType type, AZ::EntityId child) { OnChildChanged(type, child); })
        , m_parentChangedHandler([this](AZ::EntityId oldParent, AZ::EntityId parent) { OnParentChanged(oldParent, parent); })
    {
    }

    void NetworkHierarchyRootComponent::OnInit()
    {
    }

    void NetworkHierarchyRootComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_isHierarchyEnabled = true;
        m_hierarchicalEntities.push_back(GetEntity());

        NetworkHierarchyRequestBus::Handler::BusConnect(GetEntityId());

        if (AzFramework::TransformComponent* transformComponent = GetEntity()->FindComponent<AzFramework::TransformComponent>())
        {
            transformComponent->BindChildChangedEventHandler(m_childChangedHandler);
            transformComponent->BindParentChangedEventHandler(m_parentChangedHandler);
        }
    }

    void NetworkHierarchyRootComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_isHierarchyEnabled = false;

        if (m_rootEntity)
        {
            // Tell parent to re-build the hierarchy
            if (NetworkHierarchyRootComponent* root = m_rootEntity->FindComponent<NetworkHierarchyRootComponent>())
            {
                root->RebuildHierarchy();
            }
        }
        else
        {
            // Notify children that the hierarchy is disbanding
            AZStd::vector<AZ::EntityId> allChildren;
            AZ::TransformBus::EventResult(allChildren, GetEntityId(), &AZ::TransformBus::Events::GetChildren);

            for (const AZ::EntityId& childEntityId : allChildren)
            {
                if (const AZ::Entity* childEntity = AZ::Interface<AZ::ComponentApplicationRequests>::Get()->FindEntity(childEntityId))
                {
                    SetRootForEntity(nullptr, childEntity);
                }
            }
        }

        m_childChangedHandler.Disconnect();
        m_parentChangedHandler.Disconnect();

        NetworkHierarchyRequestBus::Handler::BusDisconnect();

        m_hierarchicalEntities.clear();
        m_rootEntity = nullptr;
    }

    bool NetworkHierarchyRootComponent::IsHierarchyEnabled() const
    {
        return m_isHierarchyEnabled;
    }

    bool NetworkHierarchyRootComponent::IsHierarchicalRoot() const
    {
        return GetHierarchyRoot() == InvalidNetEntityId;
    }

    bool NetworkHierarchyRootComponent::IsHierarchicalChild() const
    {
        return !IsHierarchicalRoot();
    }

    AZStd::vector<AZ::Entity*> NetworkHierarchyRootComponent::GetHierarchicalEntities() const
    {
        return m_hierarchicalEntities;
    }

    AZ::Entity* NetworkHierarchyRootComponent::GetHierarchicalRoot() const
    {
        if (m_rootEntity)
        {
            return m_rootEntity;
        }

        return GetEntity();
    }

    void NetworkHierarchyRootComponent::BindNetworkHierarchyChangedEventHandler(NetworkHierarchyChangedEvent::Handler& handler)
    {
        handler.Connect(m_networkHierarchyChangedEvent);
    }

    void NetworkHierarchyRootComponent::BindNetworkHierarchyLeaveEventHandler(NetworkHierarchyLeaveEvent::Handler& handler)
    {
        handler.Connect(m_networkHierarchyLeaveEvent);
    }

    void NetworkHierarchyRootComponent::OnChildChanged([[maybe_unused]] AZ::ChildChangeType type, [[maybe_unused]] AZ::EntityId child)
    {
        if (IsHierarchicalRoot())
        {
            // Parent-child notifications are not reliable enough to avoid duplicate notifications,
            // so we will rebuild from scratch to avoid duplicate entries in @m_hierarchicalEntities.
            RebuildHierarchy();
        }
        else if (NetworkHierarchyRootComponent* root = GetHierarchicalRoot()->FindComponent<NetworkHierarchyRootComponent>())
        {
            root->RebuildHierarchy();
        }
    }

    static AZStd::tuple<NetworkHierarchyRootComponent*, NetworkHierarchyChildComponent*> GetHierarchyComponents(const AZ::Entity* entity)
    {
        NetworkHierarchyChildComponent* childComponent = nullptr;
        NetworkHierarchyRootComponent* rootComponent = nullptr;

        for (AZ::Component* component : entity->GetComponents())
        {
            if (component->GetUnderlyingComponentType() == NetworkHierarchyChildComponent::TYPEINFO_Uuid())
            {
                childComponent = static_cast<NetworkHierarchyChildComponent*>(component);
                break;
            }

            if (component->GetUnderlyingComponentType() == NetworkHierarchyRootComponent::TYPEINFO_Uuid())
            {
                rootComponent = static_cast<NetworkHierarchyRootComponent*>(component);
                break;
            }
        }

        return AZStd::tie(rootComponent, childComponent);
    }

    void NetworkHierarchyRootComponent::OnParentChanged([[maybe_unused]] AZ::EntityId oldParent, AZ::EntityId newParent)
    {
        // If the parent is part of a hierarchy, it will detect this entity as a new child and rebuild hierarchy.
        // Thus, we only need to take care of a case when the parent is not part of a hierarchy,
        // in which case, this entity will be a new root of a new hierarchy.

        if (AZ::Entity* parentEntity = AZ::Interface<AZ::ComponentApplicationRequests>::Get()->FindEntity(newParent))
        {
            auto [rootComponent, childComponent] = GetHierarchyComponents(parentEntity);
            if (rootComponent == nullptr && childComponent == nullptr)
            {
                RebuildHierarchy();
            }
            else
            {
                m_hierarchicalEntities.clear();
            }
        }
        else
        {
            // Detached from parent
            RebuildHierarchy();
        }
    }

    void NetworkHierarchyRootComponent::RebuildHierarchy()
    {
        AZStd::vector<AZ::Entity*> previousEntities;
        m_hierarchicalEntities.swap(previousEntities);

        m_hierarchicalEntities.reserve(bg_hierarchyEntityMaxLimit);

        InternalBuildHierarchyList(GetEntity());

        bool hierarchyChanged = false;

        // Send out join and leave events.
        for (AZ::Entity* currentEntity : m_hierarchicalEntities)
        {
            const auto prevEntityIterator = AZStd::find(previousEntities.begin(), previousEntities.end(), currentEntity);
            if (prevEntityIterator != previousEntities.end())
            {
                // This entity was here before the build of the hierarchy.
                previousEntities.erase(prevEntityIterator);
            }
            else
            {
                // This is a newly added entity to the network hierarchy.
                hierarchyChanged = true;
                SetRootForEntity(GetEntity(), currentEntity);
            }
        }

        // These entities were removed since last rebuild.
        for (const AZ::Entity* previousEntity : previousEntities)
        {
            SetRootForEntity(nullptr, previousEntity);
        }

        if (!previousEntities.empty())
        {
            hierarchyChanged = true;
        }

        if (hierarchyChanged)
        {
            m_networkHierarchyChangedEvent.Signal(GetEntityId());
        }
    }

    void NetworkHierarchyRootComponent::InternalBuildHierarchyList(AZ::Entity* underEntity)
    {
        AZ::ComponentApplicationRequests* componentApplicationRequests = AZ::Interface<AZ::ComponentApplicationRequests>::Get();

        AZStd::deque<AZ::Entity*, AZStd::allocator, CommonHierarchyEntityMaxLimit> candidates;
        candidates.push_back(underEntity);

        while (!candidates.empty())
        {
            AZ::Entity* candidate = candidates.front();
            candidates.pop_front();

            if (candidate)
            {
                auto [hierarchyRootComponent, hierarchyChildComponent] = GetHierarchyComponents(candidate);

                if ((hierarchyChildComponent && hierarchyChildComponent->IsHierarchyEnabled()) ||
                    (hierarchyRootComponent && hierarchyRootComponent->IsHierarchyEnabled()))
                {
                    m_hierarchicalEntities.push_back(candidate);

                    if (m_hierarchicalEntities.size() >= bg_hierarchyEntityMaxLimit)
                    {
                        AZLOG_WARN("Network hierarchy size exceeded, current limit is %d, root entity was %s",
                            static_cast<int>(bg_hierarchyEntityMaxLimit),
                            GetEntity()->GetName().c_str());
                        return;
                    }

                    const AZStd::vector<AZ::EntityId> allChildren = candidate->GetTransform()->GetChildren();
                    for (const AZ::EntityId& newChildId : allChildren)
                    {
                        candidates.push_back(componentApplicationRequests->FindEntity(newChildId));
                    }
                }
            }
        }
    }

    void NetworkHierarchyRootComponent::SetRootForEntity(AZ::Entity* root, const AZ::Entity* childEntity)
    {
        auto [hierarchyRootComponent, hierarchyChildComponent] = GetHierarchyComponents(childEntity);

        if (hierarchyChildComponent)
        {
            hierarchyChildComponent->SetTopLevelHierarchyRootEntity(root);
        }
        else if (hierarchyRootComponent)
        {
            hierarchyRootComponent->SetTopLevelHierarchyRootEntity(root);
        }
    }

    void NetworkHierarchyRootComponent::SetTopLevelHierarchyRootEntity(AZ::Entity* hierarchyRoot)
    {
        m_rootEntity = hierarchyRoot;

        if (HasController() && GetNetBindComponent()->GetNetEntityRole() == NetEntityRole::Authority)
        {
            NetworkHierarchyChildComponentController* controller = static_cast<NetworkHierarchyChildComponentController*>(GetController());
            if (hierarchyRoot)
            {
                const NetEntityId netRootId = GetNetworkEntityManager()->GetNetEntityIdById(hierarchyRoot->GetId());
                controller->SetHierarchyRoot(netRootId);
            }
            else
            {
                controller->SetHierarchyRoot(InvalidNetEntityId);
            }
        }

        if (m_rootEntity == nullptr)
        {
            // We lost the parent hierarchical entity, so as a root we need to re-build our own hierarchy.
            RebuildHierarchy();
        }
    }
}
