#include "GlobalState.h"
#include <graphics.h>
#include <queue>
#include <algorithm>

static constexpr float SEND_INTERVAL = 1.0f;

void GlobalState::init() {
    float startX = 150;
    float gapY = 100;
    float gapX = 120;

    const int LAYERS = 3;

    // Player side
    for (int layer = 0; layer < LAYERS; ++layer) {
        for (int i = 0; i <= layer; ++i) {
            nodes.push_back(new Node(
                startX + layer * gapX,
                200 + i * gapY,
                layer,
                Owner::Player
            ));
        }
    }

    // Enemy side (mirrored)
    for (int layer = 0; layer < LAYERS; ++layer) {
        for (int i = 0; i <= layer; ++i) {
            nodes.push_back(new Node(
                800 - layer * gapX,
                200 + i * gapY,
                layer,
                Owner::Enemy
            ));
        }
    }
}

Node* GlobalState::pickNode(float x, float y) {
    for (Node* n : nodes)
        if (n->contains(x, y))
            return n;
    return nullptr;
}

Node* GlobalState::chooseTarget(Node* source) {
    int maxLayer = source->layer;
    std::vector<Node*> candidates;

    for (Node* n : source->edges) {
        if (n->layer > maxLayer) {
            maxLayer = n->layer;
            candidates.clear();
        }
        if (n->layer == maxLayer)
            candidates.push_back(n);
    }

    if (candidates.empty()) return nullptr;

    Node* target = candidates[source->roundRobinIndex % candidates.size()];
    source->roundRobinIndex++;
    return target;
}

void GlobalState::handleInput()
{
    graphics::MouseState ms;
    graphics::getMouseState(ms);

    if (ms.button_left_pressed)
    {
        float mx = (float)ms.cur_pos_x;
        float my = (float)ms.cur_pos_y;

        Node* clicked = pickNode(mx, my);

        if (!selectedNode)
        {
            selectedNode = clicked;
        }
        else
        {
            if (clicked && clicked != selectedNode)
            {
                // Create a directed edge
                selectedNode->edges.push_back(clicked);
            }
            selectedNode = nullptr;
        }
    }
}

void GlobalState::update(float dt) {
    dt *= 0.001f;
    handleInput();

    // Update nodes (production happens here now)
    for (Node* n : nodes) {
        n->update(dt);

        // Units move ONLY if connected; send rate-limited
        if (n->isConnected() && n->unitCount > 0) {
            float& t = sendTimers[n];
            t += dt;

            while (t >= SEND_INTERVAL && n->unitCount > 0) {
                t -= SEND_INTERVAL;

                Node* target = chooseTarget(n);
                if (!target) break;

                units.push_back(new Unit(n, target, n->owner));
                n->unitCount--;
            }
        } else {
            // if not connected, keep timer reset (optional)
            sendTimers[n] = 0.0f;
        }
    }

    // Update units + resolve arrivals
    for (auto it = units.begin(); it != units.end();) {
        Unit* u = *it;
        u->update(dt);

        if (u->arrived()) {
            if (u->to->owner == u->owner) {
                if (u->to->unitCount < u->to->capacity)
                    u->to->unitCount++;
            } else {
                u->to->unitCount--;
                if (u->to->unitCount <= 0) {
                    u->to->owner = u->owner;
                    u->to->unitCount = 1; // prevent staying at 0/-1
                }
            }

            delete u;
            it = units.erase(it);
        } else {
            ++it;
        }
    }
}

void GlobalState::draw() {
    for (Node* n : nodes) n->draw();
    for (Unit* u : units) u->draw();
}
