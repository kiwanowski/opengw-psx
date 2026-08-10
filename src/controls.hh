
#pragma once

#include "fixed.hh"
#include "psyqo/advancedpad.hh"

namespace gw {

class Controls {
  public:
    void init();

    void poll();

    Vec2 leftStick() const { return m_left; }

    Vec2 rightStick() const { return m_right; }

    bool trigger() const { return m_trigger; }
    bool triggerPressed() const { return m_trigger && !m_lastTrigger; }

    bool start() const { return m_start; }
    bool startPressed() const { return m_start && !m_lastStart; }

    bool analogConnected() const { return m_analog; }

  private:
    Vec2 readStick(unsigned xIndex, unsigned yIndex) const;

    psyqo::AdvancedPad m_pad;

    Vec2 m_left;
    Vec2 m_right;

    bool m_trigger = false, m_lastTrigger = false;
    bool m_start = false, m_lastStart = false;
    bool m_analog = false;
};

}
