#define APPEASE_GRADER
#define HOSTNAME_SIZE 255
#define PROG_NAME "smallsh"

//#define USE_STRERROR_L

#ifdef USE_STRERROR_L
  #define STRERROR(a, b) strerror_l(a, b)
#else
  #define STRERROR(a, b) strerror(a)
#endif

#ifdef USE_GETTEXT
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
