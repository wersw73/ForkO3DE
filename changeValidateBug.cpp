#include <AzCore/IO/Path/Path.h>
#include <AzCore/Math/Uuid.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Environment.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzQtComponents/Components/StyleManager.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <UI/PropertyEditor/PropertyIntSpinCtrl.hxx>
#include <UI/PropertyEditor/PropertyStringLineEditCtrl.hxx>

#include <QApplication>

class MyClass final
{
public:
    AZ_RTTI(MyClass, "{22309EFA-BA7C-4469-9B2B-D995FF5EBE13}")

    static AZ::Outcome<void, AZStd::string> ValidateInt([[maybe_unused]] void* newValue, [[maybe_unused]] const AZ::Uuid& valueType)
    {
        if (valueType == azrtti_typeid<int>())
        {
            auto* intValue = static_cast<int*>(newValue);
            if (intValue && *intValue == 5)
            {
                return AZ::Success();
            }
        }
        return AZ::Failure(AZStd::string("The only valid value is 5"));
    }

    static AZ::Outcome<void, AZStd::string> ValidateString([[maybe_unused]] void* newValue, [[maybe_unused]] const AZ::Uuid& valueType)
    {
        if (valueType == azrtti_typeid<AZStd::string>())
        {
            auto* stringValue = static_cast<AZStd::string*>(newValue);
            if (stringValue && *stringValue == "Hello, world!")
            {
                return AZ::Success();
            }
        }
        return AZ::Failure(AZStd::string("The only valid value is \"Hello, world!\""));
    }

    static void Reflect(AZ::ReflectContext* ctx)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(ctx))
        {
            serializeContext->Class<MyClass>()
                ->Field("theInt", &MyClass::m_int)
                ->Field("theString", &MyClass::m_string)
                ;

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<MyClass>("My Class", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MyClass::m_int, "The int", "This is this instance's int")
                        ->Attribute(AZ::Edit::Attributes::ChangeValidate, &MyClass::ValidateInt)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MyClass::m_string, "The string", "This is this instance's string")
                        ->Attribute(AZ::Edit::Attributes::ChangeValidate, &MyClass::ValidateString)
                    ;
            }
        }
    }
private:
    int m_int{};
    AZStd::string m_string{"default value"};
};

class PropertyHandlers
    : private AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Handler
{
public:
    friend class PropertyManagerComponentFactory;

    PropertyHandlers()
    {
        PropertyTypeRegistrationMessages::Bus::Handler::BusConnect();
    }
    void RegisterPropertyType([[maybe_unused]] AzToolsFramework::PropertyHandlerBase* pHandler) override
    {
    }
    void UnregisterPropertyType([[maybe_unused]] AzToolsFramework::PropertyHandlerBase* pHandler) override
    {
    }

    AzToolsFramework::PropertyHandlerBase* ResolvePropertyHandler([[maybe_unused]] AZ::u32 handlerName, const AZ::Uuid& handlerType) override
    {
        if (handlerType == azrtti_typeid<int>())
        {
            return &m_intHandler;
        }
        if (handlerType == azrtti_typeid<AZStd::string>())
        {
            return &m_stringHandler;
        }
        return nullptr;
    }

private:
    AzToolsFramework::IntSpinBoxHandler<int> m_intHandler;
    AzToolsFramework::StringPropertyLineEditHandler m_stringHandler;
};

int main(int argc, char** argv)
{
    AZ::Environment::Create(nullptr);
    AZ::AllocatorInstance<AZ::SystemAllocator>::Create();
    {
        AZ::SerializeContext ctx;
        ctx.CreateEditContext();

        MyClass::Reflect(&ctx);

        QApplication app(argc, argv);
        AzQtComponents::StyleManager styleManager(&app);
        styleManager.initialize(&app, "");

        PropertyHandlers handlers;

        MyClass myClass{};

        AzToolsFramework::ReflectedPropertyEditor rpe(nullptr);
        rpe.Setup(&ctx, nullptr, false);
        rpe.AddInstance(&myClass);
        rpe.ExpandAll();
        rpe.InvalidateAll();
        rpe.resize(640,480);
        rpe.show();

        app.exec();
    }
    AZ::AllocatorInstance<AZ::SystemAllocator>::Destroy();
    AZ::Environment::Destroy();
    return 0;
}
