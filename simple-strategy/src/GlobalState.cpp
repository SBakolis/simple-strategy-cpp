#include "GlobalState.h"
#include <graphics.h>
#include <queue>
#include <unordered_set>
#include <cmath>
#include <string>
#include <cstdlib>

static constexpr float SEND_INTERVAL = 1.0f;
static constexpr float WINDOW_W = 1200.0f;

static std::string statusText = "Click a NODE to select.";

GlobalState::~GlobalState()
{
    for (Unit* u : units) {
        delete u;
    }

    units.clear();

    for (Node* n : nodes) {
        delete n;
    }
    nodes.clear();

    selectedNode = nullptr;
    playerBase = nullptr;
    enemyBase = nullptr;
}

void GlobalState::init()
{
    float startX = 250.0f;
    float gapY   = 100.0f;
    float gapX   = 120.0f;

    const int LAYERS = 3;

    // Blue
    for (int layer = 0; layer < LAYERS; ++layer) {
        for (int i = 0; i <= layer; ++i) {
            Node* n = new Node(
                startX + layer * gapX,
                200.0f + i * gapY,
                layer,
                Owner::Player
            );
            nodes.push_back(n);
            if (layer == 0 && i == 0) playerBase = n;
        }
    }

    // Red mirrored
    for (int layer = 0; layer < LAYERS; ++layer) {
        for (int i = 0; i <= layer; ++i) {
            Node* n = new Node(
                WINDOW_W - startX - layer * gapX,
                200.0f + i * gapY,
                layer,
                Owner::Enemy
            );
            nodes.push_back(n);
            if (layer == 0 && i == 0) enemyBase = n;
        }
    }
}

Node* GlobalState::getBase(Owner owner) const
{
    return (owner == Owner::Player) ? playerBase : enemyBase;
}

Node* GlobalState::pickNode(float x, float y) const
{
    for (Node* n : nodes)
        if (n && n->contains(x, y))
            return n;
    return nullptr;
}

bool GlobalState::edgeExistsUndirected(Node* a, Node* b) const
{
    for (Node* n : a->edges) if (n == b) return true;
    for (Node* n : b->edges) if (n == a) return true;
    return false;
}

void GlobalState::createSharedConnection(Node* a, Node* b)
{
    if (!a || !b || a == b) return;
    if (edgeExistsUndirected(a, b)) return;

    a->edges.push_back(b);
    b->edges.push_back(a);
}

bool GlobalState::hasChainToBase(Node* n, Owner owner) const
{
    Node* base = getBase(owner);
    if (!n || !base) return false;
    if (n->owner != owner) return false;
    if (n == base) return true;

    std::queue<Node*> q;
    std::unordered_set<Node*> visited;

    q.push(base);
    visited.insert(base);

    while (!q.empty()) {
        Node* cur = q.front();
        q.pop();

        for (Node* nxt : cur->edges) {
            if (!nxt || visited.count(nxt)) continue;
            if (nxt->owner != owner) continue;

            if (nxt == n) return true;
            visited.insert(nxt);
            q.push(nxt);
        }
    }
    return false;
}

bool GlobalState::canCreateEdge(Node* from, Node* to, Owner owner) const
{
    if (!from || !to || from == to) return false;
    if (from->owner != owner) return false;

    Node* base = getBase(owner);
    if (!base) return false;

    // must be supplied or be base
    if (from != base && !hasChainToBase(from, owner)) return false;

    // same layer or ±1 layer
    if (std::abs(to->layer - from->layer) > 1) return false;

    if (edgeExistsUndirected(from, to)) return false;

    return true;
}

Node* GlobalState::chooseTarget(Node* source)
{
    if (!source || source->edges.empty()) return nullptr;

    const bool wantRight = (source->owner == Owner::Player);

    std::vector<Node*> forward;
    std::vector<Node*> same;

    for (Node* n : source->edges) {
        if (!n) continue;

        // same-level redistribution
        if (n->owner == source->owner &&
            n->layer == source->layer &&
            n->unitCount + 2 <= source->unitCount)
        {
            same.push_back(n);
        }

        // forward direction
        bool isForward = wantRight ? (n->x > source->x + 0.5f)
                                   : (n->x < source->x - 0.5f);
        if (isForward) {
            forward.push_back(n);
        }
    }

    auto pickFrom = [&](std::vector<Node*>& v) -> Node* {
        if (v.empty()) return nullptr;
        Node* t = v[source->roundRobinIndex % (int)v.size()];
        source->roundRobinIndex++;
        return t;
    };

    if (!forward.empty() && !same.empty()) {
        Node* target = nullptr;
        if (source->sendPhase == 0) {
            target = pickFrom(forward);
            source->sendPhase = 1;
        } else {
            target = pickFrom(same);
            source->sendPhase = 0;
        }
        if (target) return target;
    }

    if (!forward.empty()) return pickFrom(forward);
    if (!same.empty())    return pickFrom(same);

    return nullptr;
}

void GlobalState::handleInput()
{
    graphics::MouseState ms;
    graphics::getMouseState(ms);

    if (!ms.button_left_pressed) return;

    float mx = graphics::windowToCanvasX((float)ms.cur_pos_x);
    float my = graphics::windowToCanvasY((float)ms.cur_pos_y);

    Node* clicked = pickNode(mx, my);

    if (!selectedNode) {
        if (!clicked) return;

        Node* base = getBase(clicked->owner);
        if (clicked == base || hasChainToBase(clicked, clicked->owner)) {
            selectedNode = clicked;
            statusText = "Node selected. Click another node to connect.";
        }
    } else {
        if (clicked && canCreateEdge(selectedNode, clicked, selectedNode->owner)) {
            createSharedConnection(selectedNode, clicked);
            statusText = "Connection created (shared road).";
        }
        selectedNode = nullptr;
    }
}

