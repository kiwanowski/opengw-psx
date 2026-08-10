
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "psyqo/fragments.hh"
#include "psyqo/gpu.hh"
#include "psyqo/primitives/lines.hh"

namespace gw {

static constexpr int kScreenWidth = 320;
static constexpr int kScreenHeight = 240;

static constexpr size_t kMaxLines = 3000;

static constexpr size_t kLinesPerChunk = 4;
static constexpr size_t kChunkShift = 2;
static constexpr size_t kChunkMask = kLinesPerChunk - 1;
static constexpr size_t kMaxChunks = (kMaxLines + kLinesPerChunk - 1) / kLinesPerChunk;

static constexpr int kStripSegments = 11;
static constexpr int kStripPoints = kStripSegments + 1;

static constexpr size_t kMaxStrips = 400;

struct Pen {
    uint8_t r = 255, g = 255, b = 255;
    uint8_t a = 255;
    uint8_t width = 1;

    constexpr Pen() = default;
    constexpr Pen(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255, uint8_t width_ = 1)
        : r(r_), g(g_), b(b_), a(a_), width(width_) {}

    constexpr Pen(const Pen& p, uint8_t a_, uint8_t width_)
        : r(p.r), g(p.g), b(p.b), a(a_), width(width_) {}

    [[gnu::always_inline]] inline psyqo::Color resolved() const {
        auto premul = [](uint8_t c, uint8_t alpha) -> uint8_t {
            return uint8_t((unsigned(c) * unsigned(alpha) * 257u + 32768u) >> 16);
        };
        return psyqo::Color{{.r = premul(r, a), .g = premul(g, a), .b = premul(b, a)}};
    }
};

class Renderer {
  public:
    void init(psyqo::GPU* gpu);

    void beginFrame();

    void endFrame();


    void setCamera(const Vec2& center, Fixed zoom);

    Fixed cameraScale() const { return m_scale; }

    void visibleBounds(Vec2& min, Vec2& max) const {
        Fixed halfW = Fixed(kScreenWidth / 2, 0) / m_scale;
        Fixed halfH = Fixed(kScreenHeight / 2, 0) / m_scale;
        min = Vec2(m_center.x - halfW, m_center.y - halfH);
        max = Vec2(m_center.x + halfW, m_center.y + halfH);
    }

    [[gnu::always_inline]] inline psyqo::Vertex project(const Vec2& p) const {
        Fixed sx = (p.x - m_center.x) * m_scale;
        Fixed sy = (p.y - m_center.y) * m_scale;
        return psyqo::Vertex{{.x = int16_t((sx.value >> 12) + kScreenWidth / 2),
                              .y = int16_t(kScreenHeight / 2 - (sy.value >> 12))}};
    }


    void line(const Vec2& from, const Vec2& to, const Pen& pen);

    void screenLine(psyqo::Vertex from, psyqo::Vertex to, const Pen& pen);

    void screenStrip(const psyqo::Vertex* points, int count, const Pen& pen);

    unsigned overflowCount() const { return m_overflow; }

    unsigned lineCount() const { return m_count + m_stripCount * kStripSegments; }

    unsigned primitiveCount() const { return m_count + m_stripCount; }

  private:
    [[gnu::always_inline]] inline bool cullSegment(psyqo::Vertex a, psyqo::Vertex b) const {
        if (a.x < 0 && b.x < 0) return true;
        if (a.y < 0 && b.y < 0) return true;
        if (a.x >= kScreenWidth && b.x >= kScreenWidth) return true;
        if (a.y >= kScreenHeight && b.y >= kScreenHeight) return true;
        return false;
    }

    void emit(psyqo::Vertex a, psyqo::Vertex b, psyqo::Color color, uint8_t width);

    using LineChunk = psyqo::Fragments::FixedFragment<psyqo::Prim::Line, kLinesPerChunk>;

    struct StripFragment {
        size_t getActualFragmentSize() const { return pointCount + 2; }

        unsigned pointCount = 0;
        uint32_t head;
        uint32_t command;
        psyqo::Vertex points[kStripPoints + 1];
    };
    using ModeFragment = psyqo::Fragments::SimpleFragment<psyqo::Prim::TPage>;
    using ClearFragment = psyqo::Fragments::SimpleFragment<psyqo::Prim::FastFill>;

    psyqo::GPU* m_gpu = nullptr;

    LineChunk m_chunks[2][kMaxChunks];
    StripFragment m_strips[2][kMaxStrips];
    ModeFragment m_modeFragment[2];
    ClearFragment m_clearFragment[2];

    unsigned m_stripCount = 0;

    unsigned m_parity = 0;
    unsigned m_count = 0;
    unsigned m_overflow = 0;

    Vec2 m_center;
    Fixed m_scale = Fixed(4, 0);
};

}
