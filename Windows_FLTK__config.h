
// Really, REALLY hacky (and evil) way to disable ClearType/Font Smoothing on Windows...
#undef  DEFAULT_QUALITY
#define DEFAULT_QUALITY NONANTIALIASED_QUALITY

#define FLTK_DATADIR    "C:/C_Projects/Libraries/FLTK_v1.3.0rc2"
#define FLTK_DOCDIR     "C:/C_Projects/Libraries/FLTK_v1.3.0rc2/documentation"
#define BORDER_WIDTH 2
#undef  HAVE_GL
#undef  HAVE_GL_GLU_H
#undef  USE_COLORMAP
#undef  HAVE_XDBE
#undef  USE_XDBE
#undef  HAVE_XDBE
#undef  HAVE_OVERLAY
#undef  HAVE_GL_OVERLAY
#define WORDS_BIGENDIAN 0
#define U16 unsigned short
#define U32 unsigned
#undef  U64
#undef  HAVE_VSNPRINTF
#undef  HAVE_SNPRINTF
#define HAVE_STRCASECMP	1
#define HAVE_LOCALE_H 1
#define HAVE_LOCALECONV 1
#undef  HAVE_POLL
#undef  HAVE_LIBPNG
#undef  HAVE_LIBZ
#undef  HAVE_LIBJPEG
#undef  FLTK_HAVE_CAIRO
#undef  HAVE_PNG_H
#undef  HAVE_LIBPNG_PNG_H
#undef  HAVE_PNG_GET_VALID
#undef  HAVE_PNG_SET_TRNS_TO_ALPHA
