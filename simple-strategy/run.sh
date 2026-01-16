#!/bin/bash
set -e

echo "Building graph_game..."

g++ -std=c++17 \
    src/main.cpp src/GlobalState.cpp src/Node.cpp src/Unit.cpp \
    -Iinclude -Isgg -Isgg/sgg \
    -Lsgg/lib -lsgg \
    -lSDL2 -lSDL2_mixer -lGLEW -lfreetype \
    -lGL -lGLU \
    -o graph_game

echo "Build successful."
echo "Running graph_game..."
echo

./graph_game
