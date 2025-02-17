#include <stdbool.h>

enum LanguageCode {
  LANG_EN, // English
  LANG_FR, // French
  LANG_RU, // Russian
};

struct Language {
  enum LanguageCode code;
  bool has_verb_conjugations;
  bool has_grammatical_gender;
  bool has_noun_declensions;
};

struct Language make_language(enum LanguageCode code) {
  struct Language lang = {
    .code = code,
    .has_verb_conjugations = false,
    .has_grammatical_gender = false,
    .has_noun_declensions = false,
  };

  switch (code) {
    case LANG_RU:
      lang.has_noun_declensions = true;
    case LANG_FR:
      lang.has_verb_conjugations = true;
      lang.has_grammatical_gender = true;
      break;
    case LANG_EN:
      lang.has_verb_conjugations = true;
      break;
  }

  return lang;
}
