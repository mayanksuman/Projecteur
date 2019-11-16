#include "projecteur-GitVersion.h"

namespace projecteur {
  const char* version_string() { return "0.6-alpha.28"; }
  unsigned int version_major() { return 0; }
  unsigned int version_minor() { return 6; }
  unsigned int version_patch() { return 0; }
  const char* version_flag() { return "alpha"; }
  unsigned int version_distance() { return 28; }
  const char* version_shorthash() { return "3ecaa63"; }
  const char* version_fullhash() { return "3ecaa63e20ff34bef0fe901a38da69840f1e0f0c"; }
  bool version_isdirty() { return 1; }
  const char* version_branch() { return "feature/enhanced_perf_dialog"; }
}
