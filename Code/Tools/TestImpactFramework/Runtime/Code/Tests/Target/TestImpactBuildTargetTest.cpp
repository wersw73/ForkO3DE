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

#include <TestImpactTestUtils.h>

#include <Target/TestImpactProductionTargetList.h>
#include <Target/TestImpactTestTargetList.h>

#include <AzCore/UnitTest/TestTypes.h>
#include <AzTest/AzTest.h>

namespace UnitTest
{
    namespace
    {
        AZStd::string GenerateBuildTargetName(size_t index)
        {
            return AZStd::string::format("Target%u", index);
        }

        AZStd::string GenerateBuildTargetOutputName(size_t index)
        {
            return AZStd::string::format("Output%u", index);
        }

        AZ::IO::Path GenerateBuildTargetPath(size_t index)
        {
            return AZStd::string::format("C:\\Repo\\Dir%u", index);
        }

        AZStd::string GenerateTestTargetSuite(size_t index)
        {
            return AZStd::string::format("Suite%u", index);
        }

        TestImpact::LaunchMethod GenerateLaunchMethod(size_t index)
        {
            return index % 2 == 0 ? TestImpact::LaunchMethod::StandAlone : TestImpact::LaunchMethod::TestRunner;
        }

        AZStd::string GenerateStaticSourceFile(size_t index)
        {
            return AZStd::string::format("StaticSource%u", index);
        }

        TestImpact::AutogenPairs GenerateAutogenSourceFiles(size_t index)
        {
            TestImpact::AutogenPairs autogen;
            autogen.m_input = AZStd::string::format("InputSource%u", index);
            autogen.m_outputs.push_back(AZStd::string::format("OutputSource%u", index));
            autogen.m_outputs.push_back(AZStd::string::format("OutputHeader%u", index));
            return autogen;
        }

        TestImpact::TargetSources GenerateTargetSources(size_t index)
        {
            TestImpact::TargetSources sources;
            sources.m_staticSources.resize(index + 1);
            for (size_t i = 0; i < sources.m_staticSources.size(); i++)
            {
                sources.m_staticSources[i] = GenerateStaticSourceFile(i);
            }

            // Only generate autogen sources for even-numbered indexes
            if (index % 2 == 0)
            {
                sources.m_autogenSources.resize(index + 1);
                for (size_t i = 0; i < sources.m_autogenSources.size(); i++)
                {
                    sources.m_autogenSources[i] = GenerateAutogenSourceFiles(i);
                }
            }

            return sources;
        }

        TestImpact::ProductionTargetDescriptor GenerateProductionTargetDescriptor(size_t index = 0)
        {
            return TestImpact::ProductionTargetDescriptor(
                {
                    TestImpact::BuildMetaData
                    {
                        GenerateBuildTargetName(index), GenerateBuildTargetOutputName(index), GenerateBuildTargetPath(index)
                    },
                    GenerateTargetSources(index)
                });
        }

        TestImpact::TestTargetDescriptor GenerateTestTargetDescriptor(size_t index = 0)
        {
            return TestImpact::TestTargetDescriptor(
                {
                    TestImpact::BuildMetaData
                    {
                        GenerateBuildTargetName(index), GenerateBuildTargetOutputName(index), GenerateBuildTargetPath(index)
                    },
                    GenerateTargetSources(index)
                },
                TestImpact::TestTargetMeta
                {
                    GenerateTestTargetSuite(index), GenerateLaunchMethod(index)
                });
        }
    } // namespace

    template<typename Target>
    typename Target::Descriptor GenerateTargetDescriptor(size_t index = 0)
    {
        static_assert(
            AZStd::is_same_v<Target, TestImpact::ProductionTarget> || AZStd::is_same_v<Target, TestImpact::TestTarget>,
            "Unexpected target type, wants TestImpact::ProductionTarget or TestImpact::TestTarget");

        if constexpr (AZStd::is_same_v<Target, TestImpact::ProductionTarget>)
        {
            return GenerateProductionTargetDescriptor(index);
        }
        else
        {
            return GenerateTestTargetDescriptor(index);
        }
    }

