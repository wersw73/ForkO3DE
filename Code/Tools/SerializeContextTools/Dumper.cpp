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

#include <Dumper.h> // Moved to the top because AssetSerializer requires include for the SerializeContext
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Casting/lossy_cast.h>
#include <AzCore/Debug/Trace.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/IO/SystemFile.h>
#include <AzCore/JSON/stringbuffer.h>
#include <AzCore/JSON/prettywriter.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/sort.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <Application.h>
#include <Utilities.h>

namespace AZ::SerializeContextTools
{
    bool Dumper::DumpFiles(Application& application)
    {
        SerializeContext* sc = application.GetSerializeContext();
        if (!sc)
        {
            AZ_Error("SerializeContextTools", false, "No serialize context found.");
            return false;
        }

        AZStd::string outputFolder = Utilities::ReadOutputTargetFromCommandLine(application);

        AZ::IO::Path sourceGameFolder;
        if (auto settingsRegistry = AZ::SettingsRegistry::Get(); settingsRegistry != nullptr)
        {
            settingsRegistry->Get(sourceGameFolder.Native(), AZ::SettingsRegistryMergeUtils::FilePathKey_ProjectPath);
        }
        bool result = true;

        AZStd::vector<AZStd::string> fileList = Utilities::ReadFileListFromCommandLine(application, "files");
        for (const AZStd::string& filePath : fileList)
        {
            AZ_Printf("DumpFiles", "Dumping file '%.*s'\n", aznumeric_cast<int>(filePath.size()), filePath.data());

            AZ::IO::FixedMaxPath outputPath{ AZStd::string_view{ outputFolder }};

            outputPath /= AZ::IO::FixedMaxPath(filePath).LexicallyRelative(sourceGameFolder);
            outputPath.Native() += ".dump.txt";

            IO::SystemFile outputFile;
            if (!outputFile.Open(outputPath.c_str(),
                IO::SystemFile::OpenMode::SF_OPEN_CREATE |
                IO::SystemFile::OpenMode::SF_OPEN_CREATE_PATH |
                IO::SystemFile::OpenMode::SF_OPEN_WRITE_ONLY))
            {
                AZ_Error("SerializeContextTools", false, "Unable to open file '%s' for writing.", outputPath.c_str());
                result = false;
                continue;
            }

            AZStd::string content;
            content.reserve(1 * 1024 * 1024); // Reserve 1mb to avoid frequently resizing the string.

            auto callback = [&content, &result](void* classPtr, const Uuid& classId, SerializeContext* context)
            {
                result = DumpClassContent(content, classPtr, classId, context) && result;

                const SerializeContext::ClassData* classData = context->FindClassData(classId);
                if (classData && classData->m_factory)
                {
                    classData->m_factory->Destroy(classPtr);
                }
                else
                {
                    AZ_Error("SerializeContextTools", false, "Missing class factory, so data will leak.");
                    result = false;
                }
            };
            if (!Utilities::InspectSerializedFile(filePath, sc, callback))
            {
                result = false;
                continue;
            }

            outputFile.Write(content.data(), content.length());
        }

        return result;
    }

    bool Dumper::DumpSerializeContext(Application& application)
    {
        AZStd::string outputPath = Utilities::ReadOutputTargetFromCommandLine(application, "SerializeContext.json");
        AZ_Printf("dumpsc", "Writing Serialize Context at '%s'.\n", outputPath.c_str());

        IO::SystemFile outputFile;
        if (!outputFile.Open(outputPath.c_str(),
            IO::SystemFile::OpenMode::SF_OPEN_CREATE |
            IO::SystemFile::OpenMode::SF_OPEN_CREATE_PATH |
            IO::SystemFile::OpenMode::SF_OPEN_WRITE_ONLY))
        {
            AZ_Error("SerializeContextTools", false, "Unable to open output file '%s'.", outputPath.c_str());
            return false;
        }

        SerializeContext* context = application.GetSerializeContext();

        AZStd::vector<Uuid> systemComponents = Utilities::GetSystemComponents(application);
        AZStd::sort(systemComponents.begin(), systemComponents.end());

        rapidjson::Document doc;
        rapidjson::Value& root = doc.SetObject();
        rapidjson::Value scObject;
        scObject.SetObject();

        AZStd::string temp;
        temp.reserve(256 * 1024); // Reserve 256kb of memory to avoid the string constantly resizing.

        bool result = true;
        auto callback = [context, &doc, &scObject, &temp, &systemComponents, &result](const SerializeContext::ClassData* classData, const Uuid& /*typeId*/) -> bool
        {
            if (!DumpClassContent(classData, scObject, doc, systemComponents, context, temp))
            {
                result = false;
            }
            return true;
        };
        context->EnumerateAll(callback, true);
        root.AddMember("SerializeContext", AZStd::move(scObject), doc.GetAllocator());

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        outputFile.Write(buffer.GetString(), buffer.GetSize());
        outputFile.Close();
        return result;
    }

