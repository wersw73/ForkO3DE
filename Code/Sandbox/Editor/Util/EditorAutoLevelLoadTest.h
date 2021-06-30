/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#ifndef CRYINCLUDE_EDITOR_UTIL_EDITORAUTOLEVELLOADTEST_H
#define CRYINCLUDE_EDITOR_UTIL_EDITORAUTOLEVELLOADTEST_H
#pragma once


class CEditorAutoLevelLoadTest
    : public IEditorNotifyListener
{
public:
    static CEditorAutoLevelLoadTest& Instance();
private:
    CEditorAutoLevelLoadTest();
    virtual ~CEditorAutoLevelLoadTest();

    virtual void OnEditorNotifyEvent(EEditorNotifyEvent event);
};

#endif // CRYINCLUDE_EDITOR_UTIL_EDITORAUTOLEVELLOADTEST_H
