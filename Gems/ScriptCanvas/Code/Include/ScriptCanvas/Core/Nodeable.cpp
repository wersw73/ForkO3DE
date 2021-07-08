/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "Nodeable.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <ScriptCanvas/Core/SubgraphInterface.h>
#include <ScriptCanvas/Grammar/GrammarContext.h>
#include <ScriptCanvas/Grammar/GrammarContextBus.h>

namespace NodeableOutCpp
{
    void NoOp([[maybe_unused]] AZ::BehaviorValueParameter*, [[maybe_unused]] AZ::BehaviorValueParameter*, [[maybe_unused]] int) {}
}

namespace ScriptCanvas
{
    using namespace Execution;

    Nodeable::Nodeable()
        : m_noOpFunctor(&NodeableOutCpp::NoOp)
    {}

    Nodeable::Nodeable(ExecutionStateWeakPtr executionState)
        : m_noOpFunctor(&NodeableOutCpp::NoOp)
        , m_executionState(executionState)
    {}   

#if !defined(RELEASE) 
    void Nodeable::CallOut(size_t index, AZ::BehaviorValueParameter* resultBVP, AZ::BehaviorValueParameter* argsBVPs, int numArguments) const
    {
        GetExecutionOutChecked(index)(resultBVP, argsBVPs, numArguments);
    }
#else
    void Nodeable::CallOut(size_t index, AZ::BehaviorValueParameter* resultBVP, AZ::BehaviorValueParameter* argsBVPs, int numArguments) const
    {
        GetExecutionOut(index)(resultBVP, argsBVPs, numArguments);
    }
#endif // !defined(RELEASE) 

    void Nodeable::Deactivate()
    {
        OnDeactivate();
    }

    AZ::Data::AssetId Nodeable::GetAssetId() const
    {
        return m_executionState->GetAssetId();
    }

    AZ::EntityId Nodeable::GetEntityId() const
    {
        return m_executionState->GetEntityId();
    }

    void Nodeable::Reflect(AZ::ReflectContext* reflectContext)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
        {
            serializeContext->Class<Nodeable>();

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<Nodeable>("Nodeable", "Nodeable")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflectContext))
        {
            behaviorContext->Class<Nodeable>()
                ->Attribute(AZ::Script::Attributes::ExcludeFrom, AZ::Script::Attributes::ExcludeFlags::List)
                ->Attribute(AZ::ScriptCanvasAttributes::VariableCreationForbidden, AZ::AttributeIsValid::IfPresent)
                ->Attribute(AZ::Script::Attributes::UseClassIndexAllowNil, AZ::AttributeIsValid::IfPresent)
                ->Constructor<ExecutionStateWeakPtr>()
                ->Method("Deactivate", &Nodeable::Deactivate)
                ->Method("InitializeExecutionState", &Nodeable::InitializeExecutionState)
                ->Method("InitializeExecutionOuts", &Nodeable::InitializeExecutionOuts)
                ->Method("InitializeExecutionOutByRequiredCount", &Nodeable::InitializeExecutionOutByRequiredCount)
                ->Method("IsActive", &Nodeable::IsActive)
                ;
        }
    }

    const FunctorOut& Nodeable::GetExecutionOut(size_t index) const
    {
        AZ_Assert(index < m_outs.size(), "index out of range in Nodeable::m_outs");
        auto& iter = m_outs[index];
        AZ_Assert(iter, "null execution methods are not allowed, index: %zu", index);
        return iter;
    }

    const FunctorOut& Nodeable::GetExecutionOutChecked(size_t index) const
    {
        if (index >= m_outs.size() || !m_outs[index])
        {
            return m_noOpFunctor;
        }
        
        return m_outs[index];
    }

    AZ::EntityId Nodeable::GetScriptCanvasId() const
    {
        return m_executionState->GetScriptCanvasId();
    }

    void Nodeable::InitializeExecutionOuts(size_t count)
    {
        m_outs.resize(count, m_noOpFunctor);
    }

    void Nodeable::InitializeExecutionOutByRequiredCount()
    {
        InitializeExecutionOuts(GetRequiredOutCount());
    }

    void Nodeable::InitializeExecutionState(ExecutionState* executionState)
    {
        AZ_Assert(executionState != nullptr, "execution state for nodeable must not be nullptr");
        AZ_Assert(m_executionState == nullptr, "execution state already initialized");
        m_executionState = executionState->WeakFromThis();
        OnInitializeExecutionState();
    }

    void Nodeable::SetExecutionOut(size_t index, FunctorOut&& out)
    {
        AZ_Assert(out, "null executions methods are not allowed, index: %zu", index);
        m_outs[index] = AZStd::move(out);
    }

    void Nodeable::SetExecutionOutChecked(size_t index, FunctorOut&& out)
    {
        if (!out)
        {
            AZ_Error("ScriptCanvas", false, "null executions methods are not allowed, index: %zu", index);
            return;
        }

        SetExecutionOut(index, AZStd::move(out));
    }
}
