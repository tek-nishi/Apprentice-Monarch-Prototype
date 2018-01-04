﻿#pragma once

//
// ゲーム本編
//

#include <random>
#include "logic.hpp"


namespace ngs {

struct Game {
  enum {
    START_PANEL = 34
  };


  Game()
    : panels(createPanels())
  {
    // パネルを通し番号で用意
    for (int i = 0; i < 64; ++i) {
      waiting_panels.push_back(i);
    }

    {
      // 最初に置くパネルを取り除いてからシャッフル
      auto it = std::find(std::begin(waiting_panels), std::end(waiting_panels), START_PANEL);
      if (it != std::end(waiting_panels)) waiting_panels.erase(it);

      std::mt19937 engine;
      std::shuffle(std::begin(waiting_panels), std::end(waiting_panels), engine);
    }
    
    // 最初のパネルを設置
    field.addPanel(START_PANEL, {0, 0}, 0);
    getNextPanel();

    fieldUpdate();
  }


  // パネルが置けるか調べる
  bool canPutToBlank(glm::ivec2 field_pos) {
    bool can_put = false;
    
    if (std::find(std::begin(blank), std::end(blank), field_pos) != std::end(blank)) {
      can_put = canPutPanel(panels[hand_panel], field_pos, hand_rotation,
                            field, panels);
    }

    return can_put;
  }


  // 操作
  void rotationHandPanel() {
    hand_rotation = (hand_rotation + 1) % 4;
  }


  // 手持ちパネル情報
  u_int getHandPanel() const {
    return hand_panel;
  }

  u_int getHandRotation() const {
    return hand_rotation;
  }

  // フィールド情報
  const std::vector<PanelStatus>& getFieldPanels() const {
    return field_panels;
  }

  const std::vector<glm::ivec2>& getBlankPositions() const {
    return blank;
  };


private:
  std::vector<Panel> panels;

  std::vector<int> waiting_panels;
  int hand_panel;
  u_int hand_rotation;
  bool check_all_blank;

  bool can_put;

  Field field;

  // 完成した森
  std::vector<std::vector<glm::ivec2>> completed_forests;
  // 深い森
  std::vector<u_int> deep_forest;
  // 完成した道
  std::vector<std::vector<glm::ivec2>> completed_path;
  // 完成した教会
  std::vector<glm::ivec2> completed_church;

  // 列挙したフィールド上のパネル
  std::vector<PanelStatus> field_panels;
  // 列挙した置ける箇所
  std::vector<glm::ivec2> blank;


  bool getNextPanel() {
    if (waiting_panels.empty()) return false;

    hand_panel      = waiting_panels[0];
    hand_rotation   = 0;
    check_all_blank = true;

    waiting_panels.erase(std::begin(waiting_panels));

    return true;
  }

  // 各種情報を収集
  void fieldUpdate() {
    field_panels = field.enumeratePanels();
    blank        = field.searchBlank();
  }
};

}

