#pragma once

#include <stdlib.hpp>

class Ps2
{
public:
    void initialize();

    uint8 getCurrentKey();
    uint8 getScancode(bool ignoreReleases);
    uint8 readAscii();

    void operator>>(uint8& var);

private:
    bool shiftPressed;
    bool leftCtrlPressed;
    bool capsLockEnabled;
};
