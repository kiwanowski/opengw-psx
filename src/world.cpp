#include "world.hh"

namespace gw {

World g_world;

static constexpr int kBorder = 20;

static constexpr Fixed kEase = 1.0L / 30.0L;

void Camera::center() {
    currentZoom = kZoomedIn;
    targetZoom = kZoomedIn;
    targetPos = Vec2(Fixed(Grid::extentX() - 1, 0) / 2, Fixed(Grid::extentY() - 1, 0) / 2);
    currentPos = targetPos;
}

void Camera::follow(const Vec2& pos) {
    Fixed ax = pos.x / Fixed(Grid::extentX(), 0);
    Fixed ay = pos.y / Fixed(Grid::extentY(), 0);

    targetPos.x = ax * Fixed(Grid::extentX() - kBorder * 2, 0) + Fixed(kBorder, 0);
    targetPos.y = ay * Fixed(Grid::extentY() - kBorder * 2, 0) + Fixed(kBorder, 0);
}

void Camera::run() {
    currentZoom += (targetZoom - currentZoom) * kEase;
    currentPos += (targetPos - currentPos) * kEase;
}

}
