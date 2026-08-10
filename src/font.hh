
#pragma once

#include "fixed.hh"
#include "render.hh"

namespace gw {
namespace font {

enum Align { ALIGN_LEFT = -1, ALIGN_CENTER = 0, ALIGN_RIGHT = 1 };

static constexpr Fixed kCellWidth = 1.9L;
static constexpr Fixed kCellHeight = 3.5L;

Fixed measure(const char* text, Fixed scale);

void printScreen(Renderer& r, Align align, int x, int y, Fixed scale, const Pen& pen,
                 const char* text);

void printWorld(Renderer& r, Align align, const Vec2& pos, Fixed scale, const Pen& pen,
                const char* text);

const char* withCommas(int value);

char* formatInt(char* out, int size, int value);

}
}
