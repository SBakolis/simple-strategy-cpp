#pragma once
#include <vector>
#include <unordered_map>
#include "Node.h"
#include "Unit.h"

class GlobalState {
public:
    ~GlobalState();

    std::vector<Node*> nodes;
    std::vector<Unit*> units;

    Node* selectedNode = nullptr;

    Node* playerBase = nullptr;
    Node* enemyBase  = nullptr;

    bool gameOver = false;
    Owner winner = Owner::Player;

    std::unordered_map<Node*, float> sendTimers;

    void init();
    void update(float dt_ms);
    void draw();

    void handleInput();
    Node* pickNode(float x, float y) const;

    Node* chooseTarget(Node* source);

    Node* getBase(Owner owner) const;
    bool gameStarted = false;
    bool hasChainToBase(Node* n, Owner owner) const;
    bool canCreateEdge(Node* from, Node* to, Owner owner) const;
    bool edgeExistsUndirected(Node* a, Node* b) const;
    void createSharedConnection(Node* a, Node* b);
};
