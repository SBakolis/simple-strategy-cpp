#include "Node.h"
#include <graphics.h>
#include <cmath>
#include <string>

static constexpr float NODE_RADIUS = 22.0f;
static constexpr float PRODUCE_INTERVAL = 3.0f; // seconds per unit

Node::Node(float x, float y, int layer, Owner owner)
    : x(x), y(y), layer(layer), owner(owner)
{
    unitCount = 0;
    capacity = 50;
    productionTimer = 0.0f;
    roundRobinIndex = 0;
}

void Node::update(float dt)
{
    // Base always produces, others only if connected
    bool canProduce = (layer == 0) || !edges.empty();
    if (!canProduce)
        return;

    productionTimer += dt;

    while (productionTimer >= PRODUCE_INTERVAL) {
        productionTimer -= PRODUCE_INTERVAL;
        if (unitCount < capacity) {
            unitCount++;
        }
    }
}

void Node::draw()
{
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

    graphics::drawDisk(x, y, NODE_RADIUS, br);

    graphics::Brush text;
    text.fill_color[0] = 1.0f;
    text.fill_color[1] = 1.0f;
    text.fill_color[2] = 1.0f;
    text.outline_opacity = 0.0f;

    std::string s = std::to_string(unitCount);

    float textX = x - 4.5f * (float)s.size();
    float textY = y + 6.0f;

    graphics::drawText(textX, textY, 16.0f, s, text);

    graphics::Brush line;
    line.fill_color[0] = 0.85f;
    line.fill_color[1] = 0.85f;
    line.fill_color[2] = 0.85f;

    for (Node* n : edges) {
        if (!n) continue;
        graphics::drawLine(x, y, n->x, n->y, line);
    }
}

bool Node::contains(float mx, float my) const
{
    float dx = mx - x;
    float dy = my - y;
    return (dx * dx + dy * dy) <= (NODE_RADIUS * NODE_RADIUS);
}
