#!/bin/bash
set -e

echo "Building strategy_nodes..."

g++ -std=c++17 \
    src/main.cpp src/GlobalState.cpp src/Node.cpp src/Unit.cpp \
    -Iinclude -Isgg -Isgg/sgg \
    -Lsgg/lib -lsgg \
    -lSDL2 -lSDL2_mixer -lGLEW -lfreetype \
    -lGL -lGLU \
    -o strategy_nodes

echo "Build successful."
echo "Running strategy_nodes..."
echo

./strategy_nodes
