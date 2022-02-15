"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT

"""

# This suite consists of all test cases that are passing and have been verified.

import pytest
import os
import sys

from ly_test_tools.o3de.editor_test import EditorTestSuite, EditorSingleTest

@pytest.mark.parametrize("project", ["AutomatedTesting"])
@pytest.mark.parametrize("launcher_platform", ['windows_editor'])
class TestAutomation(EditorTestSuite):

    class Time_EditorLevelLoading_10KEntityCpuPerfTest(EditorSingleTest):
        # there is currently a huge discrepancy loading this level with vulkan compared to dx12 which requires the 10 min timeout
        # this should be removed once that issue has been sorted out
        timeout = 600
        extra_cmdline_args = ['-rhi=vulkan']
        use_null_renderer = False  # needs renderer to validate test

        from .tests import EditorLevelLoading_10KEntityCpuPerfTest as test_module

    class Time_EditorLevelLoading_10kVegInstancesTest(EditorSingleTest):
        extra_cmdline_args = ['-rhi=vulkan']
        use_null_renderer = False  # needs renderer to validate test

        from .tests import EditorLevelLoading_10kVegInstancesTest as test_module
