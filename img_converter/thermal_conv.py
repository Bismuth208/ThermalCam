#
#
# minimal python3
# 

import glob
import os
import argparse
from struct import *
import concurrent.futures
from PIL import Image

# ------------------------------------------------------------------------ #
parser = argparse.ArgumentParser()


# 250 colors in rgb565
ironbow_palette = [
  0x0000,
  0x0001,
  0x0002,
  0x0003,
  0x0005,
  0x0004,
  0x0005,
  0x0006,
  0x0007,
  0x0006,
  0x0007,
  0x0008,
  0x0008,
  0x0009,
  0x000a,
  0x000b,
  0x000c,
  0x000a,
  0x000c,
  0x000b,
  0x000b,
  0x000c,
  0x000c,
  0x000d,
  0x080d,
  0x000f,
  0x000e,
  0x080d,
  0x000f,
  0x080e,
  0x100e,
  0x100e,
  0x100e,
  0x100f,
  0x180f,
  0x1810,
  0x1810,
  0x2011,
  0x2810,
  0x2810,
  0x2811,
  0x3011,
  0x3011,
  0x3012,
  0x3811,
  0x3012,
  0x3811,
  0x3812,
  0x4012,
  0x4013,
  0x4012,
  0x4812,
  0x4812,
  0x4812,
  0x4813,
  0x5012,
  0x5013,
  0x5013,
  0x5012,
  0x5813,
  0x5813,
  0x6013,
  0x6813,
  0x6013,
  0x6813,
  0x7013,
  0x7013,
  0x6813,
  0x7014,
  0x7013,
  0x7813,
  0x7813,
  0x7813,
  0x8013,
  0x8013,
  0x8814,
  0x7834,
  0x8813,
  0x8813,
  0x8813,
  0x9813,
  0x9813,
  0x9813,
  0x9813,
  0x9813,
  0x9033,
  0xa013,
  0xa013,
  0xa013,
  0xa813,
  0xa813,
  0xa813,
  0xb012,
  0xb013,
  0xb013,
  0xb012,
  0xa033,
  0xb012,
  0xb012,
  0xb832,
  0xb832,
  0xb852,
  0xb852,
  0xb872,
  0xc072,
  0xc071,
  0xc071,
  0xc092,
  0xc091,
  0xc091,
  0xc890,
  0xc8b0,
  0xc8b0,
  0xc8af,
  0xd0af,
  0xd0ae,
  0xd0cf,
  0xd0ce,
  0xd12d,
  0xd12c,
  0xd92c,
  0xd92b,
  0xd94b,
  0xd98a,
  0xd988,
  0xd989,
  0xd98a,
  0xe187,
  0xe188,
  0xe187,
  0xe1c6,
  0xd9c7,
  0xe1c5,
  0xe1c5,
  0xe1e4,
  0xe1e3,
  0xe203,
  0xe222,
  0xe222,
  0xea21,
  0xea22,
  0xea41,
  0xea41,
  0xea61,
  0xea80,
  0xea80,
  0xea80,
  0xeac0,
  0xeac1,
  0xeac1,
  0xf2c0,
  0xeb00,
  0xf300,
  0xf300,
  0xf320,
  0xf340,
  0xf340,
  0xf340,
  0xf380,
  0xf380,
  0xf380,
  0xf3a0,
  0xf3c0,
  0xf3c0,
  0xf3c0,
  0xf3e0,
  0xf3e0,
  0xfc00,
  0xf400,
  0xfc00,
  0xf420,
  0xfc20,
  0xfc40,
  0xf440,
  0xfc60,
  0xfc60,
  0xfc60,
  0xfc80,
  0xfc80,
  0xfca0,
  0xfca0,
  0xfcc0,
  0xfcc0,
  0xfce0,
  0xfd00,
  0xfd00,
  0xfd00,
  0xfd00,
  0xfd20,
  0xfd20,
  0xfd20,
  0xfd40,
  0xfd60,
  0xfd60,
  0xfd60,
  0xfd80,
  0xfda0,
  0xfda0,
  0xfdc0,
  0xfdc0,
  0xfdc0,
  0xfde0,
  0xfde0,
  0xfde0,
  0xfe00,
  0xfe00,
  0xfe20,
  0xfe20,
  0xfe20,
  0xfe40,
  0xfe40,
  0xfe40,
  0xfe60,
  0xfe60,
  0xfe80,
  0xfea0,
  0xfea0,
  0xfec0,
  0xfec0,
  0xfee0,
  0xfee0,
  0xff04,
  0xff04,
  0xff04,
  0xff24,
  0xff46,
  0xff48,
  0xff49,
  0xff69,
  0xff69,
  0xff6a,
  0xff6b,
  0xff6c,
  0xff6d,
  0xff8e,
  0xff8f,
  0xff90,
  0xff90,
  0xffb1,
  0xffb2,
  0xffb3,
  0xffb4,
  0xffb3,
  0xffb5,
  0xffb6,
  0xffd7,
  0xffd8,
  0xffd9,
  0xffd9,
  0xffda,
]

