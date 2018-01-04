#pragma once

//
// 表示関連
//

#include <cinder/TriMesh.h>
#include "PLY.hpp"


namespace ngs {


enum {
  PANEL_SIZE = 64,
  GRID_NUM = 13,
};


struct View {
  // パネル
  std::vector<ci::TriMesh> panel_models;



};


// 画面表示の用意
View createView() {
  const char* model_files[] = {
    "pa00.ply",
    "pa01.ply",
    "pa02.ply",
    "pa03.ply",
    "pa04.ply",
    "pa05.ply",
    "pa06.ply",
    "pa07.ply",
    "pa08.ply",
    "pa09.ply",
    "pa10.ply",
    "pa11.ply",

    "pa00.ply",
    "pa01.ply",
    "pa02.ply",
    "pa03.ply",
    "pa04.ply",
    "pa05.ply",
    "pa06.ply",
    "pa07.ply",
    "pa08.ply",
    "pa09.ply",
    "pa10.ply",
    "pa11.ply",
    
    "pd00.ply",
    "pd01.ply",
    "pd02.ply",
    "pd03.ply",
    "pd04.ply",
    "pd05.ply",
    "pd06.ply",
    "pd07.ply",
    "pd08.ply",
    "pd09.ply",
    "pd10.ply",
    "pd11.ply",
    
    "pd00.ply",
    "pd01.ply",
    "pd02.ply",
    "pd03.ply",
    "pd04.ply",
    "pd05.ply",
    "pd06.ply",
    "pd07.ply",
    "pd08.ply",
    "pd09.ply",
    "pd10.ply",
    "pd11.ply",
    
    "pf00.ply",
    "pf01.ply",
    "pf02.ply",
    "pf03.ply",
    "pf04.ply",
    "pf05.ply",
    "pf06.ply",
    "pf07.ply",
    
    "pf00.ply",
    "pf01.ply",
    "pf02.ply",
    "pf03.ply",
    "pf04.ply",
    "pf05.ply",
    "pf06.ply",
    "pf07.ply",
  };

  View view;
  for (const auto& file : model_files) {
    view.panel_models.push_back(PLY::load(file));
  }

  return view;
}

// パネルを１枚表示
void drawPanel(int number, glm::ivec2 pos, u_int rotation, const View& view) {
  const float r_tbl[] = {
    0.0f,
    180.0f * 0.5f,
    180.0f,
    180.0f * 1.5f 
  };

  ci::gl::pushModelView();
  ci::gl::rotate(ci::Vec3f(0.0f, r_tbl[rotation], 0.0f));
  ci::gl::draw(view.panel_models[number]);
  ci::gl::popModelView();
}

// Fieldのパネルをすべて表示
void drawFieldPanels(const std::vector<PanelStatus>& panels, const View& view) {
  for (const auto& p : panels) {
    drawPanel(p.number, p.position * int(PANEL_SIZE), p.rotation, view);
  }
}


}

#if 0


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

#endif
