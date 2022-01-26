/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RPI.Edit/Material/MaterialSourceData.h>
#include <Atom/RPI.Edit/Material/MaterialPropertyValueSerializer.h>
#include <Atom/RPI.Edit/Material/MaterialTypeSourceData.h>
#include <Atom/RPI.Edit/Material/MaterialPropertyId.h>
#include <Atom/RPI.Edit/Material/MaterialUtils.h>
#include <Atom/RPI.Edit/Material/MaterialConverterBus.h>

#include <Atom/RPI.Edit/Common/AssetUtils.h>
#include <Atom/RPI.Edit/Common/JsonFileLoadContext.h>
#include <Atom/RPI.Edit/Common/JsonReportingHelper.h>
#include <Atom/RPI.Edit/Common/JsonUtils.h>

#include <Atom/RPI.Reflect/Material/MaterialAssetCreator.h>
#include <Atom/RPI.Reflect/Material/MaterialPropertiesLayout.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>

#include <AzCore/Serialization/Json/JsonUtils.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Json/JsonSerialization.h>
#include <AzCore/IO/TextStreamWriters.h>
#include <AzCore/IO/GenericStreams.h>
#include <AzCore/IO/IOUtils.h>
#include <AzCore/JSON/prettywriter.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>

#include <AzToolsFramework/API/EditorAssetSystemAPI.h>

namespace AZ
{
    namespace RPI
    {
        void MaterialSourceData::Reflect(ReflectContext* context)
        {
            if (JsonRegistrationContext* jsonContext = azrtti_cast<JsonRegistrationContext*>(context))
            {
                jsonContext->Serializer<JsonMaterialPropertyValueSerializer>()->HandlesType<MaterialSourceData::Property>();
            }
            else if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<MaterialSourceData>()
                    ->Version(2)
                    ->Field("description", &MaterialSourceData::m_description)
                    ->Field("materialType", &MaterialSourceData::m_materialType)
                    ->Field("materialTypeVersion", &MaterialSourceData::m_materialTypeVersion)
                    ->Field("parentMaterial", &MaterialSourceData::m_parentMaterial)
                    ->Field("properties", &MaterialSourceData::m_properties)
                    ;

                serializeContext->RegisterGenericType<PropertyMap>();
                serializeContext->RegisterGenericType<PropertyGroupMap>();

                serializeContext->Class<MaterialSourceData::Property>()
                    ->Version(1)
                    ;
            }
        }

        // Helper function for CreateMaterialAsset, for applying basic material property values
        template<typename T>
        void ApplyMaterialValues(MaterialAssetCreator& materialAssetCreator, const AZStd::map<Name, T>& values)
        {
            for (auto& entry : values)
            {
                const Name& propertyId = entry.first;
                materialAssetCreator.SetPropertyValue(propertyId, entry.second);
            }
        }
        