VERSION_DATA = 1

THC_FRAME_DATA_TYPE_CAL = 1
THC_FRAME_DATA_TYPE_STILL = 2
THC_FRAME_DATA_TYPE_MOV = 3

# Header format
# |---- name --------|-- size ------|-- offset - |
# | marker           |4 bytes       | +0         |
# | version          |4 bytes       | +4         |
# | size             |4 bytes       | +8         |
# | type             |4 bytes       | +12        |
# | reserved[12]     |4 bytes * 12  | +14        |
# | cal_data_size    |4 bytes       | +64        |
# | frame_data_size  |4 bytes       | +68        |
# |----------------------------------------------|
#
# File format
# |---- name --------|-- size ------|-- offset - |
# | header           | 72 bytes     | +0         |
# | data             | *(1)         | +72        |
# |----------------------------------------------|
#
# *(1) Data type and it's size depends on data type
#      described in header in field 'type'.
#


# ------------------------------------------------------------------------ #
def get_file_data(file_name):
    with open(file_name, 'rb') as file:
        return file.read()


def save_file_data(file_name, file_data_bytes):
    with open(file_name, 'wb') as file:
        file.write(file_data_bytes)

# ------------------------------------------------------------------------ #
def conv_565_to_rgb(color):
  rgb_val = []
  
  rgb_val += [(((color & 0xF800) >> 11) << 3) & 0xFF]
  rgb_val += [(((color & 0x7E0) >> 5) << 2) & 0xFF]
  rgb_val += [(((color & 0x1F)) << 3) & 0xFF]
  
  return rgb_val


def create_rgb_pal():
  palette = []
  for color_in_565 in ironbow_palette:
    palette += conv_565_to_rgb(color_in_565)

  return palette


def save_pal(pal_data):
  pal_pic_raw = []
  for i in range(len(palette_ext)):
    pal_pic_raw += [i]
  
  pal_pic = Image.new(mode='P', size=(16, 5))
  pal_pic.putdata(bytes(pal_pic_raw))
  pal_pic.putpalette(pal_data, rawmode='RGB')
  pal_pic.save('./pal.png', 'png')


def save_pic(pic_name, rgb_data, pal_data):
  img_scale_factor = 4

  img = Image.new(mode='P', size=(64, 48))
  
  img.putdata(bytes(rgb_data))
  img.putpalette(pal_data, rawmode='RGB')

  img = img.transpose(Image.FLIP_LEFT_RIGHT)
  img = img.resize((img.width * img_scale_factor, img.height * img_scale_factor))

  img.save(pic_name, 'png')
  
  #img.show()

# ------------------------------------------------------------------------ #
def search_files(dest_path):
  result = []

  for x in os.walk(dest_path):
    for y in glob.glob(os.path.join(x[0], '*.thc')):
      result.append(y)

  return result


def transcode_image(dest_path, pal_data):
  raw_pic = get_file_data(dest_path)

  version_data = int(''.join(map(str, Struct('<I').unpack(raw_pic[4 : 8]))))

  if VERSION_DATA == version_data:
    type_data = int(''.join(map(str, Struct('<I').unpack(raw_pic[12 : 16]))))

    if type_data != THC_FRAME_DATA_TYPE_CAL:
      if type_data == THC_FRAME_DATA_TYPE_STILL:

        print(f'File {dest_path}.png')

        save_pic(dest_path + '.png', raw_pic[72 : ], pal_data)

# ------------------------------------------------------------------------ #
def main():
  parser.add_argument('-d', '--path', action='store', default='./mlx/0000/', help='select where get thermoframes')
  args = parser.parse_args()

  in_file_path = args.path

  if len(in_file_path) > 0:
    palette_ext_rgb = create_rgb_pal()

    with concurrent.futures.ThreadPoolExecutor() as executor:
      process = [executor.submit(transcode_image, file, palette_ext_rgb) for file in search_files(in_file_path)]

  else:
    print(f'Dir {in_file_path} not found.')


if __name__ == '__main__':
  main()