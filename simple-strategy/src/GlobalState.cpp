#include "GlobalState.h"
#include <graphics.h>
#include <queue>
#include <algorithm>
#include <unordered_set>

static constexpr float SEND_INTERVAL = 1.0f;
const float WINDOW_W = 1200.0f;

void GlobalState::init() {

    float startX = 150;
    float gapY = 100;
    float gapX = 120;

    const int LAYERS = 3;

    // Player side
    for (int layer = 0; layer < LAYERS; ++layer) {
        for (int i = 0; i <= layer; ++i) {
            Node* n = new Node(
                startX + layer * gapX,
                200 + i * gapY,
                layer,
                Owner::Player
            );
            nodes.push_back(n);

            if (layer == 0 && i == 0) playerBase = n;
        }
    }

    // Enemy side
    for (int layer = 0; layer < LAYERS; ++layer) {
        for (int i = 0; i <= layer; ++i) {
            Node* n = new Node(
                WINDOW_W - startX - layer * gapX,
                200 + i * gapY,
                layer,
                Owner::Enemy
            );
            nodes.push_back(n);

            if (layer == 0 && i == 0) enemyBase = n;
        }
    }
}

bool GlobalState::edgeExists(Node* from, Node* to) const {
    if (!from || !to) return false;
    for (Node* n : from->edges)
        if (n == to) return true;
    return false;
}

// Node has an active chain to base = reachable from playerBase through directed edges
bool GlobalState::hasChainToPlayerBase(Node* n) const {
    if (!playerBase || !n) return false;
    if (n == playerBase) return true;

    std::queue<Node*> q;
    std::unordered_set<Node*> visited;

    q.push(playerBase);
    visited.insert(playerBase);

    while (!q.empty()) {
        Node* cur = q.front();
        q.pop();

        for (Node* nxt : cur->edges) {
            if (!nxt) continue;
            if (visited.count(nxt)) continue;

            visited.insert(nxt);
            if (nxt == n) return true;

            q.push(nxt);
        }
    }
    return false;
}

bool GlobalState::canCreateEdge(Node* from, Node* to) const {
    if (!from || !to) return false;
    if (from == to) return false;

    if (from->owner != Owner::Player) return false;
    if (to->owner   != Owner::Player) return false;

    // Must have active connection path
    if (!hasChainToPlayerBase(from)) return false;

    // Only same level or next level
    if (!(to->layer == from->layer || to->layer == from->layer + 1))
        return false;

    // Prevent duplicates
    if (edgeExists(from, to)) return false;

    return true;
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
            if (clicked && clicked->owner == Owner::Player && hasChainToPlayerBase(clicked)) {
                selectedNode = clicked;
            } else {
                selectedNode = nullptr;
            }
        } else {
            if (clicked && clicked != selectedNode)
            {
                selectedNode->edges.push_back(clicked);
            }
            selectedNode = nullptr;
        }
    }
}

void GlobalState::update(float dt) {
    dt *= 0.001f;
    handleInput();

    for (Node* n : nodes) {
        n->update(dt);

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
            sendTimers[n] = 0.0f;
        }
    }

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
