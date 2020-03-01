#define APPEASE_GRADER
#define HOSTNAME_SIZE 255
#define PROG_NAME "smallsh"

// strerror_l turned out not to be necessary to localize
// error messages, as regular strerror seems to translate itself
// if locale is set up. More investigation is needed.
//#define USE_STRERROR_L

#ifdef USE_STRERROR_L
  #define STRERROR(a, b) strerror_l(a, b)
#else
  #define STRERROR(a, b) strerror(a)
#endif

// We are using gettext, hooray!
#define USE_GETTEXT
#define GETTEXT_DOMAIN "smallsh-translations"
#define GETTEXT_DIR "translations-dir"

#ifndef USE_GETTEXT
  #define _(String) (String)
  #define N_(String) String
  #define textdomain(Domain)
  #define bindtextdomain(Package, Directory)
#else
  #include <libintl.h>
  #define _(String) gettext (String)
  #define gettext_noop(String) String
  #define N_(String) gettext_noop (String)
#endif
