#if !defined (L_BIG_ENDIAN) && !defined (L_LITTLE_ENDIAN)
# if defined (__APPLE_CC__)
#  ifdef __BIG_ENDIAN__
#   define L_BIG_ENDIAN
#  else
#   define L_LITTLE_ENDIAN
#  endif
# else
#  define $(ENDIANNESS)
# endif
#endif
