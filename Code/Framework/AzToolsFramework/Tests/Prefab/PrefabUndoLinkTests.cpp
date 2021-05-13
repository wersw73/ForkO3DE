/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <Prefab/PrefabTestDomUtils.h>
#include <Prefab/PrefabTestUndoFixture.h>

#include <Prefab/PrefabUndo.h>

#include <Prefab/PrefabTestComponent.h>

namespace UnitTest
{
    using PrefabUndoLinkTests = PrefabTestUndoFixture;

    TEST_F(PrefabUndoLinkTests, PrefabUndoLink_Add)
    {
        AZStd::unique_ptr<Instance> firstInstance = nullptr;
        AZStd::unique_ptr<Instance> secondInstance = nullptr;
        TemplateId firstTemplateId = InvalidTemplateId;
        TemplateId secondTemplateId = InvalidTemplateId;

        SetupInstances(firstInstance, secondInstance, firstTemplateId, secondTemplateId);

        firstInstance->AddInstance(AZStd::move(secondInstance));
        AZStd::vector<InstanceAlias> aliases = firstInstance->GetNestedInstanceAliases(secondTemplateId);

        //parent prefab2 to prefab 1 by creating a link
        //capture the link addition in undo node
        PrefabUndoInstanceLink undoLink("Undo Link Add Node");
        undoLink.Capture(firstTemplateId, secondTemplateId, aliases[0]);
        undoLink.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        AZStd::unique_ptr<Instance> newInstance = m_prefabSystemComponent->InstantiatePrefab(firstTemplateId);
        AZStd::vector<InstanceAlias> instances = newInstance->GetNestedInstanceAliases(secondTemplateId);
        
        ASSERT_TRUE(instances.size() > 0);
        instances.clear();

        //undo the parenting
        undoLink.Undo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        newInstance = m_prefabSystemComponent->InstantiatePrefab(firstTemplateId);
        instances = newInstance->GetNestedInstanceAliases(secondTemplateId);

        ASSERT_TRUE(instances.size() == 0);

        //undo the parenting
        undoLink.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        newInstance = m_prefabSystemComponent->InstantiatePrefab(firstTemplateId);
        instances = newInstance->GetNestedInstanceAliases(secondTemplateId);

        ASSERT_TRUE(instances.size() == 1);
    }

    TEST_F(PrefabUndoLinkTests, PrefabUndoLink_InitialPatchData_ContainerTarget_PatchSucceeds)
    {
        //create two instances
        AZStd::unique_ptr<Instance> nestedInstance = nullptr;
        AZStd::unique_ptr<Instance> rootInstance = nullptr;
        TemplateId nestedTemplateId = InvalidTemplateId;
        TemplateId rootTemplateId = InvalidTemplateId;

        SetupInstances(nestedInstance, rootInstance, nestedTemplateId, rootTemplateId);
        nestedInstance->ActivateContainerEntity();

        AZ::EntityId nestedContainerEntityId = nestedInstance->GetContainerEntityId();
        AZ::EntityId rootContainerEntityId = rootInstance->GetContainerEntityId();

        AZ::Entity* nestedContainerEntity = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        //generate a patch to add a component to the nested instance
        PrefabDom initialEntityDom;
        m_instanceToTemplateInterface->GenerateDomForEntity(initialEntityDom, *nestedContainerEntity);

        nestedContainerEntity->Deactivate();
        PrefabTestComponent* nestedTestComponent = nestedContainerEntity->CreateComponent<PrefabTestComponent>();
        ASSERT_TRUE(nestedTestComponent);
        nestedContainerEntity->Activate();

        PrefabDom modifiedEntityDom;
        m_instanceToTemplateInterface->GenerateDomForEntity(modifiedEntityDom, *nestedContainerEntity);

        PrefabDom patch;
        m_instanceToTemplateInterface->GeneratePatch(patch, initialEntityDom, modifiedEntityDom);
        m_instanceToTemplateInterface->AppendEntityAliasToPatchPaths(patch, nestedContainerEntityId);

        //apply the patch
        PrefabDom& templateDomReference = m_prefabSystemComponent->FindTemplateDom(nestedTemplateId);
        AZ::JsonSerializationResult::ResultCode result = AZ::JsonSerialization::ApplyPatch(templateDomReference,
            templateDomReference.GetAllocator(), patch, AZ::JsonMergeApproach::JsonPatch);

        AZ_Error("Prefab", result.GetOutcome() == AZ::JsonSerializationResult::Outcomes::Success,
            "Patch was not successfully applied");

        //nest the second instance under the first instance
        rootInstance->AddInstance(AZStd::move(nestedInstance));
        AZStd::vector<InstanceAlias> aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);

