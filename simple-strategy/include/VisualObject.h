#pragma once

class VisualObject {
public:
    virtual void update(float dt) = 0;
    virtual void draw() = 0;
    virtual ~VisualObject() {}
};
