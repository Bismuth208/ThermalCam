#ifndef _DUCK_LOGO_PIC_H
#define _DUCK_LOGO_PIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define DUCK_LOGO_W 60 // px
#define DUCK_LOGO_H 40 // px

#define DUCK_LOGO_POS_X 50 // in pix
#define DUCK_LOGO_POS_Y 8 // in pix

#define DUCK_LOGO_SIZE 7200 // (w * h) * 3

extern const uint8_t Duck_logo_Iron_Gradient_data[DUCK_LOGO_SIZE];

#ifdef __cplusplus
}
#endif

#endif /* _DUCK_LOGO_PIC_H */
