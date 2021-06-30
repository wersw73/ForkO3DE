/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "precompiled.h"

#include <AzTest/AzTest.h>

class ScriptCanvasDeveloperTest
    : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

};

TEST_F(ScriptCanvasDeveloperTest, Sanity_Pass)
{
    EXPECT_TRUE(true);
}


AZ_UNIT_TEST_HOOK();
