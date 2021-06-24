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

#include <Activity/AWSGameLiftCreateSessionActivity.h>
#include <AWSGameLiftClientFixture.h>

using namespace AWSGameLift;

using AWSGameLiftCreateSessionActivityTest = AWSGameLiftClientFixture;

TEST_F(AWSGameLiftCreateSessionActivityTest, BuildAWSGameLiftCreateGameSessionRequest_Call_GetExpectedResult)
{
    AWSGameLiftCreateSessionRequest request;
    request.m_creatorId = "dummyCreatorId";
    request.m_sessionName = "dummySessionName";
    request.m_maxPlayer = 1;
    request.m_sessionProperties.emplace("dummyKey", "dummyValue");
    request.m_aliasId = "dummyAliasId";
    request.m_fleetId = "dummyFleetId";
    request.m_idempotencyToken = "dummyIdempotencyToken";
    auto awsRequest = CreateSessionActivity::BuildAWSGameLiftCreateGameSessionRequest(request);

    EXPECT_TRUE(strcmp(awsRequest.GetCreatorId().c_str(), request.m_creatorId.c_str()) == 0);
    EXPECT_TRUE(strcmp(awsRequest.GetName().c_str(), request.m_sessionName.c_str()) == 0);
    EXPECT_TRUE(awsRequest.GetMaximumPlayerSessionCount() == request.m_maxPlayer);
    EXPECT_TRUE(strcmp(awsRequest.GetGameProperties()[0].GetKey().c_str(), request.m_sessionProperties.begin()->first.c_str()) == 0);
    EXPECT_TRUE(strcmp(awsRequest.GetGameProperties()[0].GetValue().c_str(), request.m_sessionProperties.begin()->second.c_str()) == 0);
    EXPECT_TRUE(strcmp(awsRequest.GetAliasId().c_str(), request.m_aliasId.c_str()) == 0);
    EXPECT_TRUE(strcmp(awsRequest.GetFleetId().c_str(), request.m_fleetId.c_str()) == 0);
    EXPECT_TRUE(strcmp(awsRequest.GetIdempotencyToken().c_str(), request.m_idempotencyToken.c_str()) == 0);
}

TEST_F(AWSGameLiftCreateSessionActivityTest, ValidateCreateSessionRequest_CallWithBaseType_GetFalseResult)
{
    auto result = CreateSessionActivity::ValidateCreateSessionRequest(AzFramework::CreateSessionRequest());
    EXPECT_FALSE(result);   
}

TEST_F(AWSGameLiftCreateSessionActivityTest, ValidateCreateSessionRequest_CallWithNegativeMaxPlayer_GetFalseResult)
{
    AWSGameLiftCreateSessionRequest request;
    request.m_maxPlayer = -1;

    auto result = CreateSessionActivity::ValidateCreateSessionRequest(request);
    EXPECT_FALSE(result);
}

TEST_F(AWSGameLiftCreateSessionActivityTest, ValidateCreateSessionRequest_CallWithoutAliasOrFleetId_GetFalseResult)
{
    AWSGameLiftCreateSessionRequest request;
    request.m_maxPlayer = 1;
    auto result = CreateSessionActivity::ValidateCreateSessionRequest(request);
    EXPECT_FALSE(result);
}

TEST_F(AWSGameLiftCreateSessionActivityTest, ValidateCreateSessionRequest_CallWithAliasId_GetTrueResult)
{
    AWSGameLiftCreateSessionRequest request;
    request.m_maxPlayer = 1;
    request.m_aliasId = "dummyAliasId";
    auto result = CreateSessionActivity::ValidateCreateSessionRequest(request);
    EXPECT_TRUE(result);
}

TEST_F(AWSGameLiftCreateSessionActivityTest, ValidateCreateSessionRequest_CallWithFleetId_GetTrueResult)
{
    AWSGameLiftCreateSessionRequest request;
    request.m_maxPlayer = 1;
    request.m_fleetId = "dummyFleetId";
    auto result = CreateSessionActivity::ValidateCreateSessionRequest(request);
    EXPECT_TRUE(result);
}
