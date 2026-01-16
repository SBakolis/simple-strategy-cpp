#include <graphics.h>
#include "GlobalState.h"

GlobalState game;

static const int W = 1200;
static const int H = 700;

void update(float dt) {
    game.update(dt);
}

void draw() {
    graphics::Brush br;
    br.fill_color[0] = br.fill_color[1] = br.fill_color[2] = 0.1f;
    graphics::drawRect(W * 0.5f, H * 0.5f, (float)W, (float)H, br);

    game.draw();
}

int main() {
    graphics::createWindow(W, H, "Strategy Nodes");
    graphics::setFont("assets/DejaVuSans.ttf");

    game.init();

    graphics::setDrawFunction(draw);
    graphics::setUpdateFunction(update);
    graphics::startMessageLoop();

    return 0;
}
