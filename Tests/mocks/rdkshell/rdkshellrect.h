#pragma once

namespace RdkShell
{
    struct RdkShellRect
    {
        public:
            RdkShellRect(): x(0), y(0), width(0), height(0) {}
            RdkShellRect(uint32_t xval, uint32_t yval, uint32_t w, uint32_t h):x(xval), y(yval), width(w), height(h) {} 
            uint32_t x;
            uint32_t y;
            uint32_t width;
            uint32_t height;
    };
}
