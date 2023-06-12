#pragma once

#include "game.h"

class InputState
{
  protected:
    // Backend needs to set these
    friend class SdlBackend;

    // Things that are usually analog on controller
    glm::vec2 leftStick;  // WASD/left stick
    glm::vec2 rightStick; // mouse/right stick
    float leftTrigger;    // left trigger/mouse 2
    float rightTrigger;   // right trigger/mouse 1

    // Stores all the binary inputs
    uint16_t state;

    // Things that are always binary on controller (the letter buttons are
    // based on the North American layout, keys are position on QWERTY)
    const uint16_t START = 0b0000000000000001;          // ESC
    const uint16_t SELECT = 0b0000000000000010;         // Tab
    const uint16_t DPAD_UP = 0b0000000000000100;        // Q
    const uint16_t DPAD_DOWN = 0b0000000000001000;      // C
    const uint16_t DPAD_LEFT = 0b0000000000010000;      // X
    const uint16_t DPAD_RIGHT = 0b0000000000100000;     // V
    const uint16_t A = 0b0000000001000000;              // Space
    const uint16_t B = 0b0000000010000000;              // F
    const uint16_t X = 0b0000000100000000;              // E
    const uint16_t Y = 0b0000001000000000;              // R
    const uint16_t LEFT_SHOULDER = 0b0000010000000000;  // Scroll up
    const uint16_t RIGHT_SHOULDER = 0b0000100000000000; // Scroll down
    const uint16_t LEFT_STICK = 0b0001000000000000;     // Shift
    const uint16_t RIGHT_STICK = 0b0010000000000000;    // Left control

    // A lot of getters
  public:
    InputState()
        : leftStick(0), rightStick(0), leftTrigger(0), rightTrigger(0), state(0)
    {
    }

    std::string GetStateDescription() const
    {
        return fmt::format(
            "left={}, {}\tright={}, {}\ttriggers={}, {}\tstart={}, select={}, "
            "up={}, down={}, left={}, right={}, A={}, B={}, X={}, Y={}, "
            "shoulders={}, {}, stick buttons={}, {}",
            leftStick.x, leftStick.y, rightStick.x, rightStick.y, leftTrigger,
            rightTrigger, GetStart(), GetSelect(), GetDpadUp(), GetDpadDown(),
            GetDpadLeft(), GetDpadRight(), GetA(), GetB(), GetX(), GetY(),
            GetLeftShoulder(), GetRightShoulder(), GetLeftStickPressed(),
            GetRightStickPressed());
    }

    const glm::vec2& GetLeftStickDirection() const
    {
        return leftStick;
    }

    const glm::vec2& GetRightStickDirection() const
    {
        return rightStick;
    }

    const float GetLeftTrigger() const
    {
        return leftTrigger;
    }

    const float GetRightTrigger() const
    {
        return rightTrigger;
    }

    const bool GetStart() const
    {
        return state & START;
    }

    const bool GetSelect() const
    {
        return state & SELECT;
    }

    const bool GetDpadUp() const
    {
        return state & DPAD_UP;
    }

    const bool GetDpadDown() const
    {
        return state & DPAD_DOWN;
    }

    const bool GetDpadLeft() const
    {
        return state & DPAD_LEFT;
    }

    const bool GetDpadRight() const
    {
        return state & DPAD_RIGHT;
    }

    const bool GetA() const
    {
        return state & A;
    }

    const bool GetB() const
    {
        return state & B;
    }

    const bool GetX() const
    {
        return state & X;
    }

    const bool GetY() const
    {
        return state & Y;
    }

    const bool GetLeftShoulder() const
    {
        return state & LEFT_SHOULDER;
    }

    const bool GetRightShoulder() const
    {
        return state & RIGHT_SHOULDER;
    }

    const bool GetLeftStickPressed() const
    {
        return state & LEFT_STICK;
    }

    const bool GetRightStickPressed() const
    {
        return state & RIGHT_STICK;
    }
};
