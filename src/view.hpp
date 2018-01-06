#pragma once

//
// 表示関連
//

#include <cinder/TriMesh.h>
#include <cinder/gl/Vbo.h>
#include "PLY.hpp"


namespace ngs {


enum {
  PANEL_SIZE = 20,
};


struct View {
  // パネル
  std::vector<ci::gl::VboMeshRef> panel_models;

  ci::gl::VboMeshRef blank_model;
  ci::gl::VboMeshRef selected_model;
  ci::gl::VboMeshRef cursor_model;

  ci::gl::VboMeshRef bg_model; 
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
    "pf08.ply",
    "pf09.ply",
    "pf10.ply",
    "pf11.ply",
  };

  View view;
  for (const auto& file : model_files) {
    auto mesh = ci::gl::VboMesh::create(PLY::load(file));
    view.panel_models.push_back(mesh);
  }

  view.blank_model    = ci::gl::VboMesh::create(PLY::load("blank.ply"));
  view.selected_model = ci::gl::VboMesh::create(PLY::load("selected.ply"));
  view.cursor_model   = ci::gl::VboMesh::create(PLY::load("cursor.ply"));
  view.bg_model       = ci::gl::VboMesh::create(PLY::load("bg.ply"));

  return view;
}


#ifdef DEBUG

// パネルのエッジを表示
void drawPanelEdge(const Panel& panel, glm::vec3 pos, u_int rotation) {
  const float r_tbl[] = {
    0.0f,
    -180.0f * 0.5f,
    -180.0f,
    -180.0f * 1.5f 
  };

  ci::gl::pushModelView();
  ci::gl::translate(pos.x, pos.y, pos.z);
  ci::gl::rotate(ci::Vec3f(0.0f, r_tbl[rotation], 0.0f));

  ci::gl::lineWidth(10);

  const auto& edge = panel.getEdge();
  for (auto e : edge) {
    ci::Color col;
    if (e & Panel::PATH)   col = ci::Color(1.0, 1.0, 0.0);
    if (e & Panel::FOREST) col = ci::Color(0.0, 0.5, 0.0);
    if (e & Panel::GRASS)  col = ci::Color(0.0, 1.0, 0.0);
    ci::gl::color(col);

    ci::gl::drawLine(ci::Vec3f(-10.1, 1, 10.1), ci::Vec3f(10.1, 1, 10.1));
    ci::gl::rotate(ci::Vec3f(0.0f, 90.0f, 0.0f));
  }

  ci::gl::popModelView();

  ci::gl::color(ci::Color(1, 1, 1));
}

#endif

// パネルを１枚表示
void drawPanel(int number, glm::vec3 pos, u_int rotation, const View& view, float rotate_offset) {
  const float r_tbl[] = {
    0.0f,
    -180.0f * 0.5f,
    -180.0f,
    -180.0f * 1.5f 
  };

  ci::gl::pushModelView();
  ci::gl::translate(pos.x, pos.y, pos.z);
  ci::gl::rotate(ci::Vec3f(0.0f, r_tbl[rotation] + rotate_offset, 0.0f));
  ci::gl::draw(view.panel_models[number]);
  ci::gl::popModelView();
}

void drawPanel(int number, glm::ivec2 pos, u_int rotation, const View& view) {
  drawPanel(number, glm::vec3(pos.x, 0.0f, pos.y), rotation, view, 0.0f);
}

// Fieldのパネルをすべて表示
void drawFieldPanels(const std::vector<PanelStatus>& panels, const View& view) {
  for (const auto& p : panels) {
    drawPanel(p.number, p.position * int(PANEL_SIZE), p.rotation, view);
  }
}

// Fieldの置ける場所をすべて表示
void drawFieldBlank(const std::vector<glm::ivec2>& blank, const View& view) {
  for (const auto& pos : blank) {
    glm::ivec2 p = pos * int(PANEL_SIZE);

    ci::gl::pushModelView();
    ci::gl::translate(p.x, 0.0f, p.y);
    ci::gl::draw(view.blank_model);
    ci::gl::popModelView();
  }
}

// 置けそうな箇所をハイライト
void drawFieldSelected(glm::ivec2 pos, glm::vec3 scale, const View& view) {
  glm::ivec2 p = pos * int(PANEL_SIZE);

  ci::gl::pushModelView();
  ci::gl::translate(p.x, 0.0f, p.y);
  ci::gl::scale(scale.x, scale.y, scale.z);
  ci::gl::draw(view.selected_model);
  ci::gl::popModelView();
}

// 背景
void drawFieldBg(const View& view) {
  // TIPS OpenGLの設定を直接操作している
  // FOG機能を有効にする
  ci::gl::enable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);

  // GL_LINEARで使用する近景位置
	glFogf(GL_FOG_START, 50.0f);
  // GL_LINEARで使用する遠景位置
	glFogf(GL_FOG_END, 900.0f);

  // 色指定
  GLfloat fog_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  glFogfv(GL_FOG_COLOR, fog_color);

  ci::gl::pushModelView();
  ci::gl::translate(10, -15.0, 10);
  ci::gl::scale(20.0, 10.0, 20.0);
  ci::gl::draw(view.bg_model);
  ci::gl::popModelView();
  
  ci::gl::disable(GL_FOG);
}

}