    AZStd::vector<Uuid> Dumper::CreateFilterListByNames(SerializeContext* context, AZStd::string_view name)
    {
        AZStd::vector<AZStd::string_view> names;
        auto AppendNames = [&names](AZStd::string_view filename)
        {
            names.emplace_back(filename);
        };
        AZ::StringFunc::TokenizeVisitor(name, AppendNames, ';');
        AZStd::vector<Uuid> filterIds;
        filterIds.reserve(names.size());

        for (const AZStd::string_view& singleName : names)
        {
            AZStd::vector<Uuid> foundFilters = context->FindClassId(Crc32(singleName.data(), singleName.length(), true));
            filterIds.insert(filterIds.end(), foundFilters.begin(), foundFilters.end());
        }

        return filterIds;
    }

    AZStd::string_view Dumper::ExtractNamespace(const AZStd::string& name)
    {
        size_t offset = 0;
        const char* startChar = name.data();
        const char* currentChar = name.data();
        while (*currentChar != 0 && *currentChar != '<')
        {
            if (*currentChar != ':')
            {
                ++currentChar;
            }
            else
            {
                ++currentChar;
                if (*currentChar == ':')
                {
                    AZ_Assert(currentChar - startChar >= 1, "Offset out of bounds while trying to extract namespace from name '%s'.", name.c_str());
                    offset = currentChar - startChar - 1; // -1 to exclude the last "::"
                }
            }
        }

        return AZStd::string_view(startChar, offset);
    }

    rapidjson::Value Dumper::WriteToJsonValue(const Uuid& uuid, rapidjson::Document& document)
    {
        char buffer[Uuid::MaxStringBuffer];
        int writtenCount = uuid.ToString(buffer, AZ_ARRAY_SIZE(buffer));
        if (writtenCount > 0)
        {
            return rapidjson::Value(buffer, writtenCount - 1, document.GetAllocator()); //-1 as the null character shouldn't be written.
        }
        else
        {
            return rapidjson::Value(rapidjson::StringRef("{uuid conversion failed}"));
        }
    }

