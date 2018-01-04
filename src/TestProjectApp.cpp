//
// 見習い君主
//

#include <cinder/app/AppNative.h>
#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/Arcball.h>
#include <glm/glm.hpp>
#include <set>
#include "logic.hpp"


using namespace ci;
using namespace ci::app;


class TestProjectApp : public AppNative {
  CameraPersp field_camera;

  Arcball arcball;

  bool mouse_draged = false;
  
  // TIPS:キー入力の判定に集合を利用
  std::set<int> pressing_key;


public:
  void prepareSettings(Settings* settings) override {
    settings->setWindowSize(800, 600);
  }

	void setup() override {
    field_camera = CameraPersp(getWindowWidth(), getWindowHeight(),
                               35.0f,
                               0.1f, 100.0f);

    // TIPS EyePoint→CenterOfInterestPointの順序で初期化する
    field_camera.setEyePoint(Vec3f(0.0f, 0.0f, -3.0f));
    field_camera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 0.0f));

    // Arcball初期化
    arcball = Arcball(getWindowSize());
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enable(GL_CULL_FACE);
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
    // パネルを配置
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

    gl::drawColorCube(Vec3f(0, 0, 0), Vec3f(1, 1, 1));
  }

};

CINDER_APP_NATIVE(TestProjectApp, RendererGl)
