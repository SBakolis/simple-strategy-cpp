#include "Unit.h"
#include <graphics.h>

Unit::Unit(Node* from, Node* to, Owner owner)
    : from(from), to(to), owner(owner) {}

void Unit::update(float dt) {
    t += dt * 0.5f;
    if (t > 1.0f) t = 1.0f;
}

bool Unit::arrived() const {
    return t >= 1.0f;
}

void Unit::draw() {
    float x = from->x + (to->x - from->x) * t;
    float y = from->y + (to->y - from->y) * t;

    graphics::Brush br;
    br.fill_color[0] = owner == Owner::Player ? 0.2f : 1.0f;
    br.fill_color[1] = owner == Owner::Player ? 1.0f : 0.2f;
    br.fill_color[2] = 0.2f;

    graphics::drawDisk(x, y, 5, br);
}