    bool Dumper::DumpClassContent(const SerializeContext::ClassData* classData, rapidjson::Value& parent, rapidjson::Document& document,
        const AZStd::vector<Uuid>& systemComponents, SerializeContext* context, AZStd::string& scratchStringBuffer)
    {
        AZ_Assert(scratchStringBuffer.empty(), "Provided scratch string buffer wasn't empty.");

        rapidjson::Value classNode(rapidjson::kObjectType);
        DumpClassName(classNode, context, classData, document, scratchStringBuffer);

        Edit::ClassData* editData = classData->m_editData;
        GenericClassInfo* genericClassInfo = context->FindGenericClassInfo(classData->m_typeId);

        if (editData && editData->m_description)
        {
            AZStd::string_view description = editData->m_description;
            // Skipping if there's only one character as there are several cases where a blank description is given.
            if (description.size() > 1)
            {
                classNode.AddMember("Description", rapidjson::Value(description.data(), document.GetAllocator()), document.GetAllocator());
            }
        }

        classNode.AddMember("Id", rapidjson::StringRef(classData->m_name), document.GetAllocator());
        classNode.AddMember("Version", classData->IsDeprecated() ?
            rapidjson::Value(rapidjson::StringRef("Deprecated")) : rapidjson::Value(classData->m_version), document.GetAllocator());

        auto systemComponentIt = AZStd::lower_bound(systemComponents.begin(), systemComponents.end(), classData->m_typeId);
        bool isSystemComponent = systemComponentIt != systemComponents.end() && *systemComponentIt == classData->m_typeId;
        classNode.AddMember("IsSystemComponent", isSystemComponent, document.GetAllocator());
        classNode.AddMember("IsPrimitive", Utilities::IsSerializationPrimitive(genericClassInfo ? genericClassInfo->GetGenericTypeId() : classData->m_typeId), document.GetAllocator());
        classNode.AddMember("IsContainer", classData->m_container != nullptr, document.GetAllocator());
        if (genericClassInfo)
        {
            classNode.AddMember("GenericUuid", WriteToJsonValue(genericClassInfo->GetGenericTypeId(), document), document.GetAllocator());
            classNode.AddMember("Generics", DumpGenericStructure(genericClassInfo, context, document, scratchStringBuffer), document.GetAllocator());
        }

        if (!classData->m_elements.empty())
        {
            rapidjson::Value fields(rapidjson::kArrayType);
            rapidjson::Value bases(rapidjson::kArrayType);

            for (const SerializeContext::ClassElement& element : classData->m_elements)
            {
                DumpElementInfo(element, classData, context, fields, bases, document, scratchStringBuffer);
            }

            if (!bases.Empty())
            {
                classNode.AddMember("Bases", AZStd::move(bases), document.GetAllocator());
            }
            if (!fields.Empty())
            {
                classNode.AddMember("Fields", AZStd::move(fields), document.GetAllocator());
            }
        }

        parent.AddMember(WriteToJsonValue(classData->m_typeId, document), AZStd::move(classNode), document.GetAllocator());

        return true;
    }

    bool Dumper::DumpClassContent(AZStd::string& output, void* classPtr, const Uuid& classId, SerializeContext* context)
    {
        const SerializeContext::ClassData* classData = context->FindClassData(classId);
        if (!classData)
        {
            AZ_Printf("", "  Class data for '%s' is missing.\n", classId.ToString<AZStd::string>().c_str());
            return false;
        }

        size_t indention = 0;
        auto begin = [context, &output, &indention](void* /*instance*/, const SerializeContext::ClassData* classData, const SerializeContext::ClassElement* classElement) -> bool
        {
            for (size_t i = 0; i < indention; ++i)
            {
                output += ' ';
            }

            if (classData)
            {
                output += classData->m_name;
            }
            DumpElementInfo(output, classElement, context);
            DumpPrimitiveTag(output, classData, classElement);

            output += '\n';
            indention += 2;
            return true;
        };
        auto end = [&indention]() -> bool
        {
            indention = indention > 0 ? indention - 2 : 0;
            return true;
        };

        SerializeContext::EnumerateInstanceCallContext callContext(begin, end, context, SerializeContext::ENUM_ACCESS_FOR_WRITE, nullptr);
        context->EnumerateInstance(&callContext, classPtr, classId, classData, nullptr);
        return true;
    }

