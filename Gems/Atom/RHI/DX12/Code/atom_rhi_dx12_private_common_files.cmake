#
# Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Source/RHI/Buffer.cpp
    Source/RHI/Buffer.h
    Source/RHI/BufferPool.cpp
    Source/RHI/BufferPool.h
    Source/RHI/BufferView.cpp
    Source/RHI/BufferView.h
    Source/RHI/CommandList.cpp
    Source/RHI/CommandList.h
    Source/RHI/CommandListBase.cpp
    Source/RHI/CommandListBase.h
    Source/RHI/CommandListPool.cpp
    Source/RHI/CommandListPool.h
    Source/RHI/CommandQueue.cpp
    Source/RHI/CommandQueue.h
    Source/RHI/CommandQueueContext.cpp
    Source/RHI/CommandQueueContext.h
    Source/RHI/AsyncUploadQueue.h
    Source/RHI/AsyncUploadQueue.cpp
    Source/RHI/Conversions.cpp
    Source/RHI/Conversions.h
    Source/RHI/Descriptor.cpp
    Source/RHI/Descriptor.h
    Source/RHI/DescriptorContext.cpp
    Source/RHI/DescriptorContext.h
    Source/RHI/DescriptorPool.cpp
    Source/RHI/DescriptorPool.h
    Source/RHI/Device.cpp
    Source/RHI/Device.h
    Source/RHI/Fence.cpp
    Source/RHI/Fence.h
    Source/RHI/FrameGraphCompiler.cpp
    Source/RHI/FrameGraphCompiler.h
    Source/RHI/FrameGraphExecuteGroup.cpp
    Source/RHI/FrameGraphExecuteGroup.h
    Source/RHI/FrameGraphExecuteGroupBase.cpp
    Source/RHI/FrameGraphExecuteGroupBase.h
    Source/RHI/FrameGraphExecuteGroupMerged.cpp
    Source/RHI/FrameGraphExecuteGroupMerged.h
    Source/RHI/FrameGraphExecuter.cpp
    Source/RHI/FrameGraphExecuter.h
    Source/RHI/Image.cpp
    Source/RHI/Image.h
    Source/RHI/ImagePool.cpp
    Source/RHI/ImagePool.h
    Source/RHI/ImageView.cpp
    Source/RHI/ImageView.h
    Source/RHI/StreamingImagePool.cpp
    Source/RHI/StreamingImagePool.h
    Source/RHI/IndirectBufferSignature.cpp
    Source/RHI/IndirectBufferSignature.h
    Source/RHI/IndirectBufferWriter.cpp
    Source/RHI/IndirectBufferWriter.h
    Source/RHI/BufferMemoryAllocator.cpp
    Source/RHI/BufferMemoryAllocator.h
    Source/RHI/BufferMemoryView.cpp
    Source/RHI/BufferMemoryView.h
    Source/RHI/Memory.h
    Source/RHI/MemoryPageAllocator.cpp
    Source/RHI/MemoryPageAllocator.h
    Source/RHI/MemorySubAllocator.h
    Source/RHI/MemoryView.cpp
    Source/RHI/MemoryView.h
    Source/RHI/NsightAftermath.h
    Source/RHI/StagingMemoryAllocator.cpp
    Source/RHI/StagingMemoryAllocator.h
    Source/RHI/PhysicalDevice.h
    Source/RHI/PipelineLayout.cpp
    Source/RHI/PipelineLayout.h
    Source/RHI/PipelineState.cpp
    Source/RHI/PipelineState.h
    Source/RHI/PipelineLibrary.cpp
    Source/RHI/PipelineLibrary.h
    Source/RHI/Query.cpp
    Source/RHI/Query.h
    Source/RHI/QueryPool.cpp
    Source/RHI/QueryPool.h
    Source/RHI/QueryPoolResolver.cpp
    Source/RHI/QueryPoolResolver.h
    Source/RHI/ReleaseQueue.h
    Source/RHI/ResourcePoolResolver.h
    Source/RHI/Sampler.cpp
    Source/RHI/Sampler.h
    Source/RHI/Scope.cpp
    Source/RHI/Scope.h
    Source/RHI/ShaderResourceGroup.cpp
    Source/RHI/ShaderResourceGroup.h
    Source/RHI/ShaderResourceGroupPool.cpp
    Source/RHI/ShaderResourceGroupPool.h
    Source/RHI/SwapChain.cpp
    Source/RHI/SwapChain.h
    Source/RHI/DX12.cpp
    Source/RHI/DX12.h
    Source/RHI/SystemComponent.cpp
    Source/RHI/SystemComponent.h
    Source/RHI/resource.h
    Source/RHI/AliasedHeap.cpp
    Source/RHI/AliasedHeap.h
    Source/RHI/AliasingBarrierTracker.cpp
    Source/RHI/AliasingBarrierTracker.h
    Source/RHI/TransientAttachmentPool.cpp
    Source/RHI/TransientAttachmentPool.h
    Source/RHI/RayTracingBufferPools.h
    Source/RHI/RayTracingBlas.cpp
    Source/RHI/RayTracingBlas.h
    Source/RHI/RayTracingTlas.cpp
    Source/RHI/RayTracingTlas.h
    Source/RHI/RayTracingPipelineState.cpp
    Source/RHI/RayTracingPipelineState.h
    Source/RHI/RayTracingShaderTable.cpp
    Source/RHI/RayTracingShaderTable.h
)
