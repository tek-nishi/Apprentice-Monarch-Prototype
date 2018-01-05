//
// 見習い君主
//

#include "Defines.hpp"
#include <cinder/app/AppNative.h>
#include <cinder/gl/gl.h>
#include <cinder/gl/Texture.h>
#include <cinder/Camera.h>
#include <cinder/Arcball.h>
#include <cinder/Rand.h>
#include <glm/glm.hpp>
#include <set>
#include "game.hpp"
#include "view.hpp"
#include "font.hpp"
#include "counter.hpp"
#include "sound.hpp"


using namespace ci;
using namespace ci::app;


class TestProjectApp : public AppNative {
  float fov = 25.0f;
  float distance = 160.0f;

  CameraPersp bg_camera;
  CameraPersp field_camera;
  CameraOrtho ui_camera;

  Arcball arcball;

  bool mouse_draged = false;
  
  // TIPS:キー入力の判定に集合を利用
  std::set<int> pressing_key;

  // パネル
  // FIXME インスタンスに時間がかかるのでGameから外に出した
  std::vector<ngs::Panel> panels;

  // ゲーム本編
  std::shared_ptr<ngs::Game> game;

  enum {
    TITLE,
    GAMESTART,        // 開始演出
    GAMEMAIN,
    GAMEEND,          // 終了演出
    RESULT,
  };
  int playing_mode = TITLE;

  // 汎用カウンタ
  ngs::Counter counter;


  // 表示関連
  ngs::View view;
  std::shared_ptr<ngs::Font> font;
  std::shared_ptr<ngs::Font> jpn_font;

  Vec3f cursor_pos; 
  glm::ivec2 field_pos;
  bool can_put = false;

  float rotate_offset = 0.0f;


  // 残り時間表示位置(xのみ)
  int remain_time_x;

  u_int frame_counter = 0;

  // 表示用スコア
  std::vector<int> game_score;
  std::vector<int> game_score_effect;


  // 背景
  // gl::Texture bg_image;


  // サウンド
  //   FIXME デフォルトコンストラクタで初期化できないのでスマポ
  std::shared_ptr<ngs::Sound> sound;



#ifdef DEBUG
  bool disp_debug_info = false;
#endif


public:
  void prepareSettings(Settings* settings) override {
    settings->setWindowSize(1024, 720);
    settings->setTitle(PREPRO_TO_STR(PRODUCT_NAME));
    // settings->enableHighDensityDisplay(true);
  }

	void setup() override {
#if defined(CINDER_MAC)
    // FIXME OSXでタイトルバーにアプリ名を表示するworkaround
    getWindow()->setTitle(PREPRO_TO_STR(PRODUCT_NAME));
#endif

    // 乱数初期化
    Rand::randomize();	

#if 0
    // 遠景用カメラ
    bg_camera = CameraPersp(getWindowWidth(), getWindowHeight(),
                            fov,
                            1.0f, 1000.0f);

    bg_camera.setEyePoint(Vec3f(0.0f, 0.0f, 0.0f));
    bg_camera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 1.0f));
