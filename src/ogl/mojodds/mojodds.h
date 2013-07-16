/*
MODIFIED for maki engine!!
*/

#ifndef _INCL_MOJODDS_H_
#define _INCL_MOJODDS_H_

/* !!! FIXME: flesh this file out. */


int MOJODDS_isDDS(const void *_ptr, const unsigned long _len);
int MOJODDS_getTexture(const void *_ptr, const unsigned long _len,
                       const void **_tex, unsigned long *_texlen,
                       unsigned int *_glfmt, unsigned int *_w,
                       unsigned int *_h, unsigned int *_miplevels);

#endif

/* end of mojodds.h ... */
