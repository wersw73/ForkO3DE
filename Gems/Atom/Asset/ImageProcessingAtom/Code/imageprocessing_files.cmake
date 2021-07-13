#
# Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Source/Compressors/CryTextureSquisher/CryTextureSquisher.cpp
    Source/Compressors/CryTextureSquisher/CryTextureSquisher.h
    Include/Atom/ImageProcessing/ImageProcessingBus.h
    Include/Atom/ImageProcessing/ImageProcessingEditorBus.h
    Include/Atom/ImageProcessing/PixelFormats.h
    Include/Atom/ImageProcessing/ImageObject.h
    ../Assets/Editor/Resources.qrc
    ../Assets/Editor/Backward.png
    ../Assets/Editor/Forward.png
    ../Assets/Editor/reset.png
    Source/ImageBuilderBaseType.h
    Source/ImageBuilderComponent.cpp
    Source/ImageBuilderComponent.h
    Source/ImageProcessingSystemComponent.cpp
    Source/ImageProcessingSystemComponent.h
    Source/BuilderSettings/BuilderSettingManager.cpp
    Source/BuilderSettings/BuilderSettingManager.h
    Source/BuilderSettings/BuilderSettings.cpp
    Source/BuilderSettings/BuilderSettings.h
    Source/BuilderSettings/CubemapSettings.cpp
    Source/BuilderSettings/CubemapSettings.h
    Source/BuilderSettings/ImageProcessingDefines.h
    Source/BuilderSettings/MipmapSettings.cpp
    Source/BuilderSettings/MipmapSettings.h
    Source/BuilderSettings/PlatformSettings.h
    Source/BuilderSettings/PresetSettings.cpp
    Source/BuilderSettings/PresetSettings.h
    Source/BuilderSettings/TextureSettings.cpp
    Source/BuilderSettings/TextureSettings.h
    Source/Processing/AzDXGIFormat.h
    Source/Processing/DDSHeader.h
    Source/Processing/ImageAssetProducer.cpp
    Source/Processing/ImageAssetProducer.h
    Source/Processing/ImageConvert.cpp
    Source/Processing/ImageConvert.h
    Source/Processing/ImageConvertJob.cpp
    Source/Processing/ImageConvertJob.h
    Source/Processing/ImageFlags.h
    Source/Processing/ImageObjectImpl.cpp
    Source/Processing/ImageObjectImpl.h
    Source/Processing/ImagePreview.cpp
    Source/Processing/ImagePreview.h
    Source/Processing/ImageToProcess.h
    Source/Processing/PixelFormatInfo.cpp
    Source/Processing/PixelFormatInfo.h
    Source/Processing/Utils.cpp
    Source/Processing/Utils.h
    Source/Previewer/ImagePreviewer.cpp
    Source/Previewer/ImagePreviewer.h
    Source/Previewer/ImagePreviewer.ui
    Source/Previewer/ImagePreviewerFactory.cpp
    Source/Previewer/ImagePreviewerFactory.h
    Source/ImageLoader/DdsLoader.cpp
    Source/ImageLoader/ImageLoaders.cpp
    Source/ImageLoader/ImageLoaders.h
    Source/ImageLoader/QtImageLoader.cpp
    Source/ImageLoader/TIFFLoader.cpp
    Source/ImageLoader/ExrLoader.cpp
    Source/Editor/EditorCommon.h
    Source/Editor/EditorCommon.cpp
    Source/Editor/ImagePopup.cpp
    Source/Editor/ImagePopup.h
    Source/Editor/ImagePopup.ui
    Source/Editor/MipmapSettingWidget.cpp
    Source/Editor/MipmapSettingWidget.h
    Source/Editor/MipmapSettingWidget.ui
    Source/Editor/PresetInfoPopup.cpp
    Source/Editor/PresetInfoPopup.h
    Source/Editor/PresetInfoPopup.ui
    Source/Editor/ResolutionSettingItemWidget.cpp
    Source/Editor/ResolutionSettingItemWidget.h
    Source/Editor/ResolutionSettingItemWidget.ui
    Source/Editor/ResolutionSettingWidget.cpp
    Source/Editor/ResolutionSettingWidget.h
    Source/Editor/ResolutionSettingWidget.ui
    Source/Editor/TexturePresetSelectionWidget.cpp
    Source/Editor/TexturePresetSelectionWidget.h
    Source/Editor/TexturePresetSelectionWidget.ui
    Source/Editor/TexturePreviewWidget.cpp
    Source/Editor/TexturePreviewWidget.h
    Source/Editor/TexturePreviewWidget.ui
    Source/Editor/TexturePropertyEditor.cpp
    Source/Editor/TexturePropertyEditor.h
    Source/Editor/TexturePropertyEditor.ui
    Source/Converters/Gamma.cpp
    Source/Converters/FIR-Filter.cpp
    Source/Converters/FIR-Windows.h
    Source/Converters/FIR-Weights.h
    Source/Converters/FIR-Weights.cpp
    Source/Converters/AlphaCoverage.cpp
    Source/Converters/PixelOperation.h
    Source/Converters/PixelOperation.cpp
    Source/Converters/Normalize.cpp
    Source/Converters/ConvertPixelFormat.cpp
    Source/Converters/Cubemap.h
    Source/Converters/Cubemap.cpp
    Source/Converters/ColorChart.cpp
    Source/Converters/HighPass.cpp
    Source/Converters/Histogram.cpp
    Source/Converters/Histogram.h
    ../External/CubeMapGen/CBBoxInt32.cpp
    ../External/CubeMapGen/CBBoxInt32.h
    ../External/CubeMapGen/CCubeMapProcessor.cpp
    ../External/CubeMapGen/CCubeMapProcessor.h
    ../External/CubeMapGen/CImageSurface.cpp
    ../External/CubeMapGen/CImageSurface.h
    ../External/CubeMapGen/VectorMacros.h
    Source/Compressors/Compressor.h
    Source/Compressors/Compressor.cpp
    Source/Compressors/CTSquisher.h
    Source/Compressors/CTSquisher.cpp
    Source/Compressors/PVRTC.cpp
    Source/Compressors/PVRTC.h
    Source/Compressors/ETC2.cpp
    Source/Compressors/ETC2.h
    Source/Compressors/CryTextureSquisher/ColorBlockRGBA4x4f.cpp
    Source/Compressors/CryTextureSquisher/ColorBlockRGBA4x4s.cpp
    Source/Compressors/CryTextureSquisher/ColorBlockRGBA4x4c.cpp
    Source/Compressors/CryTextureSquisher/ColorBlockRGBA4x4f.h
    Source/Compressors/CryTextureSquisher/ColorBlockRGBA4x4s.h
    Source/Compressors/CryTextureSquisher/ColorBlockRGBA4x4c.h
    Source/Compressors/CryTextureSquisher/ColorTypes.h
    Source/Thumbnail/ImageThumbnail.cpp
    Source/Thumbnail/ImageThumbnail.h
    Source/Thumbnail/ImageThumbnailSystemComponent.cpp
    Source/Thumbnail/ImageThumbnailSystemComponent.h
)
