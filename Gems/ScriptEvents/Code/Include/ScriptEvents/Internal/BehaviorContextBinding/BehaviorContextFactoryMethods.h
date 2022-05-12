/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include "DefaultEventHandler.h"

#include <AzCore/RTTI/BehaviorContext.h>

namespace ScriptEvents
{

    class BehaviorHandlerFactoryMethod  : public AZ::BehaviorMethod
    {
    public:
        AZ_CLASS_ALLOCATOR(BehaviorHandlerFactoryMethod, AZ::SystemAllocator, 0);

        // The result parameter takes the 0th index
        enum ParameterIndex
        {
            StartNamedArgument = 1
        };

        BehaviorHandlerFactoryMethod(AZ::BehaviorEBus* ebus, AZ::BehaviorContext* behaviorContext, const AZStd::string& name)
            : AZ::BehaviorMethod(behaviorContext)
            , m_ebus(ebus)
            , m_name(name)
        {}

        ~BehaviorHandlerFactoryMethod() override
        {}

        bool Call([[maybe_unused]] AZ::BehaviorArgument* arguments, [[maybe_unused]] unsigned int numArguments, [[maybe_unused]] AZ::BehaviorArgument* result = nullptr) const override
        {
            return false;
        }

        bool HasResult() const override
        {
            return false;
        }


        bool IsMember() const override
        {
            return false;
        }

        bool HasBusId() const override
        {
            return false;
        }

        const AZ::BehaviorParameter* GetBusIdArgument() const override
        {
            return nullptr;
        }

        void OverrideParameterTraits(size_t /*index*/, AZ::u32 /*addTraits*/, AZ::u32 /*removeTraits*/) override
        {
        }

        size_t GetNumArguments() const override
        {
            return 0;
        }

        size_t GetMinNumberOfArguments() const override
        {
            return 0;
        }

        const AZ::BehaviorParameter* GetArgument(size_t /*index*/) const override
        {
            return nullptr;
        }

        const AZStd::string* GetArgumentName(size_t /*index*/) const override
        {
            return nullptr;
        }

        void SetArgumentName(size_t /*index*/, const AZStd::string& /*name*/) override
        {
        }

        const AZStd::string* GetArgumentToolTip(size_t /*index*/) const override
        {
            return nullptr;
        }

        void SetArgumentToolTip(size_t /*index*/, const AZStd::string& /*name*/) override
        {
        }

        void SetDefaultValue(size_t /*index*/, AZ::BehaviorDefaultValuePtr /*defaultValue*/) override
        {
        }

        AZ::BehaviorDefaultValuePtr GetDefaultValue(size_t /*index*/) const override
        {
            return nullptr;
        }

        const AZ::BehaviorParameter* GetResult() const override
        {
            return nullptr;
        }

    protected:

        AZStd::string m_name;
        AZ::BehaviorEBus* m_ebus;
    };

    class DefaultBehaviorHandlerCreator : public  BehaviorHandlerFactoryMethod
    {
    public:
        AZ_CLASS_ALLOCATOR(DefaultBehaviorHandlerCreator, AZ::SystemAllocator, 0);

        DefaultBehaviorHandlerCreator(AZ::BehaviorEBus* ebus, AZ::BehaviorContext* behaviorContext, const AZStd::string& name)
            : BehaviorHandlerFactoryMethod(ebus, behaviorContext, name)
        {
        }

        bool Call(AZ::BehaviorArgument* arguments, unsigned int numArguments, AZ::BehaviorArgument* result = nullptr) const override
        {
            const ScriptEvents::ScriptEvent* scriptEventDefinition = nullptr;
            if (numArguments > 0)
            {
                scriptEventDefinition = static_cast<const ScriptEvents::ScriptEvent*>(arguments[0].GetValueAddress());
            }

            if (result)
            {
                // the result is expecting a bus handler, store the functor* as the result's value
                *static_cast<void**>(result->m_value) = aznew DefaultBehaviorHandler(m_ebus, scriptEventDefinition);
                return true;
            }

            return false;
        }

        bool HasResult() const override
        {
            return true;
        }

        bool IsMember() const override 
        {
            return true; 
        }

    };

    class DefaultBehaviorHandlerDestroyer : public BehaviorHandlerFactoryMethod
    {
    public:
        AZ_CLASS_ALLOCATOR(DefaultBehaviorHandlerDestroyer, AZ::SystemAllocator, 0);

        DefaultBehaviorHandlerDestroyer(AZ::BehaviorEBus* ebus, AZ::BehaviorContext* behaviorContext, const AZStd::string& name)
            : BehaviorHandlerFactoryMethod(ebus, behaviorContext, name)
        {
        }

        bool Call(AZ::BehaviorArgument* arguments, [[maybe_unused]] unsigned int numArguments, [[maybe_unused]] AZ::BehaviorArgument* result = nullptr) const override
        {
            AZ_Assert(arguments, "Must pass in the handler to delete");
            if (arguments)
            {
                // The first argument is the handler that needs to be deleted
                delete *arguments[0].GetAsUnsafe<DefaultBehaviorHandler*>();
            }
            return true;
        }

        bool HasResult() const override
        {
            return true;
        }

        bool IsMember() const override
        {
            return true;
        }

    };
}
