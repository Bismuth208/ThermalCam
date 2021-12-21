#ifndef _SD_WRITER_H
#define _SD_WRITER_H

// ----------------------------------------------------------------------
#include <FreeRTOS_helper.hpp>


// ----------------------------------------------------------------------

// Version of *.thc files
#define THC_FRAME_DATA_VER     2

// How much frames should be taken before STILL images will be switche to MOV mode
#define THC_MAX_FRAMES_FOR_MOV 4

// Describes type of data presented in *.thc file
enum {
  THC_FRAME_DATA_TYPE_CAL = 1,  // Calibration data
  THC_FRAME_DATA_TYPE_STILL,    // Single image frame
  THC_FRAME_DATA_TYPE_MOV       // Video (gif)
};

// Description of header data
typedef struct {
  union {
    uint32_t ul_marker;
    uint8_t uc_marker[4];
  };
  uint32_t ul_version;
  uint32_t ul_size;
  uint32_t ul_type;
  uint32_t ul_reserved[12];
  //uint32_t ul_num_frames;

  uint32_t ul_cal_data_size;
  uint32_t ul_frame_data_size;
} thc_frame_header_t;

typedef struct {
  thc_frame_header_t x_header;

  uint8_t *puc_cal_data;
  uint8_t *puc_frame_data;
} thc_frame_t;

// ----------------------------------------------------------------------
extern int16_t sSdBufTest[32 * 24];
extern BaseType_t xIsSDCardFail;

// ----------------------------------------------------------------------
//extern Timer TakeScreenShotTimer;
extern Counter <1> ScreenShootRdyCounter;

// ----------------------------------------------------------------------
void vSDInit(void);

void v_init_thc_struct(void);
void v_thc_save_cal(void);
void v_thc_check_frame_type(void);

void vTakeScreenShoot(void *pvArg);
void vTakeSreenShotFlag(BaseType_t xBtnState);
//void vTakeVideoFlag(BaseType_t xBtnState);

#endif // _SD_WRITER_H
