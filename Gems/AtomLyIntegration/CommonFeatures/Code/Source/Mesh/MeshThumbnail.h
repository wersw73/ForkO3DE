/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/std/parallel/binary_semaphore.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzToolsFramework/Thumbnails/Thumbnail.h>
#include <AzToolsFramework/Thumbnails/ThumbnailerBus.h>
#endif

namespace AZ
{
    namespace LyIntegration
    {
        namespace Thumbnails
        {
            /**
             * Custom material or model thumbnail that detects when an asset changes and updates the thumbnail
             */
            class MeshThumbnail
                : public AzToolsFramework::Thumbnailer::Thumbnail
                , public AzToolsFramework::Thumbnailer::ThumbnailerRendererNotificationBus::Handler
                , private AzFramework::AssetCatalogEventBus::Handler
            {
                Q_OBJECT
            public:
                MeshThumbnail(AzToolsFramework::Thumbnailer::SharedThumbnailKey key);
                ~MeshThumbnail() override;

                //! AzToolsFramework::ThumbnailerRendererNotificationBus::Handler overrides...
                void ThumbnailRendered(QPixmap& thumbnailImage) override;
                void ThumbnailFailedToRender() override;

            protected:
                void LoadThread() override;

            private:
                // AzFramework::AssetCatalogEventBus::Handler interface overrides...
                void OnCatalogAssetChanged(const AZ::Data::AssetId& assetId) override;

                AZStd::binary_semaphore m_renderWait;
                Data::AssetId m_assetId;
            };

            /**
             * Cache configuration for large material thumbnails
             */
            class MeshThumbnailCache
                : public AzToolsFramework::Thumbnailer::ThumbnailCache<MeshThumbnail>
            {
            public:
                MeshThumbnailCache();
                ~MeshThumbnailCache() override;

                int GetPriority() const override;
                const char* GetProviderName() const override;

                static constexpr const char* ProviderName = "Mesh Thumbnails";

            protected:
                bool IsSupportedThumbnail(AzToolsFramework::Thumbnailer::SharedThumbnailKey key) const override;
            };
        } // namespace Thumbnails
    } // namespace LyIntegration
} // namespace AZ
