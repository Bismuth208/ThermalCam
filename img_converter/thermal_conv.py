#
# Convert *.thc files created by ThermalCam to *.png or *.gif files.
#
# Author:
#   Alexandr Antonov (@Bismuth208)
#
# Tested on: 
#   Python3 (Mac OS)
#
# Dependency:
#   Pillow Image Library (PIL)
# 
# Licence: MIT
#

import glob
import os
import argparse
import concurrent.futures
from PIL import Image

# ------------------------------------------------------------------------ #
parser = argparse.ArgumentParser()


# 250 colors in rgb565
ironbow_palette = [
  0x0000, 0x0001, 0x0002, 0x0003, 0x0005, 0x0004, 0x0005, 0x0006,
  0x0007, 0x0006, 0x0007, 0x0008, 0x0008, 0x0009, 0x000A, 0x000B,
  0x000C, 0x000A, 0x000C, 0x000B, 0x000B, 0x000C, 0x000C, 0x000D,
  0x080D, 0x000F, 0x000E, 0x080D, 0x000F, 0x080E, 0x100E, 0x100E,
  0x100E, 0x100F, 0x180F, 0x1810, 0x1810, 0x2011, 0x2810, 0x2810,
  0x2811, 0x3011, 0x3011, 0x3012, 0x3811, 0x3012, 0x3811, 0x3812,
  0x4012, 0x4013, 0x4012, 0x4812, 0x4812, 0x4812, 0x4813, 0x5012,
  0x5013, 0x5013, 0x5012, 0x5813, 0x5813, 0x6013, 0x6813, 0x6013,
  0x6813, 0x7013, 0x7013, 0x6813, 0x7014, 0x7013, 0x7813, 0x7813,
  0x7813, 0x8013, 0x8013, 0x8814, 0x7834, 0x8813, 0x8813, 0x8813,
  0x9813, 0x9813, 0x9813, 0x9813, 0x9813, 0x9033, 0xA013, 0xA013,
  0xA013, 0xA813, 0xA813, 0xA813, 0xB012, 0xB013, 0xB013, 0xB012,
  0xA033, 0xB012, 0xB012, 0xB832, 0xB832, 0xB852, 0xB852, 0xB872,
  0xC072, 0xC071, 0xC071, 0xC092, 0xC091, 0xC091, 0xC890, 0xC8B0,
  0xC8B0, 0xC8AF, 0xD0AF, 0xD0AE, 0xD0CF, 0xD0CE, 0xD12D, 0xD12C,
  0xD92C, 0xD92B, 0xD94B, 0xD98A, 0xD988, 0xD989, 0xD98A, 0xE187,
  0xE188, 0xE187, 0xE1C6, 0xD9C7, 0xE1C5, 0xE1C5, 0xE1E4, 0xE1E3,
  0xE203, 0xE222, 0xE222, 0xEA21, 0xEA22, 0xEA41, 0xEA41, 0xEA61,
  0xEA80, 0xEA80, 0xEA80, 0xEAC0, 0xEAC1, 0xEAC1, 0xF2C0, 0xEB00,
  0xF300, 0xF300, 0xF320, 0xF340, 0xF340, 0xF340, 0xF380, 0xF380,
  0xF380, 0xF3A0, 0xF3C0, 0xF3C0, 0xF3C0, 0xF3E0, 0xF3E0, 0xFC00,
  0xF400, 0xFC00, 0xF420, 0xFC20, 0xFC40, 0xF440, 0xFC60, 0xFC60,
  0xFC60, 0xFC80, 0xFC80, 0xFCA0, 0xFCA0, 0xFCC0, 0xFCC0, 0xFCE0,
  0xFD00, 0xFD00, 0xFD00, 0xFD00, 0xFD20, 0xFD20, 0xFD20, 0xFD40,
  0xFD60, 0xFD60, 0xFD60, 0xFD80, 0xFDA0, 0xFDA0, 0xFDC0, 0xFDC0,
  0xFDC0, 0xFDE0, 0xFDE0, 0xFDE0, 0xFE00, 0xFE00, 0xFE20, 0xFE20,
  0xFE20, 0xFE40, 0xFE40, 0xFE40, 0xFE60, 0xFE60, 0xFE80, 0xFEA0,
  0xFEA0, 0xFEC0, 0xFEC0, 0xFEE0, 0xFEE0, 0xFF04, 0xFF04, 0xFF04,
  0xFF24, 0xFF46, 0xFF48, 0xFF49, 0xFF69, 0xFF69, 0xFF6A, 0xFF6B,
  0xFF6C, 0xFF6D, 0xFF8E, 0xFF8F, 0xFF90, 0xFF90, 0xFFB1, 0xFFB2,
  0xFFB3, 0xFFB4, 0xFFB3, 0xFFB5, 0xFFB6, 0xFFD7, 0xFFD8, 0xFFD9,
  0xFFD9, 0xFFDA,
]

# ------------------------------------------------------------------------ #
def conv_565_to_rgb(color):
    rgb_val = []
    
    rgb_val += [(((color & 0xF800) >> 11) << 3) & 0xFF]
    rgb_val += [(((color & 0x7E0) >> 5) << 2) & 0xFF]
    rgb_val += [(((color & 0x1F)) << 3) & 0xFF]
    
    return rgb_val