#endif

    field_camera = CameraPersp(getWindowWidth(), getWindowHeight(),
                               fov,
                               1.0f, 1000.0f);

    field_camera.setEyePoint(Vec3f(0.0f, 0.0f, -distance));
    field_camera.setCenterOfInterestPoint(Vec3f(0.0f, 10.0f, 0.0f));

    // UIカメラ
    auto half_size = getWindowSize() / 2;
    ui_camera = CameraOrtho(-half_size.x, half_size.x,
                            -half_size.y, half_size.y,
                            -1, 1);
    ui_camera.setEyePoint(Vec3f(0, 0, 0));
    ui_camera.setCenterOfInterestPoint(Vec3f(0, 0, -1));


    // Arcball初期化
    arcball = Arcball(getWindowSize());
    Quatf q(toRadians(-30.0f), toRadians(45.0f), 0.0f);
    arcball.setQuat(q);

    // パネル生成
    panels = ngs::createPanels();
    // 本編生成
    game = std::make_shared<ngs::Game>(panels);

    // 表示
    view = ngs::createView();
    font = std::make_shared<ngs::Font>("MAIAN.TTF");
    font->size(80);

    {
      // 残り時間表示のx位置はあらかじめ決めておく
      font->size(80);
      auto size = font->drawSize("9'99");
      remain_time_x = -size.x / 2;
    }

    jpn_font = std::make_shared<ngs::Font>("DFHSM5001.ttf");

    // 背景
    // bg_image = loadImage(loadAsset("bg.png"));

    // サウンド
    sound = std::make_shared<ngs::Sound>();

    sound->play("title");
  }


  void calcFieldPos(Vec2i pos) {
    float x = pos.x / float(getWindowWidth());
    float y = 1.0f - pos.y / float(getWindowHeight());

    // 画面奥に伸びるRayを生成
    Ray ray = field_camera.generateRay(x, y,
                                       field_camera.getAspectRatio());

    // 逆行列で掛けてRayをAABB側の座標系に変換
    Quatf rotate = arcball.getQuat();
    Matrix44f m  = rotate.toMatrix44();
    m.invert();

    Vec3f dir = m.transformVec(ray.getDirection());
    ray.setDirection(dir);
    Vec3f origin = m * ray.getOrigin();
    ray.setOrigin(origin);

    // 地面との交差を調べ、正確な位置を計算
    float z;
    float on_field = ray.calcPlaneIntersection(Vec3f(0, 10, 0), Vec3f(0, 1, 0), &z);
    can_put = false;
    if (on_field) {
      cursor_pos = ray.calcPosition(z);

      field_pos.x = ngs::roundValue(cursor_pos.x, ngs::PANEL_SIZE);
      field_pos.y = ngs::roundValue(cursor_pos.z, ngs::PANEL_SIZE);

      can_put = game->canPutToBlank(field_pos);
    }
  }

  // 操作関連
  void mouseMove(MouseEvent event) override {
    // フィールド上での座標を計算
    Vec2i pos = event.getPos();
    calcFieldPos(pos);
  }

  void mouseDown(MouseEvent event) override {
    if (!event.isLeft()) return;

    mouse_draged = false;

    Vec2i pos = event.getPos();
    arcball.mouseDown(pos);
  }
  
  void mouseDrag(MouseEvent event) override {
    if (!event.isLeftDown()) return;

    mouse_draged = true;

    Vec2i pos = event.getPos();
    arcball.mouseDrag(pos);
  }
  
  void mouseUp(MouseEvent event) override {
    switch (playing_mode) {
    case TITLE:
      if (event.isLeft() && !mouse_draged) {
        // ゲーム開始
        game->beginPlay();
        playing_mode = GAMESTART;
        
        game_score = game->getScores();
        game_score_effect.resize(game_score.size());
        std::fill(std::begin(game_score_effect), std::end(game_score_effect), 0);

        counter.add("gamestart", 90);

        sound->stop("title");
        sound->play("agree");
      }
      break;

    case GAMESTART:
    case GAMEMAIN:
      if (game->isPlaying()) {
        if (event.isLeft() && !mouse_draged) {
          // パネルを配置
          if (can_put) {
            game->putHandPanel(field_pos);
            rotate_offset = 0.0f;
            can_put = false;
            
            sound->play("panel-set");
          }
        }
        else if (event.isRight()) {
          // パネルを回転
          game->rotationHandPanel();
          rotate_offset = 90.0f;
          can_put = game->canPutToBlank(field_pos);

          sound->play("panel-rotate");
        }
      }
      break;

    case GAMEEND:
      {
      }
      break;

    case RESULT:
      if (event.isLeft() && !mouse_draged) {
        // 再ゲーム
        game = std::make_shared<ngs::Game>(panels);
        playing_mode = TITLE;
        
        sound->play("agree");
      }
      break;
    }
  }

  void mouseWheel(MouseEvent event) override {
    distance -= event.getWheelIncrement() * 5.0f;

    field_camera.setEyePoint(Vec3f(0.0f, 0.0f, -distance));
    field_camera.setCenterOfInterestPoint(Vec3f(0.0f, 10.0f, 0.0f));
  }


  void keyDown(KeyEvent event) override {
    int code = event.getCode();
    pressing_key.insert(code);

#ifdef DEBUG
    if (code == KeyEvent::KEY_r) {
      // 強制リセット
      game = std::make_shared<ngs::Game>(panels);
      playing_mode = TITLE;
    }
    if (code == KeyEvent::KEY_t) {
      // 時間停止
      game->pauseTimeCount();
    }
    if (code == KeyEvent::KEY_e) {
      // 強制終了
      game->endPlay();
    }

    if (code == KeyEvent::KEY_d) {
      disp_debug_info = !disp_debug_info;
    }

    // 手持ちパネル強制変更
    if (code == KeyEvent::KEY_b) {
      game->changePanelForced(-1);
    }
    else if (code == KeyEvent::KEY_n) {
      game->changePanelForced(1);
    }
#endif
  }

  void keyUp(KeyEvent event) override {
    int code = event.getCode();
    pressing_key.erase(code);
  }


	void update() override {
    counter.update();
    game->update();
    
    switch (playing_mode) {
    case TITLE:
      {
      }
      break;

    case GAMESTART:
      {
        if (!counter.check("gamestart")) {
          playing_mode = GAMEMAIN;
          DOUT << "GAMEMAIN." << std::endl;
        }
      }
      break;

    case GAMEMAIN:
      if (!game->isPlaying()) {
        // 結果画面へ
        playing_mode = GAMEEND;
        counter.add("gameend", 120);

        sound->play("gameover");
      }
      break;

    case GAMEEND:
      {
        if (!counter.check("gameend")) {
          game_score = game->getScores();
          std::fill(std::begin(game_score_effect), std::end(game_score_effect), 0);
          playing_mode = RESULT;
          DOUT << "RESULT." << std::endl;
        }
      }
      break;

    case RESULT:
      {
      }
      break;
    }

    frame_counter += 1;
  }

  
	void draw() override {
    gl::clear(Color(0, 0, 0));

    // 本編
    gl::disableAlphaBlending();

    Quatf rotate = arcball.getQuat();
    Matrix44f m = rotate.toMatrix44();

#if 0
    // 遠景
    gl::setMatrices(bg_camera);
    gl::multModelView(m);

    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::enable(GL_CULL_FACE);
    bg_image.enableAndBind();

    // 球の半径をマイナスにしているので、見た目に上下左右が反転している
    // それを解消するために回転している
    gl::rotate(Vec3f(0, 0, 180));

    // 球を内側から見る→画像が左右反転する
    // これを解決する方法の１つとして半径をマイナスの値にする
    gl::drawSphere(Vec3f(0.0f, 0.0f, 0.0f), -50.0, 32);
    bg_image.disable();
#endif

    // プレイ画面
    gl::setMatrices(field_camera);
    gl::multModelView(m);

    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enable(GL_CULL_FACE);

    // フィールド
    const auto& field_panels = game->getFieldPanels();
    ngs::drawFieldPanels(field_panels, view);

    if (game->isPlaying()) {
      // 置ける場所
      const auto& blank = game->getBlankPositions();
      ngs::drawFieldBlank(blank, view);

      if (can_put) { 
        drawFieldSelected(field_pos, view);
      }
      
      // 手持ちパネル
      rotate_offset *= 0.8f;
      glm::vec3 pos(cursor_pos.x, cursor_pos.y, cursor_pos.z);
      ngs::drawPanel(game->getHandPanel(), pos, game->getHandRotation(), view, rotate_offset);
#ifdef DEBUG
      if (disp_debug_info) {
        // 手元のパネル
        ngs::drawPanelEdge(panels[game->getHandPanel()], pos, game->getHandRotation());

        // 置こうとしている場所の周囲
        auto around = game->enumerateAroundPanels(field_pos);
        if (!around.empty()) {
          for (auto it : around) {
            glm::ivec2 pos = it.first;
            pos *= int(ngs::PANEL_SIZE);

            glm::vec3 p(pos.x, 0.0f, pos.y);

            auto status = it.second;
            ngs::drawPanelEdge(panels[status.number], p, status.rotation);
          }
        }
      }
#endif

    }
    drawFieldBg(view);

    // UI
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::disable(GL_CULL_FACE);
    gl::enableAlphaBlending();

    gl::setMatrices(ui_camera);

    switch (playing_mode) {
    case TITLE:
      {
        {
          font->size(100);
          std::string text("Apprentice Monarch");
          auto size = font->drawSize(text);
          font->draw(text, Vec2f(-size.x / 2, 150), ColorA(1, 1, 1, 1));
        }
        {
          font->size(60);
          std::string text("Left click to start");
          auto size = font->drawSize(text);

          float r = frame_counter * 0.05f;
          font->draw(text, Vec2f(-size.x / 2, -200), ColorA(1, 1, 1, (std::sin(r) + 1.0f) * 0.5f));
        }
      }
      break;

    case GAMESTART:
      {
        font->size(90);
        const char* text = "Secure the territory!";

        auto size = font->drawSize(text);
        font->draw(text, Vec2f(-size.x / 2.0f, -size.y / 2.0f), ColorA(1, 1, 1, 1));
      }
      break;

    case GAMEMAIN:
      {
        // 残り時間
        font->size(80);

        char text[100];
        u_int remainig_time = (game->getRemainingTime() + 59) / 60;
        u_int minutes = remainig_time / 60;
        u_int seconds = remainig_time % 60;
        sprintf(text, "%d'%02d", minutes, seconds);

        // 時間が10秒切ったら色を変える
        ColorA color(1, 1, 1, 1);
        if (remainig_time <= 10) {
          color = ColorA(1, 0, 0, 1);
        }

        font->draw(text, Vec2f(remain_time_x, 280), color);
      }
      drawGameInfo(25, Vec2f(-500, 0), -40);
      break;

    case GAMEEND:
      {
        font->size(90);
        const char* text = "Well done!";

        auto size = font->drawSize(text);
        font->draw(text, Vec2f(-size.x / 2.0f, -size.y / 2.0f), ColorA(1, 1, 1, 1));
      }
      break;

    case RESULT:
      {
        {
          font->size(80);
          std::string text("Result");
          auto size = font->drawSize(text);
          font->draw(text, Vec2f(-size.x / 2, 250), ColorA(1, 1, 1, 1));
        }
        {
          font->size(40);
          std::string text("Left click to title");
          auto size = font->drawSize(text);

          float r = frame_counter * 0.05f;
          font->draw(text, Vec2f(-size.x / 2, -300), ColorA(1, 1, 1, (std::sin(r) + 1.0f) * 0.5f));
        }
        drawResult();
      }
      break;
    }
  }


