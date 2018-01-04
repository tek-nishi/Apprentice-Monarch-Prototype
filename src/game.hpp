#pragma once

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
  }


  void update() {
  }


  // フィールド上のパネルを列挙
  std::vector<PanelStatus> enumeratePanels() {
    return field.enumeratePanels();
  }

  // パネルを置ける箇所を列挙
  std::vector<glm::ivec2> enumerateBlank() {
    return field.searchBlank();
  }


private:
  std::vector<Panel> panels;

  std::vector<int> waiting_panels;
  u_int hand_rotation  = 0;
  bool check_all_blank = true;

  Field field;

  // 完成した森
  std::vector<std::vector<glm::ivec2>> completed_forests;
  // 深い森
  std::vector<u_int> deep_forest;
  // 完成した道
  std::vector<std::vector<glm::ivec2>> completed_path;
  // 完成した教会
  std::vector<glm::ivec2> completed_church;

};

}