    void Dumper::DumpElementInfo(const SerializeContext::ClassElement& element, const SerializeContext::ClassData* classData, SerializeContext* context,
        rapidjson::Value& fields, rapidjson::Value& bases, rapidjson::Document& document, AZStd::string& scratchStringBuffer)
    {
        AZ_Assert(fields.IsArray(), "Expected 'fields' to be an array.");
        AZ_Assert(bases.IsArray(), "Expected 'bases' to be an array.");
        AZ_Assert(scratchStringBuffer.empty(), "Provided scratch string buffer wasn't empty.");

        const SerializeContext::ClassData* elementClass = context->FindClassData(element.m_typeId, classData);

        AppendTypeName(scratchStringBuffer, elementClass, element.m_typeId);
        Uuid elementTypeId = element.m_typeId;
        if (element.m_genericClassInfo)
        {
            DumpGenericStructure(scratchStringBuffer, element.m_genericClassInfo, context);
            elementTypeId = element.m_genericClassInfo->GetSpecializedTypeId();
        }
        if ((element.m_flags & SerializeContext::ClassElement::FLG_POINTER) != 0)
        {
            scratchStringBuffer += '*';
        }
        rapidjson::Value elementTypeString(scratchStringBuffer.c_str(), document.GetAllocator());
        scratchStringBuffer.clear();

        if ((element.m_flags & SerializeContext::ClassElement::FLG_BASE_CLASS) != 0)
        {
            rapidjson::Value baseNode(rapidjson::kObjectType);
            baseNode.AddMember("Type", AZStd::move(elementTypeString), document.GetAllocator());
            baseNode.AddMember("Uuid", WriteToJsonValue(elementTypeId, document), document.GetAllocator());

            bases.PushBack(AZStd::move(baseNode), document.GetAllocator());
        }
        else
        {
            rapidjson::Value elementNode(rapidjson::kObjectType);
            elementNode.AddMember("Name", rapidjson::StringRef(element.m_name), document.GetAllocator());
            elementNode.AddMember("Type", AZStd::move(elementTypeString), document.GetAllocator());
            elementNode.AddMember("Uuid", WriteToJsonValue(elementTypeId, document), document.GetAllocator());

            elementNode.AddMember("HasDefault", (element.m_flags & SerializeContext::ClassElement::FLG_NO_DEFAULT_VALUE) == 0, document.GetAllocator());
            elementNode.AddMember("IsDynamic", (element.m_flags & SerializeContext::ClassElement::FLG_DYNAMIC_FIELD) != 0, document.GetAllocator());
            elementNode.AddMember("IsPointer", (element.m_flags & SerializeContext::ClassElement::FLG_POINTER) != 0, document.GetAllocator());
            elementNode.AddMember("IsUiElement", (element.m_flags & SerializeContext::ClassElement::FLG_UI_ELEMENT) != 0, document.GetAllocator());
            elementNode.AddMember("DataSize", static_cast<uint64_t>(element.m_dataSize), document.GetAllocator());
            elementNode.AddMember("Offset", static_cast<uint64_t>(element.m_offset), document.GetAllocator());

            Edit::ElementData* elementEditData = element.m_editData;
            if (elementEditData)
            {
                elementNode.AddMember("Description", rapidjson::StringRef(elementEditData->m_description), document.GetAllocator());
            }

            if (element.m_genericClassInfo)
            {
                rapidjson::Value genericArray(rapidjson::kArrayType);
                rapidjson::Value classObject(rapidjson::kObjectType);

                const SerializeContext::ClassData* genericClassData = element.m_genericClassInfo->GetClassData();
                classObject.AddMember("Type", rapidjson::StringRef(genericClassData->m_name), document.GetAllocator());
                classObject.AddMember("GenericUuid", WriteToJsonValue(element.m_genericClassInfo->GetGenericTypeId(), document), document.GetAllocator());
                classObject.AddMember("SpecializedUuid", WriteToJsonValue(element.m_genericClassInfo->GetSpecializedTypeId(), document), document.GetAllocator());
                classObject.AddMember("Generics", DumpGenericStructure(element.m_genericClassInfo, context, document, scratchStringBuffer), document.GetAllocator());

                genericArray.PushBack(AZStd::move(classObject), document.GetAllocator());
                elementNode.AddMember("Generics", AZStd::move(genericArray), document.GetAllocator());
            }

            fields.PushBack(AZStd::move(elementNode), document.GetAllocator());
        }
    }

    void Dumper::DumpElementInfo(AZStd::string& output, const SerializeContext::ClassElement* classElement, SerializeContext* context)
    {
        if (classElement)
        {
            if (classElement->m_genericClassInfo)
            {
                DumpGenericStructure(output, classElement->m_genericClassInfo, context);
            }
            if ((classElement->m_flags & SerializeContext::ClassElement::FLG_POINTER) != 0)
            {
                output += '*';
            }
            output += ' ';
            output += classElement->m_name;
            if ((classElement->m_flags & SerializeContext::ClassElement::FLG_BASE_CLASS) != 0)
            {
                output += " [Base]";
            }
        }
    }

