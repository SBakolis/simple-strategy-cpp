#pragma once
#include "VisualObject.h"
#include "Node.h"

class Unit : public VisualObject {
public:
    Node* from;
    Node* to;
    Owner owner;

    float t = 0.0f;

    Unit(Node* from, Node* to, Owner owner);

    void update(float dt) override;
    void draw() override;

    bool arrived() const;
};
