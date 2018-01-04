//
// 見習い君主
//

#include "Defines.hpp"
#include <cinder/app/AppNative.h>
#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/Arcball.h>
#include <glm/glm.hpp>
#include <set>
#include "game.hpp"
#include "view.hpp"
#include "font.hpp"


using namespace ci;
using namespace ci::app;


class TestProjectApp : public AppNative {
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
    GAMEMAIN,
    RESULT,
  };
  int playing_mode = TITLE;


  // 表示関連
  ngs::View view;
  std::shared_ptr<ngs::Font> font;

  Vec3f cursor_pos; 
  glm::ivec2 field_pos;
  bool can_put = false;


  int remain_time_x;




  u_int frame_counter = 0;




public:
  void prepareSettings(Settings* settings) override {
    settings->setWindowSize(1024, 720);
  }

	void setup() override {
    field_camera = CameraPersp(getWindowWidth(), getWindowHeight(),
                               35.0f,
                               1.0f, 1000.0f);

    // TIPS EyePoint→CenterOfInterestPointの順序で初期化する
    field_camera.setEyePoint(Vec3f(0.0f, 100.0f, -150.0f));
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
      auto size = font->drawSize("9'99");
      remain_time_x = -size.x / 2;
    }
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
        playing_mode = GAMEMAIN;
      }
      break;

    case GAMEMAIN:
      if (game->isPlaying()) {
        if (event.isLeft() && !mouse_draged) {
          // パネルを配置
          if (can_put) {
            game->putHandPanel(field_pos);
            can_put = false;
          }
        }
        else if (event.isRight()) {
          // パネルを回転
          game->rotationHandPanel();
          can_put = game->canPutToBlank(field_pos);
        }
      }
      break;

    case RESULT:
      if (event.isLeft() && !mouse_draged) {
        // 再ゲーム
        game = std::make_shared<ngs::Game>(panels);
        playing_mode = TITLE;
      }
      break;
    }
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

    }
    if (code == KeyEvent::KEY_e) {
      // 強制終了
      game->endPlay();
    }
#endif
  }

  void keyUp(KeyEvent event) override {
    int code = event.getCode();
    pressing_key.erase(code);
  }


	void update() override {
    game->update();
    
    switch (playing_mode) {
    case TITLE:
      {
      }
      break;

    case GAMEMAIN:
      if (!game->isPlaying()) {
        // 結果画面へ
        playing_mode = RESULT;
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
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enable(GL_CULL_FACE);
    gl::disableAlphaBlending();

    gl::setMatrices(field_camera);
    
    Quatf rotate = arcball.getQuat();
    Matrix44f m = rotate.toMatrix44();
    gl::multModelView(m);

    // フィールド
    const auto& panels = game->getFieldPanels();
    ngs::drawFieldPanels(panels, view);

    if (game->isPlaying()) {
      // 置ける場所
      const auto& blank = game->getBlankPositions();
      ngs::drawFieldBlank(blank, view);

      if (can_put) { 
        drawFieldSelected(field_pos, view);
      }
      
      // 手持ちパネル
      glm::vec3 pos(cursor_pos.x, cursor_pos.y, cursor_pos.z);
      ngs::drawPanel(game->getHandPanel(), pos, game->getHandRotation(), view);
    }

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

    case GAMEMAIN:
      {
        // 残り時間
        font->size(80);

        char text[100];
        u_int remainig_time = (game->getRemainingTime() + 59) / 60;
        u_int minutes = remainig_time / 60;
        u_int seconds = remainig_time % 60;
        sprintf(text, "%d'%02d", minutes, seconds);
        
        font->draw(text, Vec2f(remain_time_x, 280), ColorA(1, 1, 1, 1));
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
      }
      break;
    }
  }

};

CINDER_APP_NATIVE(TestProjectApp, RendererGl)