    void Dumper::DumpGenericStructure(AZStd::string& output, GenericClassInfo* genericClassInfo, SerializeContext* context)
    {
        output += '<';

        const SerializeContext::ClassData* classData = genericClassInfo->GetClassData();
        if (classData && classData->m_container)
        {
            bool firstArgument = true;
            auto callback = [&output, context, &firstArgument](const Uuid& elementClassId, const SerializeContext::ClassElement* genericClassElement) -> bool
            {
                if (!firstArgument)
                {
                    output += ',';
                }
                else
                {
                    firstArgument = false;
                }

                const SerializeContext::ClassData* argClassData = context->FindClassData(elementClassId);
                AppendTypeName(output, argClassData, elementClassId);
                if (genericClassElement->m_genericClassInfo)
                {
                    DumpGenericStructure(output, genericClassElement->m_genericClassInfo, context);
                }
                if ((genericClassElement->m_flags & SerializeContext::ClassElement::FLG_POINTER) != 0)
                {
                    output += '*';
                }
                return true;
            };
            classData->m_container->EnumTypes(callback);
        }
        else
        {
            // No container information available, so as much as possible through other means, although
            // this might not be complete information.
            size_t numArgs = genericClassInfo->GetNumTemplatedArguments();
            for (size_t i = 0; i < numArgs; ++i)
            {
                if (i != 0)
                {
                    output += ',';
                }
                const Uuid& argClassId = genericClassInfo->GetTemplatedTypeId(i);
                const SerializeContext::ClassData* argClass = context->FindClassData(argClassId);
                AppendTypeName(output, argClass, argClassId);
            }
        }
        output += '>';
    }

    rapidjson::Value Dumper::DumpGenericStructure(GenericClassInfo* genericClassInfo, SerializeContext* context,
        rapidjson::Document& parentDoc, AZStd::string& scratchStringBuffer)
    {
        AZ_Assert(scratchStringBuffer.empty(), "Provided scratch string buffer still contains data.");

        rapidjson::Value result(rapidjson::kArrayType);

        const SerializeContext::ClassData* classData = genericClassInfo->GetClassData();
        if (classData && classData->m_container)
        {
            auto callback = [&result, context, &parentDoc, &scratchStringBuffer](const Uuid& elementClassId,
                const SerializeContext::ClassElement* genericClassElement) -> bool
            {
                rapidjson::Value classObject(rapidjson::kObjectType);

                const SerializeContext::ClassData* argClassData = context->FindClassData(elementClassId);
                AppendTypeName(scratchStringBuffer, argClassData, elementClassId);
                classObject.AddMember("Type", rapidjson::Value(scratchStringBuffer.c_str(), parentDoc.GetAllocator()), parentDoc.GetAllocator());
                scratchStringBuffer.clear();

                classObject.AddMember("IsPointer", (genericClassElement->m_flags & SerializeContext::ClassElement::FLG_POINTER) != 0, parentDoc.GetAllocator());
                if (genericClassElement->m_genericClassInfo)
                {
                    GenericClassInfo* genericClassInfo = genericClassElement->m_genericClassInfo;
                    classObject.AddMember("GenericUuid", WriteToJsonValue(genericClassInfo->GetGenericTypeId(), parentDoc), parentDoc.GetAllocator());
                    classObject.AddMember("SpecializedUuid", WriteToJsonValue(genericClassInfo->GetSpecializedTypeId(), parentDoc), parentDoc.GetAllocator());
                    classObject.AddMember("Generics", DumpGenericStructure(genericClassInfo, context, parentDoc, scratchStringBuffer), parentDoc.GetAllocator());
                }
                else
                {
                    classObject.AddMember("GenericUuid", WriteToJsonValue(elementClassId, parentDoc), parentDoc.GetAllocator());
                    classObject.AddMember("SpecializedUuid", WriteToJsonValue(elementClassId, parentDoc), parentDoc.GetAllocator());
                }

                result.PushBack(AZStd::move(classObject), parentDoc.GetAllocator());
                return true;
            };
            classData->m_container->EnumTypes(callback);
        }
        else
        {
            // No container information available, so as much as possible through other means, although
            // this might not be complete information.
            size_t numArgs = genericClassInfo->GetNumTemplatedArguments();
            for (size_t i = 0; i < numArgs; ++i)
            {
                const Uuid& elementClassId = genericClassInfo->GetTemplatedTypeId(i);

                rapidjson::Value classObject(rapidjson::kObjectType);

                const SerializeContext::ClassData* argClassData = context->FindClassData(elementClassId);
                AppendTypeName(scratchStringBuffer, argClassData, elementClassId);
                classObject.AddMember("Type", rapidjson::Value(scratchStringBuffer.c_str(), parentDoc.GetAllocator()), parentDoc.GetAllocator());
                scratchStringBuffer.clear();

                classObject.AddMember("GenericUuid",
                    WriteToJsonValue(argClassData ? argClassData->m_typeId : elementClassId, parentDoc), parentDoc.GetAllocator());
                classObject.AddMember("SpecializedUuid", WriteToJsonValue(elementClassId, parentDoc), parentDoc.GetAllocator());
                classObject.AddMember("IsPointer", false, parentDoc.GetAllocator());

                result.PushBack(AZStd::move(classObject), parentDoc.GetAllocator());
            }
        }

        return result;
    }

