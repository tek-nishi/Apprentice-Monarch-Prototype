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


  Game(const std::vector<Panel>& panels_)
    : panels(panels_),
      scores(8, 0)
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
    field_panels = field.enumeratePanels();
  }

  // 内部時間を進める
  void update() {
    if (isPlaying() && !--play_time) {
      // 時間切れ
      DOUT << "Time Up." << std::endl;
      endPlay();
    }
  }


  // 本編開始
  void beginPlay() {
    started = true;
    // とりあえず３分
    play_time = 60 * 60 * 3;

    getNextPanel();
    fieldUpdate();

    DOUT << "Game started." << std::endl;
  }

  // 本編終了
  void endPlay() {
    finished = true;
    DOUT << "Game ended." << std::endl;
  }

  
  // 残り時間
  u_int getRemainingTime() const {
    return play_time;
  }


  // プレイ中？
  // 始まったらtrueになり、終了しても変化しない
  bool isBeganPlay() const {
    return started;
  }

  // 始まって、結果発表直前まで
  bool isPlaying() const {
    return started && !finished;
  }

  // 結果発表以降
  bool isEndedPlay() const {
    return finished;
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
  void putHandPanel(glm::ivec2 field_pos) {
    // プレイ中でなければ置けない
    if (!isPlaying()) return;

    field.addPanel(hand_panel, field_pos, hand_rotation);

    bool update_score = false;
    {
      // 森完成チェック
      auto completed = isCompleteAttribute(Panel::FOREST, field_pos, field, panels);
      if (!completed.empty()) {
        // 得点
        DOUT << "Forest: " << completed.size() << '\n';
        u_int deep_num = 0;
        for (const auto& comp : completed) {
          DOUT << " Point: " << comp.size() << '\n';

          // 深い森
          bool deep = isDeepForest(comp, field, panels);
          if (deep) {
            deep_num += 1;
          }
          deep_forest.push_back(deep ? 1 : 0);
        }
        DOUT << "  Deep: " << deep_num 
             << std::endl;

        // TIPS コンテナ同士の連結
        std::copy(std::begin(completed), std::end(completed), std::back_inserter(completed_forests));
        update_score = true;
      }
    }
    {
      // 道完成チェック
      auto completed = isCompleteAttribute(Panel::PATH, field_pos, field, panels);
      if (!completed.empty()) {
        // 得点
        DOUT << "  Path: " << completed.size() << '\n';
        for (const auto& comp : completed) {
          DOUT << " Point: " << comp.size() << '\n';
        }
        DOUT << std::endl;

        // TIPS コンテナ同士の連結
        std::copy(std::begin(completed), std::end(completed), std::back_inserter(completed_path));
        update_score = true;
      }
    }
    {
      // 教会完成チェック
      auto completed = isCompleteChurch(field_pos, field, panels);
      if (!completed.empty()) {
        // 得点
        DOUT << "Church: " << completed.size() << std::endl;
              
        // TIPS コンテナ同士の連結
        std::copy(std::begin(completed), std::end(completed), std::back_inserter(completed_church));
        update_score = true;
      }
    }

    if (update_score) {
      updateScores();
    }


    fieldUpdate();

    // 新しいパネル
    if (!getNextPanel()) {
      // 全パネルを使い切った
      DOUT << "End of panels." << std::endl;
      endPlay();
      return;
    }

    // パネルをどこかに置けるか調べる
    if (!canPanelPutField(panels[hand_panel], blank,
                          field, panels)) {
      // 置けない…
      DOUT << "Can't put panel." << std::endl;
      endPlay();
      return;
    }
  }

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


  // プレイ中のスコア
  const std::vector<int>& getScores() const {
    return scores;
  }


  // プレイ結果
  void calcResult() {
    DOUT << "Forest: " << completed_forests.size() << '\n'
         << "  area: " << countTotalAttribute(completed_forests, field, panels) << '\n'
         << "  deep: " << std::count(std::begin(deep_forest), std::end(deep_forest), 1) << '\n'
         << "  Path: " << completed_path.size() << '\n'
         << "length: " << countTotalAttribute(completed_path, field, panels) << '\n'
         << "  Town: " << countTown(completed_path, field, panels)
         << std::endl;
  }



private:
  // FIXME 参照で持つのいくない
  const std::vector<Panel>& panels;

  bool started    = false;
  bool finished   = false;
  u_int play_time = 0;

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
  // スコア
  std::vector<int> scores;

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

  // スコア更新
  void updateScores() {
    scores[0] = completed_path.size();
    scores[1] = countTotalAttribute(completed_path, field, panels);
    scores[2] = completed_forests.size();
    scores[3] = countTotalAttribute(completed_forests, field, panels);
    scores[4] = std::count(std::begin(deep_forest), std::end(deep_forest), 1);
    scores[5] = countTown(completed_path, field, panels);
    scores[6] = completed_church.size();
  }
};

}

