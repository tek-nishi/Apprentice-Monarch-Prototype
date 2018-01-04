#pragma once

//
// OSごとのパスの違いを吸収
//

#include "Defines.hpp"
#include <cinder/Utilities.h>


namespace ngs {

ci::fs::path getAssetPath(const std::string& path);
ci::fs::path getDocumentPath();

}
