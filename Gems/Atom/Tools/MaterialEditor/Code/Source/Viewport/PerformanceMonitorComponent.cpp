/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RHI/RHISystemInterface.h>
#include <Atom/RHI.Reflect/CpuTimingStatistics.h>
#include <Atom/RHI/CpuProfiler.h>
#include <Atom/RPI.Public/Pass/ParentPass.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>

#include <Viewport/PerformanceMonitorComponent.h>

namespace MaterialEditor
{
    void PerformanceMonitorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<PerformanceMonitorComponent, AZ::Component>()
                ->Version(0);
        }
    }

    PerformanceMonitorComponent::PerformanceMonitorComponent()
    {
    }

    void PerformanceMonitorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("PerformanceMonitorService", 0x6a44241a));
    }

    void PerformanceMonitorComponent::Init()
    {
    }

    void PerformanceMonitorComponent::Activate()
    {
        PerformanceMonitorRequestBus::Handler::BusConnect();
    }

    void PerformanceMonitorComponent::Deactivate()
    {
        PerformanceMonitorRequestBus::Handler::BusDisconnect();
    }

    void PerformanceMonitorComponent::SetProfilerEnabled(bool enabled)
    {
        if (m_profilingEnabled == enabled)
        {
            return;
        }

        AZ::RHI::Ptr<AZ::RPI::ParentPass> rootPass = AZ::RPI::PassSystemInterface::Get()->GetRootPass();
        if (rootPass)
        {
            rootPass->SetTimestampQueryEnabled(enabled);
        }
        else
        {
            AZ_Error("PerformanceMonitorComponent", false, "Failed to find root pass.");
        }

        AZ::RHI::RHISystemInterface::Get()->ModifyFrameSchedulerStatisticsFlags(
            AZ::RHI::FrameSchedulerStatisticsFlags::GatherCpuTimingStatistics,
            enabled);

        AZ::RHI::CpuProfiler::Get()->SetProfilerEnabled(enabled);

        if (enabled)
        {
            ResetStats();
        }

        m_profilingEnabled = enabled;
    }

    void PerformanceMonitorComponent::GatherMetrics()
    {
        if (!m_profilingEnabled)
        {
            return;
        }

        if (++m_sample > SampleCount)
        {
            m_sample = 0;

            UpdateMetrics();
            ResetStats();
        }

        const AZ::RHI::CpuTimingStatistics* stats = AZ::RHI::RHISystemInterface::Get()->GetCpuTimingStatistics();
        if (stats)
        {
            m_cpuFrameTimeMs.PushSample(stats->GetFrameToFrameTimeMilliseconds());
        }

        AZ::RHI::Ptr<AZ::RPI::ParentPass> rootPass = AZ::RPI::PassSystemInterface::Get()->GetRootPass();
        if (rootPass)
        {
            AZ::RPI::TimestampResult timestampResult = rootPass->GetLatestTimestampResult();
            double gpuFrameTimeMs = aznumeric_cast<double>(timestampResult.GetDurationInNanoseconds()) / 1000000;
            m_gpuFrameTimeMs.PushSample(gpuFrameTimeMs);
        }
    }

    const PerformanceMetrics& PerformanceMonitorComponent::GetMetrics()
    {
        UpdateMetrics();
        return m_metrics;
    }

    void PerformanceMonitorComponent::UpdateMetrics()
    {
        m_metrics.m_cpuFrameTimeMs = m_cpuFrameTimeMs.GetAverage();
        m_metrics.m_gpuFrameTimeMs = m_gpuFrameTimeMs.GetAverage();
    }

    void PerformanceMonitorComponent::ResetStats()
    {
        m_cpuFrameTimeMs.Reset();
        m_gpuFrameTimeMs.Reset();
    }
}
