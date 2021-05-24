#!/bin/bash
     
# All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
# its licensors.
#
# For complete copyright and license terms please see the LICENSE at the root of this
# distribution (the "License"). All use of this software is governed by the License,
# or, if provided, by the license below or the license accompanying this file. Do not
# remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 
# This script must be run as root
if [[ $EUID -ne 0 ]]
then
    echo "This script must be run as root (sudo)"
    exit 1
fi
 
# Install python3 if necessary
python3 --version >/dev/null 2>&1
if [ $? -ne 0 ]
then
    echo Installing Python3
    apt-get install python3

    if [ $? -ne 0 ]
    then
        echo Error installing python3
        exit 1
    fi

else
    PYTHON_VERSION=$(python3 --version)
    echo Python3 already installed \($PYTHON_VERSION\)
fi
 
# Install python3 pip if necessary
pip3 --version >/dev/null 2>&1
if [ $? -ne 0 ]
then
    echo Installing Python3 PIP
    apt-get install -y python3-pip

    if [ $? -ne 0 ]
    then
        echo Error installing python3
        exit 1
    fi

else
    PYTHON_VERSION=$(pip3 --version | awk '{print $2}')
    echo Python3 Pip already installed \($PYTHON_VERSION\)
fi
 
 
# Read from the package list and process each package
PIP_REQUIREMENTS_FILE=requirements.txt
 
pip3 install -r $PIP_REQUIREMENTS_FILE
if [ $? -ne 0 ]
then
    echo Error installing python3
    exit 1
fi


echo Python3 setup complete
exit 0
