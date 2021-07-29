"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT

Self-tests for ctest_driver.py.

Expects you to have generated a folder using cmake and to pass in that folder name
as the only param
"""
import os
import subprocess
import sys
import argparse
from ctest_driver import SUITES_AND_DESCRIPTIONS

def main(build_path, ctest_executable):
    script_folder = os.path.dirname(__file__)
    # -N prevents tests from running, just lists them:
    base_args = [sys.executable, os.path.join(script_folder,'ctest_driver.py'), "--build-path", build_path, '-N']
    if ctest_executable:
        base_args.append("--ctest-executable")
        base_args.append(ctest_executable)

    base_args.append("-R") # limit it to only the sanity tests, faster
    base_args.append("pytest_sanity_.*")

    for suite in SUITES_AND_DESCRIPTIONS:
        for gpu_option in [None, True, False]:
            args_to_send = base_args.copy()
            args_to_send.append("--suite")
            args_to_send.append(suite)
            print(f"----- Test case: [suite = {suite}, gpu = {gpu_option}] ----")
            if gpu_option is not None:
                if gpu_option:
                    args_to_send.append("--only-gpu")
                else:
                    args_to_send.append("--no-gpu")
            result = subprocess.check_output(args_to_send, shell=False, cwd=build_path, stderr=sys.stderr)
            output = result.decode('utf-8')

            # ensure that the appropriate suites are filtered in and out
            for suite_name in SUITES_AND_DESCRIPTIONS.keys():
                if (f"pytest_sanity_{suite_name}_" in output and suite != suite_name):
                    print(output)
                    print("Test failed - executed a test not in the currently selected suite.")
                    return -1

            gpu_test_present = "_requires_gpu" in output
            non_gpu_test_present = "_no_gpu" in output

            if gpu_option is not None:
                if gpu_option and non_gpu_test_present:
                    print("Test failed - required gpu only, and a non-gpu tes was run")
                    print(output)
                    return -1

                if not gpu_option and gpu_test_present:
                    print("Test failed - executed a gpu test when forbidden")
                    print(output)
                    return -1
            else:
                if not (gpu_test_present and non_gpu_test_present):
                    print("Test failed - expected both kinds of tests (gpu and non gpu)")
                    print(output)
                    return -1

    print("All passed.")
    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="CTest CLI driver self-tests.")
    parser.add_argument('-x', '--ctest-executable', 
                        help="Override path to the CTest executable (will use PATH env otherwise)")
    parser.add_argument('-b', '--build-path', 
                        required=True,
                        help="Path to a CMake build folder (generated by running cmake)")
    args = parser.parse_args()
    sys.exit(main(args.build_path, args.ctest_executable))


