#pragma once

#include <stdlib.hpp>

class Ps2
{
public:
    void initialize();

    uint8 get_current_key();
    uint8 get_scancode(bool ignoreReleases);
    uint8 read_ascii();

    void operator>>(uint8& var);

private:
    bool shift_pressed;
    bool left_ctrl_pressed;
    bool caps_lock_enabled;
};
