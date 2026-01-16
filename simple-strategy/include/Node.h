#pragma once
#include <vector>
#include "VisualObject.h"

enum class Owner {
    Player,
    Enemy
};

class Node : public VisualObject {
public:
    float x, y;
    int layer;
    Owner owner;

    int unitCount;
    int capacity;

    std::vector<Node*> edges;
    int roundRobinIndex = 0;

    float productionTimer = 0.0f;

    Node(float x, float y, int layer, Owner owner);

    void update(float dt) override;
    void draw() override;

    bool contains(float mx, float my) const;
    bool isConnected() const { return !edges.empty(); }
    int sendPhase = 0; // 0: forward, 1: same level
};
