#include "Node.h"
#include <graphics.h>
#include <cmath>
#include <string>

static constexpr float NODE_RADIUS = 20.0f;
static constexpr float PRODUCE_INTERVAL = 0.5f; // 1 unit per 0.5 sec

Node::Node(float x, float y, int layer, Owner owner)
    : x(x), y(y), layer(layer), owner(owner)
{
    unitCount = 0;
    capacity = 50;
}

void Node::update(float dt) {
    const bool canProduce = (layer == 0) || isConnected();
        if (!canProduce)
            return;

    productionTimer += dt;

    while (productionTimer >= PRODUCE_INTERVAL) {
        productionTimer -= PRODUCE_INTERVAL;
        if (unitCount < capacity) {
            unitCount += 1;
        }
    }
}

void Node::draw() {
    // --- Draw node circle ---
    graphics::Brush br;
    br.outline_opacity = 1.0f;

    if (owner == Owner::Player) {
        br.fill_color[0] = 0.2f;
        br.fill_color[1] = 0.6f;
        br.fill_color[2] = 1.0f;
    } else {
        br.fill_color[0] = 1.0f;
        br.fill_color[1] = 0.3f;
        br.fill_color[2] = 0.3f;
    }

    graphics::drawDisk(x, y, 20.0f, br);

    // --- Draw unit count (WHITE text) ---
    graphics::Brush text;
    text.fill_color[0] = 1.0f;
    text.fill_color[1] = 1.0f;
    text.fill_color[2] = 1.0f;
    text.outline_opacity = 0.0f;

    std::string s = std::to_string(unitCount);

    // crude but effective centering
    float textX = x - 4.0f * s.size();
    float textY = y + 5.0f;

    graphics::drawText(textX, textY, 14.0f, s, text);

    // --- Draw outgoing edges ---
    graphics::Brush line;
    line.fill_color[0] = line.fill_color[1] = line.fill_color[2] = 0.9f;

    for (Node* n : edges) {
        graphics::drawLine(x, y, n->x, n->y, line);
    }
}

bool Node::contains(float mx, float my) const {
    float dx = mx - x;
    float dy = my - y;
    return (dx * dx + dy * dy) <= (NODE_RADIUS * NODE_RADIUS);
}