    void ValidateSources(const TestImpact::TargetSources& sources, size_t index = 0)
    {
        EXPECT_EQ(sources.m_staticSources.size(), index + 1);
        for (size_t i = 0; i < sources.m_staticSources.size(); i++)
        {
            EXPECT_EQ(sources.m_staticSources[i], GenerateStaticSourceFile(i));
        }

        // Even numbered indexes have autogen sources
        if (index % 2 == 0)
        {
            EXPECT_EQ(sources.m_autogenSources.size(), index + 1);
            for (size_t i = 0; i < sources.m_autogenSources.size(); i++)
            {
                enum
                {
                    OutputSource = 0,
                    OutputHeader = 1
                };

                const TestImpact::AutogenPairs expectedAutogenSources = GenerateAutogenSourceFiles(i);
                EXPECT_EQ(sources.m_autogenSources[i].m_input, expectedAutogenSources.m_input);
                EXPECT_EQ(sources.m_autogenSources[i].m_outputs[OutputSource], expectedAutogenSources.m_outputs[OutputSource]);
                EXPECT_EQ(sources.m_autogenSources[i].m_outputs[OutputHeader], expectedAutogenSources.m_outputs[OutputHeader]);
            }
        }
        else
        {
            EXPECT_TRUE(sources.m_autogenSources.empty());
        }
    }

    void ValidateTarget(const TestImpact::ProductionTarget& target, size_t index = 0)
    {
        EXPECT_EQ(target.GetName(), GenerateBuildTargetName(index));
        EXPECT_EQ(target.GetOutputName(), GenerateBuildTargetOutputName(index));
        EXPECT_EQ(target.GetPath(), GenerateBuildTargetPath(index));
        EXPECT_EQ(target.GetType(), TestImpact::TargetType::Production);
        ValidateSources(target.GetSources(), index);
    }

    void ValidateTarget(const TestImpact::TestTarget& target, size_t index = 0)
    {
        EXPECT_EQ(target.GetName(), GenerateBuildTargetName(index));
        EXPECT_EQ(target.GetOutputName(), GenerateBuildTargetOutputName(index));
        EXPECT_EQ(target.GetPath(), GenerateBuildTargetPath(index));
        EXPECT_EQ(target.GetType(), TestImpact::TargetType::Test);
        EXPECT_EQ(target.GetSuite(), GenerateTestTargetSuite(index));
        EXPECT_EQ(target.GetLaunchMethod(), GenerateLaunchMethod(index));
        ValidateSources(target.GetSources(), index);
    }

    template<typename TargetList>
    class TargetListFixture
        : public AllocatorsTestFixture
    {
    public:
        using TargetListType = TargetList;
        using TargetType = typename TargetList::TargetType;

        static constexpr size_t m_numTargets = 10;
    };

    using TargetTypes = testing::Types<TestImpact::ProductionTargetList, TestImpact::TestTargetList>;
    TYPED_TEST_CASE(TargetListFixture, TargetTypes);

    TYPED_TEST(TargetListFixture, CreateTarget_ExpectValidTarget)
    {
        // Given a target of the specified type
        TargetType target(GenerateTargetDescriptor<TargetType>());

        // Expect the target to match the procedurally generated target descriptor
        ValidateTarget(target);
    }

    TYPED_TEST(TargetListFixture, CreateEmptyTargetList_ExpectTargetException)
    {
        try
        {
            // Given an empty target list
            TargetListType targetList({});

            // Do not expect the target list construction to succeed
            FAIL();
        }
        catch ([[maybe_unused]] const TestImpact::TargetException& e)
        {
            // Expect a target exception to be thrown
            SUCCEED();
        }
        catch (...)
        {
            // Do not expect any other exceptions
            FAIL();
        }
    }

