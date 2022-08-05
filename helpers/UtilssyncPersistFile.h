#pragma once

#include <core/core.h>

#include <stdio.h>

namespace Utils {
inline void syncPersistFile(const string file)
{
    FILE* fp = NULL;
    fp = fopen(file.c_str(), "r");
    if (fp == NULL) {
        printf("fopen NULL\n");
        return;
    }
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
}

inline void persistJsonSettings(const string strFile, const string strKey, const JsonValue& jsValue)
{
    WPEFramework::Core::File file;
    file = strFile.c_str();

    file.Open(false);
    if (!file.IsOpen())
        file.Create();

    JsonObject cecSetting;
    cecSetting.IElement::FromFile(file);
    file.Destroy();
    file.Create();
    cecSetting[strKey.c_str()] = jsValue;
    cecSetting.IElement::ToFile(file);

    file.Close();

    //Sync the settings
    Utils::syncPersistFile(strFile);

    return;
}
}
