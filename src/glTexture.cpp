//
// OpenGLのテクスチャハンドリング
//

#include "glTexture.hpp"
#include <iostream>


namespace ngs {

GlTexture::GlTexture() {
  DOUT << "GlTexture()" << std::endl;
  glGenTextures(1, &id_);
}

GlTexture::~GlTexture() {
  DOUT << "~GlTexture()" << std::endl;
  glDeleteTextures(1, &id_);
}


// OpenGLのコンテキストに拘束する
void GlTexture::bind() const {
  glBindTexture(GL_TEXTURE_2D, id_);
}

// 拘束を解除
void GlTexture::unbind() const {
  glBindTexture(GL_TEXTURE_2D, 0);
}

}
