#pragma once

#include "game.h"

namespace Input
{
// Things that are always binary on controller (the letter buttons are
// based on the North American layout, keys are position on QWERTY)
static constexpr uint16_t START = 0b0000000000000001;          // ESC
static constexpr uint16_t SELECT = 0b0000000000000010;         // Tab
static constexpr uint16_t DPAD_UP = 0b0000000000000100;        // Q
static constexpr uint16_t DPAD_DOWN = 0b0000000000001000;      // C
static constexpr uint16_t DPAD_LEFT = 0b0000000000010000;      // X
static constexpr uint16_t DPAD_RIGHT = 0b0000000000100000;     // V
static constexpr uint16_t A = 0b0000000001000000;              // Space
static constexpr uint16_t B = 0b0000000010000000;              // F
static constexpr uint16_t X = 0b0000000100000000;              // E
static constexpr uint16_t Y = 0b0000001000000000;              // R
static constexpr uint16_t LEFT_SHOULDER = 0b0000010000000000;  // Scroll up
static constexpr uint16_t RIGHT_SHOULDER = 0b0000100000000000; // Scroll down
static constexpr uint16_t LEFT_STICK = 0b0001000000000000;     // Left control
static constexpr uint16_t RIGHT_STICK = 0b0010000000000000;    // Left shift

// FIXME: hardcoded deadzones based on my specific controller
static constexpr float LEFT_STICK_MIN_X = 0.3f;
static constexpr float LEFT_STICK_MAX_X = 1.0f;
static constexpr float LEFT_STICK_MIN_Y = 0.3f;
static constexpr float LEFT_STICK_MAX_Y = 1.0f;
static constexpr float RIGHT_STICK_MIN_X = 0.3f;
static constexpr float RIGHT_STICK_MAX_X = 1.0f;
static constexpr float RIGHT_STICK_MIN_Y = 0.3f;
static constexpr float RIGHT_STICK_MAX_Y = 1.0f;

class State
{
  public:
    // Things that are usually analog on controller
    PxVec2 leftStick;  // WASD/left stick
    PxVec2 rightStick; // mouse/right stick
    float leftTrigger;    // left trigger/mouse 2
    float rightTrigger;   // right trigger/mouse 1
    float scrollAmount;   // shoulders/mouse wheel

    // Stores all the binary inputs
    uint16_t state;

    // Probably better ways to do this
    template <typename T> T Deadzone(T value, T min, T max)
    {
        if (std::abs(value) < min)
        {
            return 0;
        }
        return std::clamp(value, -max, max);
    }

    State()
        : leftStick(0), rightStick(0), leftTrigger(0), rightTrigger(0),
          scrollAmount(0), state(0)
    {
    }

    std::string DescribeState() const
    {
        return fmt::format(
            "left={:.03}, {:.03}\tright={:.03}, {:.03}\ttriggers={:.03}, "
            "{:.03}\tstart={}, select={}, "
            "up={}, down={}, left={}, right={}, A={}, B={}, X={}, Y={}, "
            "shoulders={}, {}, stick buttons={}, {}, scroll amount={}",
            GetLeftStickDirection().x, GetLeftStickDirection().y,
            GetRightStickDirection().x, GetRightStickDirection().y,
            GetLeftTrigger(), GetRightTrigger(), GetStart(), GetSelect(),
            GetDpadUp(), GetDpadDown(), GetDpadLeft(), GetDpadRight(), GetA(),
            GetB(), GetX(), GetY(), GetLeftShoulder(), GetRightShoulder(),
            GetLeftStickPressed(), GetRightStickPressed(), GetScrollAmount());
    }

    // Constrain sticks to deadzones
    void AdjustSticks()
    {
        leftStick.x = Deadzone(leftStick.x, LEFT_STICK_MIN_X, LEFT_STICK_MAX_X);
        leftStick.y = Deadzone(leftStick.y, LEFT_STICK_MIN_Y, LEFT_STICK_MAX_Y);
        rightStick.x =
            Deadzone(rightStick.x, RIGHT_STICK_MIN_X, RIGHT_STICK_MAX_X);
        rightStick.y =
            Deadzone(rightStick.y, RIGHT_STICK_MIN_Y, RIGHT_STICK_MAX_Y);
        scrollAmount = Deadzone(scrollAmount, 0.0f, 1.0f);
    }

    // A lot of getters

    const PxVec2& GetLeftStickDirection() const
    {
        return leftStick;
    }

    const PxVec2& GetRightStickDirection() const
    {
        return rightStick;
    }

    float GetLeftTrigger() const
    {
        return leftTrigger;
    }

    float GetRightTrigger() const
    {
        return rightTrigger;
    }

    float GetScrollAmount() const
    {
        return scrollAmount;
    }

    void ResetScrollAmount()
    {
        scrollAmount = 0.0f;
    }

    bool GetStart() const
    {
        return state & START;
    }

    bool GetSelect() const
    {
        return state & SELECT;
    }

    bool GetDpadUp() const
    {
        return state & DPAD_UP;
    }

    bool GetDpadDown() const
    {
        return state & DPAD_DOWN;
    }

    bool GetDpadLeft() const
    {
        return state & DPAD_LEFT;
    }

    bool GetDpadRight() const
    {
        return state & DPAD_RIGHT;
    }

    bool GetA() const
    {
        return state & A;
    }

    bool GetB() const
    {
        return state & B;
    }

    bool GetX() const
    {
        return state & X;
    }

    bool GetY() const
    {
        return state & Y;
    }

    bool GetLeftShoulder() const
    {
        return state & LEFT_SHOULDER;
    }

    bool GetRightShoulder() const
    {
        return state & RIGHT_SHOULDER;
    }

    bool GetLeftStickPressed() const
    {
        return state & LEFT_STICK;
    }

    bool GetRightStickPressed() const
    {
        return state & RIGHT_STICK;
    }
};

} // namespace Input