        Outcome<Data::Asset<MaterialAsset>> MaterialSourceData::CreateMaterialAsset(
            Data::AssetId assetId, AZStd::string_view materialSourceFilePath, MaterialAssetProcessingMode processingMode, bool elevateWarnings) const
        {
            MaterialAssetCreator materialAssetCreator;
            materialAssetCreator.SetElevateWarnings(elevateWarnings);

            if (m_materialType.empty())
            {
                AZ_Error("MaterialSourceData", false, "materialType was not specified");
                return Failure();
            }

            Outcome<Data::AssetId> materialTypeAssetId = AssetUtils::MakeAssetId(materialSourceFilePath, m_materialType, 0);
            if (!materialTypeAssetId)
            {
                return Failure();
            }

            Data::Asset<MaterialTypeAsset> materialTypeAsset;
            
            switch (processingMode)
            {
                case MaterialAssetProcessingMode::DeferredBake:
                {
                     // Don't load the material type data, just create a reference to it
                     materialTypeAsset = Data::Asset<MaterialTypeAsset>{ materialTypeAssetId.GetValue(), azrtti_typeid<MaterialTypeAsset>(), m_materialType };
                     break;
                }
                case MaterialAssetProcessingMode::PreBake:
                {
                    // In this case we need to load the material type data in preparation for the material->Finalize() step below.
                    auto materialTypeAssetOutcome = AssetUtils::LoadAsset<MaterialTypeAsset>(materialTypeAssetId.GetValue());
                    if (!materialTypeAssetOutcome)
                    {
                        return Failure();
                    }
                    materialTypeAsset = materialTypeAssetOutcome.GetValue();
                    break;
                }
                default:
                {
                    AZ_Assert(false, "Unhandled MaterialAssetProcessingMode");
                    return Failure();
                }
            }

            materialAssetCreator.Begin(assetId, materialTypeAsset, processingMode == MaterialAssetProcessingMode::PreBake);

            if (!m_parentMaterial.empty())
            {
                auto parentMaterialAsset = AssetUtils::LoadAsset<MaterialAsset>(materialSourceFilePath, m_parentMaterial);
                if (!parentMaterialAsset.IsSuccess())
                {
                    return Failure();
                }

                // Make sure the parent material has the same material type
                {
                    Data::AssetId parentsMaterialTypeId = parentMaterialAsset.GetValue()->GetMaterialTypeAsset().GetId();

                    if (materialTypeAssetId.GetValue() != parentsMaterialTypeId)
                    {
                        AZ_Error("MaterialSourceData", false, "This material and its parent material do not share the same material type.");
                        return Failure();
                    }
                }

                // Inherit the parent's property values...
                switch (processingMode)
                {
                    case MaterialAssetProcessingMode::DeferredBake:
                    {
                        for (auto& property : parentMaterialAsset.GetValue()->GetRawPropertyValues())
                        {
                            materialAssetCreator.SetPropertyValue(property.first, property.second);
                        }

                        break;
                    }
                    case MaterialAssetProcessingMode::PreBake:
                    {
                        const MaterialPropertiesLayout* propertiesLayout = parentMaterialAsset.GetValue()->GetMaterialPropertiesLayout();

                        if (parentMaterialAsset.GetValue()->GetPropertyValues().size() != propertiesLayout->GetPropertyCount())
                        {
                            AZ_Assert(false, "The parent material should have been finalized with %zu properties but it has %zu. Something is out of sync.",
                                propertiesLayout->GetPropertyCount(), parentMaterialAsset.GetValue()->GetPropertyValues().size());
                            return Failure();
                        }

                        for (size_t propertyIndex = 0; propertyIndex < propertiesLayout->GetPropertyCount(); ++propertyIndex)
                        {
                            materialAssetCreator.SetPropertyValue(
                                propertiesLayout->GetPropertyDescriptor(MaterialPropertyIndex{propertyIndex})->GetName(),
                                parentMaterialAsset.GetValue()->GetPropertyValues()[propertyIndex]);
                        }

                        break;
                    }
                    default:
                    {
                        AZ_Assert(false, "Unhandled MaterialAssetProcessingMode");
                        return Failure();
                    }
                }
            }

            ApplyPropertiesToAssetCreator(materialAssetCreator, materialSourceFilePath);

            Data::Asset<MaterialAsset> material;
            if (materialAssetCreator.End(material))
            {
                return Success(material);
            }
            else
            {
                return Failure();
            }
        }

