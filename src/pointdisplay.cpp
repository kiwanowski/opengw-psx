#include "pointdisplay.hh"

#include "font.hh"

namespace gw {

static constexpr Fixed kPointDisplayScale = 1.4L;

void PointDisplays::init() {
    for (auto& d : m_displays) d.timer = 0;
}

void PointDisplays::show(const Vec2& pos, const Pen& pen, const char* message) {
    for (auto& d : m_displays) {
        if (d.timer > 0) continue;

        d.pos = pos;
        d.pen = pen;
        d.timer = kPointDisplayTime;

        int i = 0;
        for (; message[i] && i < kPointMessageLen - 1; i++) d.message[i] = message[i];
        d.message[i] = '\0';
        return;
    }
}

void PointDisplays::run() {
    for (auto& d : m_displays) {
        if (d.timer > 0) --d.timer;
    }
}

void PointDisplays::draw(Renderer& r) {
    for (const auto& d : m_displays) {
        if (d.timer <= 0) continue;

        Fixed percent = Fixed(d.timer, 0) / kPointDisplayTime * 2;
        if (percent > Fixed(1, 0)) percent = Fixed(1, 0);

        Pen pen = d.pen;
        pen.a = uint8_t((Fixed(d.pen.a, 0) * percent).value >> 12);

        font::printWorld(r, font::ALIGN_CENTER, d.pos, kPointDisplayScale * percent, pen,
                         d.message);
    }
}

}
