#pragma once

namespace RdkShell
{

struct InputEvent
{
    uint32_t deviceId;
    uint32_t timestampMs;
    enum Type { InvalidEvent, KeyEvent, TouchPadEvent, SliderEvent } type;

    union Details
    {
        struct Key
        {
            int code;
            enum State { Pressed, Released, VirtualPress, VirtualRelease } state;
        } key;

        struct TouchPad
        {
            int x, y;
            enum State { Down, Up, Click } state;
        } touchpad;

        struct Slider
        {
            int x;
            enum State { Down, Up } state;
        } slider;

    } details;

    InputEvent()
    : deviceId(0), timestampMs(0), type(InvalidEvent), details()
    { }

    InputEvent(uint32_t id, uint32_t ts, Type t)
    : deviceId(id), timestampMs(ts), type(t), details()
    { }
};

}
