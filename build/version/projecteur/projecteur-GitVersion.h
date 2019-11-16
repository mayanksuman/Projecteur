#ifndef projecteur_GITVERSION_H
#define projecteur_GITVERSION_H

namespace projecteur {
  const char* version_string();
  unsigned int version_major();
  unsigned int version_minor();
  unsigned int version_patch();
  const char* version_flag();
  unsigned int version_distance();
  const char* version_shorthash();
  const char* version_fullhash();
  bool version_isdirty();
  const char* version_branch();
}

#endif