    void Dumper::DumpPrimitiveTag(AZStd::string& output, const SerializeContext::ClassData* classData, const SerializeContext::ClassElement* classElement)
    {
        if (classData)
        {
            Uuid classId = classData->m_typeId;
            if (classElement && classElement->m_genericClassInfo)
            {
                classId = classElement->m_genericClassInfo->GetGenericTypeId();
            }
            if (Utilities::IsSerializationPrimitive(classId))
            {
                output += " [Primitive]";
            }
        }
    }

    void Dumper::DumpClassName(rapidjson::Value& parent, SerializeContext* context, const SerializeContext::ClassData* classData,
        rapidjson::Document& parentDoc, AZStd::string& scratchStringBuffer)
    {
        AZ_Assert(scratchStringBuffer.empty(), "Scratch string buffer is not empty.");

        Edit::ClassData* editData = classData->m_editData;

        GenericClassInfo* genericClassInfo = context->FindGenericClassInfo(classData->m_typeId);
        if (genericClassInfo)
        {
            // If the type itself is a generic, dump it's information.
            scratchStringBuffer = classData->m_name;
            DumpGenericStructure(scratchStringBuffer, genericClassInfo, context);
        }
        else
        {
            bool hasEditName = editData && editData->m_name && strlen(editData->m_name) > 0;
            scratchStringBuffer = hasEditName ? editData->m_name : classData->m_name;
        }

        AZStd::string_view namespacePortion = ExtractNamespace(scratchStringBuffer);
        if (!namespacePortion.empty())
        {
            parent.AddMember("Namespace",
                rapidjson::Value(namespacePortion.data(), azlossy_caster(namespacePortion.length()), parentDoc.GetAllocator()),
                parentDoc.GetAllocator());
            parent.AddMember("Name", rapidjson::Value(scratchStringBuffer.c_str() + namespacePortion.length() + 2, parentDoc.GetAllocator()), parentDoc.GetAllocator());
        }
        else
        {
            parent.AddMember("Name", rapidjson::Value(scratchStringBuffer.c_str(), parentDoc.GetAllocator()), parentDoc.GetAllocator());
        }
        scratchStringBuffer.clear();
    }

    void Dumper::AppendTypeName(AZStd::string& output, const SerializeContext::ClassData* classData, const Uuid& classId)
    {
        if (classData)
        {
            output += classData->m_name;
        }
        else if (classId == GetAssetClassId())
        {
            output += "Asset";
        }
        else
        {
            output += classId.ToString<AZStd::string>();
        }
    }
    // namespace AZ::SerializeContextTools
}
