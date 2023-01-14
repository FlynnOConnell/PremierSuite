#pragma once
#include "PremierSuite.h"

struct ThemeManager {
    
public:
    enum Theme {
        Default = 0,
        DarkDefault,
        LightDefault,
        DarkAlt1,
        DarkAlt2,
        Valve,
        Theme_Count
    };

    Theme m_theme = Default;
    ThemeManager();
    ~ThemeManager();
    void cycle();
    void setTheme(Theme theme);
public:
    
    //void LoadFromDisk(std::string fileName);
    //void SaveToDisk(std::string fileName);
};