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


using namespace ci;
using namespace ci::app;


class TestProjectApp : public AppNative {
  CameraPersp field_camera;

  Arcball arcball;

  bool mouse_draged = false;
  
  // TIPS:キー入力の判定に集合を利用
  std::set<int> pressing_key;

  // ゲーム本編
  std::shared_ptr<ngs::Game> game;


  // 表示
  ngs::View view;

  bool on_field = false;
  Vec3f cursor_pos; 
  glm::ivec2 field_pos;

  bool can_put = false;



public:
  void prepareSettings(Settings* settings) override {
    settings->setWindowSize(1024, 720);
  }

	void setup() override {
    field_camera = CameraPersp(getWindowWidth(), getWindowHeight(),
                               35.0f,
                               1.0f, 1000.0f);

    // TIPS EyePoint→CenterOfInterestPointの順序で初期化する
    field_camera.setEyePoint(Vec3f(0.0f, 0.0f, -200.0f));
    field_camera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 0.0f));

    // Arcball初期化
    arcball = Arcball(getWindowSize());

    // 本編
    game = std::make_shared<ngs::Game>();

    // 表示
    view = ngs::createView();


    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enable(GL_CULL_FACE);
  }


  void mouseMove(MouseEvent event) override {
    // フィールド上での座標を計算
    Vec2i pos = event.getPos();
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
    on_field = ray.calcPlaneIntersection(Vec3f(0, 1, 0), Vec3f(0, 1, 0), &z);
    can_put  = false;
    if (on_field) {
      cursor_pos = ray.calcPosition(z);

      field_pos.x = ngs::roundValue(cursor_pos.x, ngs::PANEL_SIZE);
      field_pos.y = ngs::roundValue(cursor_pos.z, ngs::PANEL_SIZE);

      can_put = game->canPutToBlank(field_pos);
    }
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
    if (event.isLeft() && !mouse_draged) {
      // パネルを配置
    }
    else if (event.isRight()) {
      // パネルを回転
      game->rotationHandPanel();
    }
  }


  void keyDown(KeyEvent event) override {
    int code = event.getCode();
    pressing_key.insert(code);
  }

  void keyUp(KeyEvent event) override {
    int code = event.getCode();
    pressing_key.erase(code);
  }


	void update() override {
  }

  
	void draw() override {
    gl::clear(Color(0, 0, 0));

    gl::setMatrices(field_camera);
    
    Quatf rotate = arcball.getQuat();
    Matrix44f m = rotate.toMatrix44();
    gl::multModelView(m);

    // フィールド
    const auto& panels = game->getFieldPanels();
    ngs::drawFieldPanels(panels, view);

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

};

CINDER_APP_NATIVE(TestProjectApp, RendererGl)
