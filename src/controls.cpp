#include "controls.hh"

namespace gw {

using Pad = psyqo::AdvancedPad::Pad;
using Button = psyqo::AdvancedPad::Button;

static constexpr unsigned kAdcRightX = 0;
static constexpr unsigned kAdcRightY = 1;
static constexpr unsigned kAdcLeftX = 2;
static constexpr unsigned kAdcLeftY = 3;

static constexpr Fixed kDeadzone = 0.2L;

void Controls::init() { m_pad.initialize(); }

Vec2 Controls::readStick(unsigned xIndex, unsigned yIndex) const {
    int rawX = int(m_pad.getAdc(Pad::Pad1a, xIndex)) - 128;
    int rawY = int(m_pad.getAdc(Pad::Pad1a, yIndex)) - 128;

    Vec2 v(Fixed(int32_t(rawX << 12) / 128, Fixed::RAW), Fixed(int32_t(-rawY << 12) / 128, Fixed::RAW));

    if (v.lengthSquared() < kDeadzone * kDeadzone) return Vec2();

    return mathutils::clamp(v, Fixed(1, 0));
}

void Controls::poll() {
    m_lastTrigger = m_trigger;
    m_lastStart = m_start;

    uint8_t type = m_pad.getPadType(Pad::Pad1a);
    m_analog = (type == psyqo::AdvancedPad::PadType::AnalogPad ||
                type == psyqo::AdvancedPad::PadType::AnalogStick);

    if (m_analog) {
        m_left = readStick(kAdcLeftX, kAdcLeftY);
        m_right = readStick(kAdcRightX, kAdcRightY);
    } else {
        auto axis = [this](Button neg, Button pos) -> Fixed {
            Fixed v;
            if (m_pad.isButtonPressed(Pad::Pad1a, pos)) v += Fixed(1, 0);
            if (m_pad.isButtonPressed(Pad::Pad1a, neg)) v -= Fixed(1, 0);
            return v;
        };

        Vec2 move(axis(Button::Left, Button::Right), axis(Button::Down, Button::Up));
        Vec2 aim(axis(Button::Square, Button::Circle), axis(Button::Cross, Button::Triangle));

        m_left = move.lengthSquared().value ? move.normalized() : Vec2();
        m_right = aim.lengthSquared().value ? aim.normalized() : Vec2();
    }

    m_trigger = m_pad.isButtonPressed(Pad::Pad1a, Button::L1) ||
                m_pad.isButtonPressed(Pad::Pad1a, Button::R1);
    m_start = m_pad.isButtonPressed(Pad::Pad1a, Button::Start);
}

}
