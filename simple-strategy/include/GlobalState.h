#pragma once
#include <vector>
#include <unordered_map>
#include "Node.h"
#include "Unit.h"

class GlobalState {
public:
    std::vector<Node*> nodes;
    std::vector<Unit*> units;

    Node* selectedNode = nullptr;

    std::unordered_map<Node*, float> sendTimers;

    void init();
    void update(float dt);
    void draw();

    void handleInput();
    Node* pickNode(float x, float y);
    Node* chooseTarget(Node* source);
};