        //create patch for nesting
        m_instanceToTemplateInterface->GenerateDomForEntity(initialEntityDom, *nestedContainerEntity);
        nestedTestComponent->m_entityIdProperty = rootContainerEntityId;
        m_instanceToTemplateInterface->GenerateDomForEntity(modifiedEntityDom, *nestedContainerEntity);
        m_instanceToTemplateInterface->GeneratePatch(patch, initialEntityDom, modifiedEntityDom);
        m_instanceToTemplateInterface->AppendEntityAliasToPatchPaths(patch, nestedContainerEntityId);

        //create an undo node to apply the patch and prep for undo
        PrefabUndoInstanceLink undoInstanceLinkNode("Undo Link Patch");
        undoInstanceLinkNode.Capture(rootTemplateId, nestedTemplateId, aliases[0], AZStd::move(patch), InvalidLinkId);
        undoInstanceLinkNode.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        //verify the application worked
        InstanceOptionalReference nestedInstanceRef = rootInstance->FindNestedInstance(aliases[0]);
        AZ::EntityId nestedContainerId = nestedInstanceRef->get().GetContainerEntityId();

        AZ::Entity* nestedContainer = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(nestedContainer, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerId);
        ASSERT_TRUE(nestedContainer);

        PrefabTestComponent* nestedComponent = nestedContainer->FindComponent<PrefabTestComponent>();
        ASSERT_EQ(nestedComponent->m_entityIdProperty, rootInstance->GetContainerEntityId());
    }

    TEST_F(PrefabUndoLinkTests, PrefabUndoLink_InitialPatchData_UpdateLinkEntity_PatchSucceeds)
    {
        //create two instance
        AZStd::unique_ptr<Instance> nestedInstance = nullptr;
        AZStd::unique_ptr<Instance> rootInstance = nullptr;
        TemplateId nestedTemplateId = InvalidTemplateId;
        TemplateId rootTemplateId = InvalidTemplateId;

        SetupInstances(nestedInstance, rootInstance, nestedTemplateId, rootTemplateId);
        nestedInstance->ActivateContainerEntity();

        AZ::EntityId nestedContainerEntityId = nestedInstance->GetContainerEntityId();
        AZ::EntityId rootContainerEntityId = rootInstance->GetContainerEntityId();

        AZ::Entity* nestedContainerEntity = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        //add a component for testing to the instance that will be nested
        PrefabDom initialEntityDom;
        m_instanceToTemplateInterface->GenerateDomForEntity(initialEntityDom, *nestedContainerEntity);

        nestedContainerEntity->Deactivate();
        PrefabTestComponent* nestedTestComponent = nestedContainerEntity->CreateComponent<PrefabTestComponent>();
        ASSERT_TRUE(nestedTestComponent);
        nestedContainerEntity->Activate();

        PrefabDom modifiedEntityDom;
        m_instanceToTemplateInterface->GenerateDomForEntity(modifiedEntityDom, *nestedContainerEntity);

        //create patch
        PrefabDom patch;
        m_instanceToTemplateInterface->GeneratePatch(patch, initialEntityDom, modifiedEntityDom);
        m_instanceToTemplateInterface->AppendEntityAliasToPatchPaths(patch, nestedContainerEntityId);
        m_instanceToTemplateInterface->PatchTemplate(patch, nestedTemplateId);
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        //instantiate a new nested instance
        nestedInstance = m_prefabSystemComponent->InstantiatePrefab(nestedTemplateId);
        nestedContainerEntityId = nestedInstance->GetContainerEntityId();
        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        //nest the second instance under the first instance
        m_instanceToTemplateInterface->GenerateDomForEntity(initialEntityDom, *nestedContainerEntity);

        rootInstance->AddInstance(AZStd::move(nestedInstance));
        nestedTestComponent->m_entityIdProperty = rootContainerEntityId;

        m_instanceToTemplateInterface->GenerateDomForEntity(modifiedEntityDom, *nestedContainerEntity);

        //create patch
        PrefabDom linkPatch;
        m_instanceToTemplateInterface->GeneratePatch(linkPatch, initialEntityDom, modifiedEntityDom);
        m_instanceToTemplateInterface->AppendEntityAliasToPatchPaths(linkPatch, nestedContainerEntityId);

        AZStd::vector<InstanceAlias> aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);

        //create an undo node to apply the patch and prep for undo
        PrefabUndoInstanceLink undoInstanceLinkNode("Undo Link Patch");
        undoInstanceLinkNode.Capture(rootTemplateId, nestedTemplateId, aliases[0], AZStd::move(linkPatch), InvalidLinkId);
        undoInstanceLinkNode.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        LinkId linkId = undoInstanceLinkNode.GetLinkId();

        rootInstance = m_prefabSystemComponent->InstantiatePrefab(rootTemplateId);
        aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);

        //verify the link was created
        InstanceOptionalReference nestedInstanceRef = rootInstance->FindNestedInstance(aliases[0]);
        nestedContainerEntityId = nestedInstanceRef->get().GetContainerEntityId();

        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        //update the property on the nested component
        m_instanceToTemplateInterface->GenerateDomForEntity(initialEntityDom, *nestedContainerEntity);

        nestedTestComponent = nestedContainerEntity->FindComponent<PrefabTestComponent>();
        nestedTestComponent->m_boolProperty = true;

        m_instanceToTemplateInterface->GenerateDomForEntity(modifiedEntityDom, *nestedContainerEntity);

        //create patch
        PrefabDom updatePatch;
        m_instanceToTemplateInterface->GeneratePatch(updatePatch, initialEntityDom, modifiedEntityDom);
        m_instanceToTemplateInterface->AppendEntityAliasToPatchPaths(updatePatch, nestedContainerEntityId);

        //create the update link undo/redo node
        PrefabUndoLinkUpdate undoLinkUpdateNode("Undo Link Update");
        undoLinkUpdateNode.Capture(updatePatch, linkId);
        undoLinkUpdateNode.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        //verify the update worked
        rootInstance = m_prefabSystemComponent->InstantiatePrefab(rootTemplateId);
        aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);
        nestedInstanceRef = rootInstance->FindNestedInstance(aliases[0]);
        nestedContainerEntityId = nestedInstanceRef->get().GetContainerEntityId();

        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        nestedTestComponent = nestedContainerEntity->FindComponent<PrefabTestComponent>();
        ASSERT_EQ(nestedTestComponent->m_boolProperty, true);

        //undo the update
        undoLinkUpdateNode.Undo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        //verify the undo update worked
        rootInstance = m_prefabSystemComponent->InstantiatePrefab(rootTemplateId);
        aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);
        nestedInstanceRef = rootInstance->FindNestedInstance(aliases[0]);
        nestedContainerEntityId = nestedInstanceRef->get().GetContainerEntityId();

        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        nestedTestComponent = nestedContainerEntity->FindComponent<PrefabTestComponent>();
        ASSERT_EQ(nestedTestComponent->m_boolProperty, false);

        //redo the update so we can test if updating previously changed values matter
        undoLinkUpdateNode.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        rootInstance = m_prefabSystemComponent->InstantiatePrefab(rootTemplateId);
        aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);
        nestedInstanceRef = rootInstance->FindNestedInstance(aliases[0]);
        nestedContainerEntityId = nestedInstanceRef->get().GetContainerEntityId();

        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        //update the property on the nested component
        m_instanceToTemplateInterface->GenerateDomForEntity(initialEntityDom, *nestedContainerEntity);

        nestedTestComponent = nestedContainerEntity->FindComponent<PrefabTestComponent>();
        nestedTestComponent->m_intProperty = 1;

        m_instanceToTemplateInterface->GenerateDomForEntity(modifiedEntityDom, *nestedContainerEntity);

        //create patch
        PrefabDom updatePatchIntField;
        m_instanceToTemplateInterface->GeneratePatch(updatePatchIntField, initialEntityDom, modifiedEntityDom);
        m_instanceToTemplateInterface->AppendEntityAliasToPatchPaths(updatePatchIntField, nestedContainerEntityId);

        //create the update link undo/redo node
        PrefabUndoLinkUpdate undoIntFieldNode("Undo Link Update");
        undoIntFieldNode.Capture(updatePatchIntField, linkId);
        undoIntFieldNode.Redo();
        m_instanceUpdateExecutorInterface->UpdateTemplateInstancesInQueue();

        //verify the update worked
        rootInstance = m_prefabSystemComponent->InstantiatePrefab(rootTemplateId);
        aliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);
        nestedInstanceRef = rootInstance->FindNestedInstance(aliases[0]);
        nestedContainerEntityId = nestedInstanceRef->get().GetContainerEntityId();

        AZ::ComponentApplicationBus::BroadcastResult(nestedContainerEntity, &AZ::ComponentApplicationBus::Events::FindEntity, nestedContainerEntityId);
        ASSERT_TRUE(nestedContainerEntity);

        nestedTestComponent = nestedContainerEntity->FindComponent<PrefabTestComponent>();
        ASSERT_EQ(nestedTestComponent->m_intProperty, 1);
    }
} // namespace UnitTest
