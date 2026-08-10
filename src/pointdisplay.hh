
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "render.hh"

namespace gw {

static constexpr int kMaxPointDisplays = 12;

static constexpr int kPointDisplayTime = 50;

static constexpr int kPointMessageLen = 16;

class PointDisplays {
  public:
    void init();

    void show(const Vec2& pos, const Pen& pen, const char* message);

    void run();
    void draw(Renderer& r);

  private:
    struct Display {
        Vec2 pos;
        Pen pen;
        char message[kPointMessageLen];
        int16_t timer = 0;
    };

    Display m_displays[kMaxPointDisplays];
};

}