private:
  // プレイ情報を表示
  void drawGameInfo(int font_size, Vec2f pos, float next_y) {
    jpn_font->size(font_size);

    const auto& scores = game->getScores();
    // 変動した箇所の色を変える演出用
    for (size_t i = 0; i < scores.size(); ++i) {
      if (game_score[i] != scores[i]) {
        game_score_effect[i] = 60;
      }
      game_score[i] = scores[i];
    }

    for (auto& i : game_score_effect) {
      i = std::max(i - 1, 0);
    }

    const char* text[] = {
      u8"道の数:   %d",
      u8"道の長さ: %d",
      u8"森の数:   %d",
      u8"森の広さ: %d",
      u8"深い森:   %d",
      u8"街の数:   %d",
      u8"教会の数: %d",
      // u8"城の数:   %d",
    };

    u_int i = 0;
    for (const auto* t : text) {
      ColorA col = game_score_effect[i] ? ColorA(1, 0, 0, 1)
                                        : ColorA(1, 1, 1, 1);

      char buffer[100];
      sprintf(buffer, t, game_score[i]);
      jpn_font->draw(buffer, pos,col);

      pos.y += next_y;
      i += 1;
    }
  }

  // 結果を表示
  void drawResult() {
    drawGameInfo(30, Vec2f(-300, 150), -50);

    int score   = game->getTotalScore();
    int ranking = game->getTotalRanking();

    const char* ranking_text[] = {
      "Emperor",
      "King",
      "Viceroy",
      "Grand Duke",
      "Prince",
      "Landgrave",
      "Duke",
      "Marquess",
      "Margrave",
      "Count",  
      "Viscount",
      "Baron", 
      "Baronet",
    };

    {
      char text[100];
      sprintf(text, "Your Score: %d", score);
      font->size(60);
      font->draw(text, Vec2f(0, 0), ColorA(1, 1, 1, 1));
    }
    {
      char text[100];
      sprintf(text, "Your Rank: %s", ranking_text[ranking]);
      font->size(60);
      font->draw(text, Vec2f(0, -100), ColorA(1, 1, 1, 1));
    }
  }

};

CINDER_APP_NATIVE(TestProjectApp, RendererGl)
