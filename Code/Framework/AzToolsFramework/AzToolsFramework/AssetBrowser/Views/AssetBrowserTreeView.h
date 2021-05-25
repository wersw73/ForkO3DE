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

#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Asset/AssetCommon.h>

#include <AzToolsFramework/AssetBrowser/AssetBrowserBus.h>
#include <AzToolsFramework/AssetBrowser/AssetBrowserFilterModel.h>
#include <AzToolsFramework/UI/UICore/QTreeViewStateSaver.hxx>

#include <QModelIndex>
#include <QPointer>
#endif

class QTimer;

namespace AzToolsFramework
{
    namespace AssetBrowser
    {
        class AssetBrowserEntry;
        class AssetBrowserModel;
        class AssetBrowserFilterModel;
        class EntryDelegate;

        class AssetBrowserTreeView
            : public QTreeViewWithStateSaving
            , public AssetBrowserViewRequestBus::Handler
            , public AssetBrowserComponentNotificationBus::Handler
        {
            Q_OBJECT
        public:
            explicit AssetBrowserTreeView(QWidget* parent = nullptr);
            ~AssetBrowserTreeView() override;

            //////////////////////////////////////////////////////////////////////////
            // QTreeView
            void setModel(QAbstractItemModel* model) override;
            //////////////////////////////////////////////////////////////////////////

            //! Set unique asset browser name, used to persist tree expansion states
            void SetName(const QString& name);

            // O3DE_DEPRECATED
            void LoadState(const QString& name);
            void SaveState() const;

            AZStd::vector<AssetBrowserEntry*> GetSelectedAssets() const;

            void SelectFolder(AZStd::string_view folderPath);

            //////////////////////////////////////////////////////////////////////////
            // AssetBrowserViewRequestBus
            void SelectProduct(AZ::Data::AssetId assetID) override;
            void SelectFileAtPath(const AZStd::string& assetPath) override;
            void ClearFilter() override;

            void Update() override;

            //////////////////////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////////////////////
            // AssetBrowserComponentNotificationBus
            void OnAssetBrowserComponentReady() override;
            //////////////////////////////////////////////////////////////////////////

            void SetThumbnailContext(const char* context) const;
            void SetShowSourceControlIcons(bool showSourceControlsIcons);
            void UpdateAfterFilter(bool hasFilter, bool selectFirstValidEntry);

            template <class TEntryType>
            const TEntryType* GetEntryFromIndex(const QModelIndex& index) const;

            bool IsIndexExpandedByDefault(const QModelIndex& index) const override;

        Q_SIGNALS:
            void selectionChangedSignal(const QItemSelection& selected, const QItemSelection& deselected);
            void ClearStringFilter();
            void ClearTypeFilter();

        protected Q_SLOTS:
            void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
            void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) override;

        private:
            QPointer<AssetBrowserModel> m_assetBrowserModel = nullptr;
            QPointer<AssetBrowserFilterModel> m_assetBrowserSortFilterProxyModel = nullptr;
            EntryDelegate* m_delegate = nullptr;

            bool m_expandToEntriesByDefault = false;

            QTimer* m_scTimer = nullptr;
            const int m_scUpdateInterval = 100;

            QString m_name;

            bool SelectProduct(const QModelIndex& idxParent, AZ::Data::AssetId assetID);
            bool SelectEntry(const QModelIndex& idxParent, const AZStd::vector<AZStd::string>& entryPathTokens, const uint32_t entryPathIndex = 0, bool useDisplayName = false);

            //! Grab one entry from the source thumbnail list and update it
            void UpdateSCThumbnails();

        private Q_SLOTS:
            void OnContextMenu(const QPoint& point);

            //! Get all visible source entries and place them in a queue to update their source control status
            void OnUpdateSCThumbnailsList();
        };

        template <class TEntryType>
        const TEntryType* AssetBrowserTreeView::GetEntryFromIndex(const QModelIndex& index) const
        {
            if (index.isValid())
            {
                QModelIndex sourceIndex = m_assetBrowserSortFilterProxyModel->mapToSource(index);
                AssetBrowserEntry* entry = static_cast<AssetBrowserEntry*>(sourceIndex.internalPointer());
                return azrtti_cast<const TEntryType*>(entry);
            }
            return nullptr;
        }
    } // namespace AssetBrowser
} // namespace AzToolsFramework
