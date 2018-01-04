#pragma once

//
// 文字表示
//
// FIXME:FONScontextがメモリリークしている
//

#include "Defines.hpp"
#include <fontstash.h>
#include <string>
#include <memory>
#include <cinder/Vector.h>
#include "glTexture.hpp"


namespace ngs {

class Font {
  enum {
    DEFAULT_SIZE = 20,
  };

  struct Context
  {
    std::shared_ptr<GlTexture> tex;
    int width, height;
  };

  Context gl_;
  FONScontext* context_;


  // 以下、fontstashからのコールバック関数
  static int create(void* userPtr, int width, int height);
  static int resize(void* userPtr, int width, int height);
  static void update(void* userPtr, int* rect, const unsigned char* data);
  static void draw(void* userPtr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts);


public:
  // コンストラクタ
  // path フォントファイルのパス(ttf,otf)
  Font(const std::string& path);

  // フォントサイズ指定
  void size(const int size);

  // 描画した時のサイズを取得
  ci::Vec2f drawSize(const std::string& text);

  // 描画
  // text  表示文字列
  // pos   表示位置
  // color 表示色
  void draw(const std::string& text, const ci::Vec2f& pos, const ci::ColorA& color);

};

}
