#pragma once

//
// 表示関連
//

enum {
  PANEL_SIZE = 64,
  GRID_NUM = 13,
};




// パネル１枚表示
void drawPanel(int number, glm::ivec2 pos, u_int rotation, const Texture& image) {
  int tx = (number % 8) * 128;
  int ty = (number / 8) * 128;

  const float r_tbl[] = {
    0.0f,
    M_PI * 0.5f,
    M_PI,
    M_PI * 1.5f 
  };

  drawTextureBox(pos.x, pos.y,
                 PANEL_SIZE, PANEL_SIZE,
                 tx, ty,
                 128, 128,
                 image, Color(1, 1, 1),
                 r_tbl[rotation], Vec2f(1, 1),
                 Vec2f(PANEL_SIZE / 2, PANEL_SIZE / 2));
}

// Fieldのパネルをすべて表示
void drawFieldPanels(const std::vector<PanelStatus>& panels, const Texture& image) {
  for (const auto& p : panels) {
    drawPanel(p.number, p.position * int(PANEL_SIZE), p.rotation, image);
  }
}

// Fieldの置ける場所をすべて表示
void drawFieldBlank(const std::vector<glm::ivec2>& blank) {
  for (const auto& p : blank) {
    drawFillBox(p.x * PANEL_SIZE - PANEL_SIZE / 2, p.y * PANEL_SIZE - PANEL_SIZE / 2,
                PANEL_SIZE, PANEL_SIZE,
                Color(0.3, 0.3, 0.3));
  }
}



// Grid表示
void drawFieldGrid() {
  // 繰り返し回数から決まる
  int start = -PANEL_SIZE * (GRID_NUM / 2) - PANEL_SIZE / 2;

  for (int x = 0; x <= GRID_NUM; ++x) {
    drawLine(start + x * PANEL_SIZE, start,
             start + x * PANEL_SIZE, start + PANEL_SIZE * GRID_NUM,
             1,
             Color(0.5, 0.5, 0.5));
  }

  for (int y = 0; y <= GRID_NUM; ++y) {
    drawLine(start,                         start + y * PANEL_SIZE,
             start + PANEL_SIZE * GRID_NUM, start + y * PANEL_SIZE,
             1,
             Color(0.5, 0.5, 0.5));
  }
}

