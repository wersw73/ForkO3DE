/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <FocusedEntity/FocusedMeshEntity.h>

#include <Atom/RPI.Public/Model/Model.h>
#include <Atom/RPI.Public/Model/ModelLodUtils.h>
#include <Atom/RPI.Public/MeshDrawPacket.h>
#include <Atom/RPI.Public/DynamicDraw/DynamicDrawInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AzCore/Component/TransformBus.h>

namespace AZ
{
    namespace Render
    {
        // Gets the view for the specified scene.
        static const RPI::ViewPtr GetViewFromScene(const RPI::Scene* scene)
        {
            const auto viewportContextRequests = RPI::ViewportContextRequests::Get();
            const auto viewportContext = viewportContextRequests->GetViewportContextByScene(scene);
            const RPI::ViewPtr viewPtr = viewportContext->GetDefaultView();
            return viewPtr;
        }

        // Get the world transform for the specified entity.
        static AZ::Transform GetWorldTransformForEntity(EntityId entityId)
        {
            AZ::Transform worldTM;
            AZ::TransformBus::EventResult(worldTM, entityId, &AZ::TransformBus::Events::GetWorldTM);
            return worldTM;
        }

        // Utility class for common drawable data
        class DrawableMetaData
        {
        public:
            DrawableMetaData(EntityId entityId)
            {
                m_scene = RPI::Scene::GetSceneForEntityId(entityId);
                m_view = GetViewFromScene(m_scene);
                m_featureProcessor = m_scene->GetFeatureProcessor<MeshFeatureProcessorInterface>();
            }

            RPI::Scene* GetScene() const
            {
                return m_scene;
            }

            const RPI::ViewPtr GetView() const
            {
                return m_view;
            }

            const MeshFeatureProcessorInterface* GetFeatureProcessor() const
            {
                return m_featureProcessor;
            }

        private:
            RPI::Scene* m_scene = nullptr;
            RPI::ViewPtr m_view = nullptr;
            MeshFeatureProcessorInterface* m_featureProcessor = nullptr;
        };

        FocuseMeshdEntity::FocuseMeshdEntity(EntityId entityId, Data::Instance<RPI::Material> maskMaterial)
            : m_entityId(entityId)
            , m_maskMaterial(maskMaterial)
        {
            AZ::Render::AtomMeshNotificationBus::Handler::BusConnect(m_entityId);
        }
        
        FocuseMeshdEntity::~FocuseMeshdEntity()
        {
            AZ::Render::AtomMeshNotificationBus::Handler::BusDisconnect();
        }

        void FocuseMeshdEntity::ClearDrawData()
        {
            m_modelLodIndex = RPI::ModelLodIndex::Null;
            m_meshDrawPackets.clear();
        }
        
        bool FocuseMeshdEntity::CanDraw() const
        {
            return !m_meshDrawPackets.empty();
        }

        void FocuseMeshdEntity::Draw()
        {
            if (!CanDraw())
            {
                AZ_Warning(
                    "EditorModeFeedbackSystemComponent",
                    false,
                    "Attempted to draw entity '%s' but entity has no draw data!",
                    m_entityId.ToString().c_str());
        
                return;
            }
        
            const DrawableMetaData drawabaleMetaData(m_entityId);

            // If the mesh level of detail index has changed, rebuild the mesh draw packets with the new index
            const auto model = drawabaleMetaData.GetFeatureProcessor()->GetModel(*m_meshHandle);
            if (const auto modelLodIndex = GetModelLodIndex(drawabaleMetaData.GetView(), model);
                m_modelLodIndex != modelLodIndex)
            {
                CreateOrUpdateMeshDrawPackets(drawabaleMetaData.GetFeatureProcessor(), modelLodIndex, model);
            }
        
            AZ::RPI::DynamicDrawInterface* dynamicDraw = AZ::RPI::GetDynamicDraw();
            for (auto& drawPacket : m_meshDrawPackets)
            {
                drawPacket.Update(*drawabaleMetaData.GetScene());
                dynamicDraw->AddDrawPacket(drawabaleMetaData.GetScene(), drawPacket.GetRHIDrawPacket());
            }
        }
         
        RPI::ModelLodIndex FocuseMeshdEntity::GetModelLodIndex(const RPI::ViewPtr view, Data::Instance<RPI::Model> model) const
        {
            const auto worldTM = GetWorldTransformForEntity(m_entityId);
            return RPI::ModelLodUtils::SelectLod(view.get(), worldTM, *model);
        }
        
        void FocuseMeshdEntity::OnAcquireMesh(const MeshFeatureProcessorInterface::MeshHandle* meshHandle)
        {
            m_meshHandle = meshHandle;
            const DrawableMetaData drawabaleMetaData(m_entityId);
            const auto model = drawabaleMetaData.GetFeatureProcessor()->GetModel(*m_meshHandle);
            const auto modelLodIndex = GetModelLodIndex(drawabaleMetaData.GetView(), model);
            CreateOrUpdateMeshDrawPackets(drawabaleMetaData.GetFeatureProcessor(), modelLodIndex, model);
        }
        
        void FocuseMeshdEntity::CreateOrUpdateMeshDrawPackets(
            const MeshFeatureProcessorInterface* featureProcessor, const RPI::ModelLodIndex modelLodIndex, Data::Instance<RPI::Model> model)
        {
            if (!m_meshHandle || !m_meshHandle->IsValid())
            {
                return;
            }
        
            ClearDrawData();

            if (m_meshHandle->IsValid())
            {
                const auto maskMeshObjectSrg = CreateMaskShaderResourceGroup(featureProcessor);
                m_modelLodIndex = modelLodIndex;
                BuildMeshDrawPackets(model->GetModelAsset(), maskMeshObjectSrg);
            }
        }

        void FocuseMeshdEntity::BuildMeshDrawPackets(
            const Data::Asset<RPI::ModelAsset> modelAsset, Data::Instance<RPI::ShaderResourceGroup> meshObjectSrg)
        {
            const Data::Asset<RPI::ModelLodAsset>& modelLodAsset = modelAsset->GetLodAssets()[m_modelLodIndex.m_index];
            Data::Instance<RPI::ModelLod> modelLod = RPI::ModelLod::FindOrCreate(modelLodAsset, modelAsset).get();

            for (size_t i = 0; i < modelLod->GetMeshes().size(); i++)
            {
                RPI::MeshDrawPacket drawPacket(*modelLod, i, m_maskMaterial, meshObjectSrg);
                m_meshDrawPackets.emplace_back(drawPacket);
            }
        }

        Data::Instance<RPI::ShaderResourceGroup> FocuseMeshdEntity::CreateMaskShaderResourceGroup(
            const MeshFeatureProcessorInterface* featureProcessor) const
        {
            const auto& shaderAsset = m_maskMaterial->GetAsset()->GetMaterialTypeAsset()->GetShaderAssetForObjectSrg();
            const auto& objectSrgLayout = m_maskMaterial->GetAsset()->GetObjectSrgLayout();
            const auto maskMeshObjectSrg = RPI::ShaderResourceGroup::Create(shaderAsset, objectSrgLayout->GetName());

            // Set the object id so the correct MVP matrices can be selected in the shader
            const auto objectId = featureProcessor->GetObjectId(*m_meshHandle).GetIndex();
            RHI::ShaderInputNameIndex objectIdIndex = "m_objectId";
            maskMeshObjectSrg->SetConstant(objectIdIndex, objectId);
            maskMeshObjectSrg->Compile();

            return maskMeshObjectSrg;
        }
    } // namespace Render
} // namespace AZ
