#include "frontends/libretro/joypad.h"
#include "frontends/libretro/environment.h"

#include "libretro.h"

namespace ra2
{

  Joypad::Joypad(unsigned device)
    : JoypadBase(device)
    , myAxisCodes(2)
  {
    myAxisCodes[0][RETRO_DEVICE_ID_JOYPAD_LEFT] = -1.0;
    myAxisCodes[0][RETRO_DEVICE_ID_JOYPAD_RIGHT] = 1.0;
    myAxisCodes[1][RETRO_DEVICE_ID_JOYPAD_UP] = -1.0;
    myAxisCodes[1][RETRO_DEVICE_ID_JOYPAD_DOWN] = 1.0;
  }

  double Joypad::getAxis(int i) const
  {
    for (const auto & axis : myAxisCodes[i])
    {
      const int value = input_state_cb(0, myDevice, 0, axis.first);
      if (value)
      {
        return axis.second;
      }
    }

    return 0.0;
  }

}