        Outcome<Data::Asset<MaterialAsset>> MaterialSourceData::CreateMaterialAssetFromSourceData(
            Data::AssetId assetId,
            AZStd::string_view materialSourceFilePath,
            bool elevateWarnings,
            AZStd::unordered_set<AZStd::string>* sourceDependencies) const
        {
            if (m_materialType.empty())
            {
                AZ_Error("MaterialSourceData", false, "materialType was not specified");
                return Failure();
            }

            const auto materialTypeSourcePath = AssetUtils::ResolvePathReference(materialSourceFilePath, m_materialType);
            const auto materialTypeAssetId = AssetUtils::MakeAssetId(materialTypeSourcePath, 0);
            if (!materialTypeAssetId.IsSuccess())
            {
                AZ_Error("MaterialSourceData", false, "Failed to create material type asset ID: '%s'.", materialTypeSourcePath.c_str());
                return Failure();
            }

            MaterialTypeSourceData materialTypeSourceData;
            if (!AZ::RPI::JsonUtils::LoadObjectFromFile(materialTypeSourcePath, materialTypeSourceData))
            {
                AZ_Error("MaterialSourceData", false, "Failed to load MaterialTypeSourceData: '%s'.", materialTypeSourcePath.c_str());
                return Failure();
            }

            materialTypeSourceData.ResolveUvEnums();

            const auto materialTypeAsset =
                materialTypeSourceData.CreateMaterialTypeAsset(materialTypeAssetId.GetValue(), materialTypeSourcePath, elevateWarnings);
            if (!materialTypeAsset.IsSuccess())
            {
                AZ_Error("MaterialSourceData", false, "Failed to create material type asset from source data: '%s'.", materialTypeSourcePath.c_str());
                return Failure();
            }

            // Track all of the material and material type assets loaded while trying to create a material asset from source data. This will
            // be used for evaluating circular dependencies and returned for external monitoring or other use.
            AZStd::unordered_set<AZStd::string> dependencies;
            dependencies.insert(materialSourceFilePath);
            dependencies.insert(materialTypeSourcePath);

            // Load and build a stack of MaterialSourceData from all of the parent materials in the hierarchy. Properties from the source
            // data will be applied in reverse to the asset creator.
            AZStd::vector<MaterialSourceData> parentSourceDataStack;

            AZStd::string parentSourceRelPath = m_parentMaterial;
            AZStd::string parentSourceAbsPath = AssetUtils::ResolvePathReference(materialSourceFilePath, parentSourceRelPath);
            while (!parentSourceRelPath.empty())
            {
                if (!dependencies.insert(parentSourceAbsPath).second)
                {
                    AZ_Error("MaterialSourceData", false, "Detected circular dependency between materials: '%s' and '%s'.", materialSourceFilePath.data(), parentSourceAbsPath.c_str());
                    return Failure();
                }

                MaterialSourceData parentSourceData;
                if (!AZ::RPI::JsonUtils::LoadObjectFromFile(parentSourceAbsPath, parentSourceData))
                {
                    AZ_Error("MaterialSourceData", false, "Failed to load MaterialSourceData for parent material: '%s'.", parentSourceAbsPath.c_str());
                    return Failure();
                }

                // Make sure that all materials in the hierarchy share the same material type
                const auto parentTypeAssetId = AssetUtils::MakeAssetId(parentSourceAbsPath, parentSourceData.m_materialType, 0);
                if (!parentTypeAssetId)
                {
                    AZ_Error("MaterialSourceData", false, "Parent material asset ID wasn't found: '%s'.", parentSourceAbsPath.c_str());
                    return Failure();
                }

                if (parentTypeAssetId.GetValue() != materialTypeAssetId.GetValue())
                {
                    AZ_Error("MaterialSourceData", false, "This material and its parent material do not share the same material type.");
                    return Failure();
                }

                // Get the location of the next parent material and push the source data onto the stack 
                parentSourceRelPath = parentSourceData.m_parentMaterial;
                parentSourceAbsPath = AssetUtils::ResolvePathReference(parentSourceAbsPath, parentSourceRelPath);
                parentSourceDataStack.emplace_back(AZStd::move(parentSourceData));
            }
            
            // Unlike CreateMaterialAsset(), we can always finalize the material here because we loaded created the MaterialTypeAsset from
            // the source .materialtype file, so the necessary data is always available.
            // (In case you are wondering why we don't use CreateMaterialAssetFromSourceData in MaterialBuilder: that would require a
            // source dependency between the .materialtype and .material file, which would cause all .material files to rebuild when you
            // edit the .materialtype; it's faster to not read the material type data at all ... until it's needed at runtime)
            const bool finalize = true;

            // Create the material asset from all the previously loaded source data 
            MaterialAssetCreator materialAssetCreator;
            materialAssetCreator.SetElevateWarnings(elevateWarnings);
            materialAssetCreator.Begin(assetId, materialTypeAsset.GetValue(), finalize);

            while (!parentSourceDataStack.empty())
            {
                parentSourceDataStack.back().ApplyPropertiesToAssetCreator(materialAssetCreator, materialSourceFilePath);
                parentSourceDataStack.pop_back();
            }

            ApplyPropertiesToAssetCreator(materialAssetCreator, materialSourceFilePath);

            Data::Asset<MaterialAsset> material;
            if (materialAssetCreator.End(material))
            {
                if (sourceDependencies)
                {
                    sourceDependencies->insert(dependencies.begin(), dependencies.end());
                }

                return Success(material);
            }

            return Failure();
        }

        void MaterialSourceData::ApplyPropertiesToAssetCreator(
            AZ::RPI::MaterialAssetCreator& materialAssetCreator, const AZStd::string_view& materialSourceFilePath) const
        {
            for (auto& group : m_properties)
            {
                for (auto& property : group.second)
                {
                    MaterialPropertyId propertyId{ group.first, property.first };
                    if (!property.second.m_value.IsValid())
                    {
                        materialAssetCreator.ReportWarning("Source data for material property value is invalid.");
                    }
                    // If the source value type is a string, there are two possible property types: Image and Enum. If there is a "." in
                    // the string (for the extension) we assume it's an Image and look up the referenced Asset. Otherwise, we can assume
                    // it's an Enum value and just preserve the original string.
                    else if (property.second.m_value.Is<AZStd::string>() && AzFramework::StringFunc::Contains(property.second.m_value.GetValue<AZStd::string>(), "."))
                    {
                        Data::Asset<ImageAsset> imageAsset;

                        MaterialUtils::GetImageAssetResult result = MaterialUtils::GetImageAssetReference(
                            imageAsset, materialSourceFilePath, property.second.m_value.GetValue<AZStd::string>());
                                    
                        if (result == MaterialUtils::GetImageAssetResult::Missing)
                        {
                            materialAssetCreator.ReportWarning(
                                "Material property '%s': Could not find the image '%s'", propertyId.GetCStr(),
                                property.second.m_value.GetValue<AZStd::string>().data());
                        }
                                    
                        imageAsset.SetAutoLoadBehavior(Data::AssetLoadBehavior::PreLoad);
                        materialAssetCreator.SetPropertyValue(propertyId, imageAsset);
                    }
                    else
                    {
                        materialAssetCreator.SetPropertyValue(propertyId, property.second.m_value);
                    }
                }
            }
        }

    } // namespace RPI
} // namespace AZ
