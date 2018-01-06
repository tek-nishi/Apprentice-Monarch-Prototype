#pragma once

//
// 超簡易サウンド管理
//

#include <cinder/audio/Context.h>
#include <cinder/audio/SamplePlayerNode.h>
#include <map>
#include <string>


namespace ngs {

// 生成用の情報
struct SoundInfo {
  std::string key;            // 識別子
  std::string path;           // パス
};


struct Sound {
  
  Sound() {
    // サウンド機能を有効にする
    ci::audio::Context* ctx = ci::audio::master();
    ctx->enable();

    // 読み込み
    const SoundInfo info[] = {
      { "title",        "title.m4a" },
      { "agree",        "agree.m4a" },
      { "panel-rotate", "panel-rotate.m4a" },
      { "panel-set",    "panel-set.m4a" },
      { "gameover",     "stageclear.m4a" },
      { "complete",     "complete.m4a" },
    };

    for (const auto& i : info) {
      auto node = ctx->makeNode(new ci::audio::BufferPlayerNode());
      
      ci::audio::SourceFileRef source = ci::audio::load(ci::app::loadAsset(i.path));
      node->loadBuffer(source);
      node >> ctx->getOutput();

      sounds.insert({ i.key, node });

      DOUT << "sound: " << i.key << " path: " << i.path << std::endl;
    }
  }

  void play(const std::string& key) {
    if (!sounds.count(key)) {
      DOUT << "No sound: " << key << std::endl;
      return;
    }

    sounds.at(key)->start();
  }

  void stop(const std::string& key) {
    if (!sounds.count(key)) {
      DOUT << "No sound: " << key << std::endl;
      return;
    }

    sounds.at(key)->stop();
  }

  void stopAll() {
    for (auto& it : sounds) {
      stop(it.first);
    }
  }


private:
  std::map<std::string, ci::audio::BufferPlayerNodeRef> sounds;

};

}