    TYPED_TEST(TargetListFixture, CreateTargetListWithDuplicateDescriptor_ExpectTargetException)
    {
        try
        {
            // Given a set of target descriptors containing a single duplicate            
            AZStd::vector<TargetType::Descriptor> descriptors;
            descriptors.reserve(m_numTargets);
            for (size_t i = 0; i < m_numTargets; i++)
            {
                // Wrap the last index round to repeat the first index
                descriptors.push_back(GenerateTargetDescriptor<TargetType>(i % (m_numTargets - 1)));
            }

            // When constructing the target list containing the duplicate target descriptor
            TargetListType targetList(AZStd::move(descriptors));

            // Do not expect the target list construction to succeed
            FAIL();
        }
        catch ([[maybe_unused]] const TestImpact::TargetException& e)
        {
            // Expect a target exception to be thrown
            SUCCEED();
        }
        catch (...)
        {
            // Do not expect any other exceptions
            FAIL();
        }
    }

    TYPED_TEST(TargetListFixture, CreateTargetListWithValidDescriptors_ExpectValidTargetList)
    {
        // Given a valid set of target descriptor
        AZStd::vector<TargetType::Descriptor> descriptors;
        descriptors.reserve(m_numTargets);
        for (size_t i = 0; i < m_numTargets; i++)
        {
            descriptors.push_back(GenerateTargetDescriptor<TargetType>(i));
        }

        // When constructing the target list containing the valid target descriptors
        TargetListType targetList(AZStd::move(descriptors));

        // Expect the number of targets in the list to match the number of target descriptors used to construct the list
        EXPECT_EQ(targetList.GetNumTargets(), m_numTargets);

        for (size_t i = 0; i < targetList.GetNumTargets(); i++)
        {
            // Expect the target to match the procedurally generated target descriptor
            ValidateTarget(targetList.GetTargets()[i], i);

            // Expect the target obtained by name to match the procedurally generated target descriptor
            auto target = targetList.GetTarget(GenerateBuildTargetName(i));
            EXPECT_TRUE(target);
            EXPECT_EQ(target->GetName(), GenerateBuildTargetName(i));
        }
    }

    TYPED_TEST(TargetListFixture, FindNonExistantTargets_ExpectEmptyResults)
    {
        // Given a valid set of target descriptor
        AZStd::vector<TargetType::Descriptor> descriptors;
        descriptors.reserve(m_numTargets);
        for (size_t i = 0; i < m_numTargets; i++)
        {
            descriptors.push_back(GenerateTargetDescriptor<TargetType>(i));
        }

        // When constructing the target list containing the valid target descriptors
        TargetListType targetList(AZStd::move(descriptors));

        // Expect the number of targets in the list to match the number of target descriptors used to construct the list
        EXPECT_EQ(targetList.GetNumTargets(), m_numTargets);

        for (size_t i = 0; i < targetList.GetNumTargets(); i++)
        {
            // When attempting to find a target that does not exist
            auto target = targetList.GetTarget(GenerateBuildTargetName(i + targetList.GetNumTargets()));

            // Expect an empty result
            EXPECT_FALSE(target);
        }
    }

    TYPED_TEST(TargetListFixture, FindNonExistantTargetsAndThrow_ExpectTargetExceptionss)
    {
        // Given a valid set of target descriptor
        AZStd::vector<TargetType::Descriptor> descriptors;
        descriptors.reserve(m_numTargets);
        for (size_t i = 0; i < m_numTargets; i++)
        {
            descriptors.push_back(GenerateTargetDescriptor<TargetType>(i));
        }

        // When constructing the target list containing the valid target descriptors
        TargetListType targetList(AZStd::move(descriptors));

        // Expect the number of targets in the list to match the number of target descriptors used to construct the list
        EXPECT_EQ(targetList.GetNumTargets(), m_numTargets);

        for (size_t i = 0; i < targetList.GetNumTargets(); i++)
        {
            try
            {
                // When attempting to find a target that does not exist
                targetList.GetTargetOrThrow(GenerateBuildTargetName(i + targetList.GetNumTargets()));

                // Do not expect the target list construction to succeed
                FAIL();
            }
            catch ([[maybe_unused]] const TestImpact::TargetException& e)
            {
                // Expect a target exception to be thrown
                SUCCEED();
            }
            catch (...)
            {
                // Do not expect any other exceptions
                FAIL();
            }
        }
    }
} // namespace UnitTest
