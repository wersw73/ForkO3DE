/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/RTTI/RTTI.h>

#include <AtomToolsFramework/Document/AtomToolsDocument.h>
#include <Document/MaterialCanvasDocumentRequestBus.h>

namespace MaterialCanvas
{
    //! MaterialCanvasDocument
    class MaterialCanvasDocument
        : public AtomToolsFramework::AtomToolsDocument
        , public MaterialCanvasDocumentRequestBus::Handler
    {
    public:
        AZ_RTTI(MaterialCanvasDocument, "{90299628-AD02-4FEB-9527-7278FA2817AD}", AtomToolsFramework::AtomToolsDocument);
        AZ_CLASS_ALLOCATOR(MaterialCanvasDocument, AZ::SystemAllocator, 0);
        AZ_DISABLE_COPY_MOVE(MaterialCanvasDocument);

        static void Reflect(AZ::ReflectContext* context);

        MaterialCanvasDocument() = default;
        MaterialCanvasDocument(const AZ::Crc32& toolId);
        virtual ~MaterialCanvasDocument();

        // AtomToolsFramework::AtomToolsDocument overrides...
        static AtomToolsFramework::DocumentTypeInfo BuildDocumentTypeInfo();
        AtomToolsFramework::DocumentTypeInfo GetDocumentTypeInfo() const override;
        AtomToolsFramework::DocumentObjectInfoVector GetObjectInfo() const override;
        bool Open(const AZStd::string& loadPath) override;
        bool Save() override;
        bool SaveAsCopy(const AZStd::string& savePath) override;
        bool SaveAsChild(const AZStd::string& savePath) override;
        bool IsOpen() const override;
        bool IsModified() const override;
        bool BeginEdit() override;
        bool EndEdit() override;

        // MaterialCanvasDocumentRequestBus::Handler overrides...

    private:
        // AtomToolsFramework::AtomToolsDocument overrides...
        void Clear() override;
        bool ReopenRecordState() override;
        bool ReopenRestoreState() override;
    };
} // namespace MaterialCanvas