# Convert rgb565 colors to RGB format for PIL
def create_rgb_pal():
    palette = []
    for color_in_565 in ironbow_palette:
        palette += conv_565_to_rgb(color_in_565)

    return palette


palette_ext_rgb = create_rgb_pal()

# ------------------------------------------------------------------------ #
THC_FRAME_VERSION_DATA = 1

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

THC_DATA_OFFSET = 72

header_fileds = { 'marker' : 0,
                  'version' : 4,
                  'size' : 8,
                  'type' : 12,
                  'cal_data_size' : 64,
                  'frame_data_size' : 68
                }


# ------------------------------------------------------------------------ #
def get_file_data(file_name, size = None):
    with open(file_name, 'rb') as file:
        return file.read(size)


def save_file_data(file_name, file_data_bytes):
    with open(file_name, 'wb') as file:
        file.write(file_data_bytes)


def search_files(dest_path, extension = '*?????.thc'):
    result = []

    for x in os.walk(dest_path):
        for y in glob.glob(os.path.join(x[0], extension)):
            result.append(y)

    return result

# ------------------------------------------------------------------------ #
def save_pal(pal_data):
    pal_pic_raw = []
    for i in range(len(palette_ext)):
        pal_pic_raw += [i]
    
    pal_pic = Image.new(mode='P', size=(16, 5))
    pal_pic.putdata(bytes(pal_pic_raw))
    pal_pic.putpalette(pal_data, rawmode='RGB')
    pal_pic.save('./pal.png', 'png')


def create_pic(rgb_data):
    img_scale_factor = 4

    img = Image.new(mode='P', size=(64, 48))
    
    img.putdata(bytes(rgb_data))
    img.putpalette(palette_ext_rgb, rawmode='RGB')

    img = img.transpose(Image.FLIP_LEFT_RIGHT)
    img = img.resize((img.width * img_scale_factor, img.height * img_scale_factor))

    return img

# ------------------------------------------------------------------------ #
def read_header(raw_data):
    header_data = {}

    for name, offset in header_fileds.items():
        header_data[name] = int.from_bytes(raw_data[offset : offset + 4], byteorder='little')

    return header_data

# ------------------------------------------------------------------------ #
def convert_image(dest_path):
    raw_pic = get_file_data(dest_path)[ THC_DATA_OFFSET : ]
    return create_pic(raw_pic)


def transcode_image(dest_path):
    convert_image(dest_path).save(dest_path + '.png', 'png')
    print(f'File {dest_path}.png')

# ------------------------------------------------------------------------ #
# Splits all gif frames to sublists for multiple gif files
def group_consecutives(vals, step=1):
    run = []
    result = [run]
    expect = None

    for v in vals:
        if (v[-9 : -4] == expect) or (expect is None):
            run.append(v)
        else:
            run = [v]
            result.append(run)

        expect = '%.5d' % (int(v[-9 : -4]) + int(step))

    return result


def sort_thc_files(files):
    stills = []
    movs = []

    for file in files:
        header = read_header(get_file_data(file, THC_DATA_OFFSET))

        if header['version'] == THC_FRAME_VERSION_DATA:
            if (header['type'] != THC_FRAME_DATA_TYPE_CAL):
                if (header['type'] == THC_FRAME_DATA_TYPE_STILL):
                    stills.append(file)
                if (header['type'] == THC_FRAME_DATA_TYPE_MOV):
                    movs.append(file)

    movs = group_consecutives(movs)

    return stills, movs

# ------------------------------------------------------------------------ #
def create_stills(still_list):
    if len(still_list) > 0:
        with concurrent.futures.ThreadPoolExecutor() as executor:
            process = [executor.submit(transcode_image, file) for file in still_list]


def create_movs(mov_lists, dest_path):
    if len(mov_lists) > 0:
        img_gif_num = 0

        for mov_list in mov_lists:
            if len(mov_list) < 2:
                create_stills(mov_list)
                pass

            # now load all frames
            images = []

            with concurrent.futures.ThreadPoolExecutor() as executor:
              images = [result for result in executor.map(convert_image, mov_list)]

            # len(scanned_files) FIXME: god damn! make some magic! you know...
            # MAGIC!!
            gif_duration = 140

            gif_str_num = '/%.5d.thc.gif' % img_gif_num;
            file_name = os.path.dirname(dest_path) + gif_str_num

            images[0].save(fp=file_name, format='GIF', append_images=images[1:],
                   save_all=True, duration=gif_duration, loop=0)

            print(f'File {file_name}')
            img_gif_num += 1

# ------------------------------------------------------------------------ #
def main():
    parser.add_argument('-d', '--path', 
                        action='store',
                        default='./mlx/0000/',
                        help='select where get thermograms')
    args = parser.parse_args()

    in_file_path = args.path

    if len(in_file_path) > 0:
        scanned_files = search_files(in_file_path)

        # remove path and extension to left only name aka index
        frames = [file[ len(os.path.dirname(file))+1 : -4] for file in scanned_files]

        # sort frames in numeric order... kinda...
        frames.sort(key=int)

        restored_frame_names = [in_file_path + frame + '.thc' for frame in frames]

        still_list, mov_lists = sort_thc_files(restored_frame_names)

        create_stills(still_list)
        create_movs(mov_lists, in_file_path)
    else:
        print(f'Dir {in_file_path} not found.')


if __name__ == '__main__':
    main()