void GlobalState::update(float dt_ms)
{
    if (!gameStarted) {
        if (graphics::getKeyState(graphics::SCANCODE_RETURN)) {
            gameStarted = true;
        }

        return;
    }

    if (gameOver) {
        if (graphics::getKeyState(graphics::SCANCODE_ESCAPE)) {
            graphics::destroyWindow();
            std::exit(0);
        }

        return;
    }

    float dt = dt_ms * 0.001f;
    handleInput();

    // production
    for (Node* n : nodes)
        if (hasChainToBase(n, n->owner))
            n->update(dt);

    // sending
    for (Node* n : nodes) {
        if (!hasChainToBase(n, n->owner) || n->unitCount <= 0) {
            sendTimers[n] = 0.0f;
            continue;
        }

        float& t = sendTimers[n];
        t += dt;

        if (t >= SEND_INTERVAL) {
            t -= SEND_INTERVAL;
            Node* target = chooseTarget(n);
            if (target) {
                units.push_back(new Unit(n, target, n->owner));
                n->unitCount--;
            }
        }
    }

    // units arrival
    for (auto it = units.begin(); it != units.end();) {
        Unit* u = *it;
        u->update(dt);

        if (u->arrived()) {
            Node* dest = u->to;

            if (dest->owner == u->owner) {
                if (dest->unitCount < dest->capacity)
                    dest->unitCount++;
            } else {
                dest->unitCount--;
                if (dest->unitCount <= 0) {
                    dest->owner = u->owner;
                    dest->unitCount = 1;
                    dest->roundRobinIndex = 0;

                    if (dest == playerBase || dest == enemyBase) {
                        gameOver = true;
                        winner = u->owner;
                    }
                }
            }

            delete u;
            it = units.erase(it);
        } else {
            ++it;
        }
    }
}

void GlobalState::draw()
{
    for (Node* n : nodes) n->draw();
    for (Unit* u : units) u->draw();

    if (selectedNode) {
        graphics::Brush ring;
        ring.fill_opacity = 0.0f;
        ring.outline_opacity = 1.0f;
        ring.outline_color[0] = 1.0f;
        ring.outline_color[1] = 1.0f;
        ring.outline_color[2] = 0.0f;
        graphics::drawDisk(selectedNode->x, selectedNode->y, 26.0f, ring);
    }

    graphics::Brush text;
    text.fill_color[0] = text.fill_color[1] = text.fill_color[2] = 1.0f;
    graphics::drawText(20, 30, 18, statusText, text);

    if (!gameStarted) {
        float panelX = 350.0f;
        float panelY = 220.0f;
        float panelW = 500.0f;
        float panelH = 180.0f;

        graphics::Brush panel;
        panel.fill_color[0] = 1.0f;
        panel.fill_color[1] = 1.0f;
        panel.fill_color[2] = 1.0f;
        panel.fill_opacity = 1.0f;
        panel.outline_opacity = 1.0f;
        panel.outline_color[0] = 0.0f;
        panel.outline_color[1] = 0.0f;
        panel.outline_color[2] = 0.0f;

        graphics::drawRect(
            panelX + panelW * 0.5f,
            panelY + panelH * 0.5f,
            panelW,
            panelH,
            panel
        );

        graphics::Brush text;
        text.fill_color[0] = 0.0f;
        text.fill_color[1] = 0.0f;
        text.fill_color[2] = 0.0f;

        graphics::drawText(panelX + 90, panelY + 80, 36, "Press ENTER to start", text);

        return;
    }

    if (gameOver) {
        float panelX = 350.0f;
        float panelY = 220.0f;
        float panelW = 500.0f;
        float panelH = 180.0f;

        graphics::Brush panel;
        panel.fill_color[0] = 1.0f;
        panel.fill_color[1] = 1.0f;
        panel.fill_color[2] = 1.0f;
        panel.fill_opacity = 1.0f;
        panel.outline_opacity = 1.0f;
        panel.outline_color[0] = 0.0f;
        panel.outline_color[1] = 0.0f;
        panel.outline_color[2] = 0.0f;

        graphics::drawRect(
            panelX + panelW * 0.5f,
            panelY + panelH * 0.5f,
            panelW,
            panelH,
            panel
        );

        graphics::Brush text;
        if (winner == Owner::Player) {
            text.fill_color[0] = 0.2f;
            text.fill_color[1] = 0.5f;
            text.fill_color[2] = 1.0f;
            graphics::drawText(panelX + 120, panelY + 80, 48, "BLUE WON", text);
        } else {
            text.fill_color[0] = 1.0f;
            text.fill_color[1] = 0.3f;
            text.fill_color[2] = 0.3f;
            graphics::drawText(panelX + 140, panelY + 80, 48, "RED WON", text);
        }

        text.fill_color[0] = 0.0f;
        text.fill_color[1] = 0.0f;
        text.fill_color[2] = 0.0f;
        graphics::drawText(panelX + 140, panelY + 130, 24, "Press ESC to quit", text);
    }
}
