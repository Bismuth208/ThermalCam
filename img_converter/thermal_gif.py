#
#
#
#

import glob
import os
import argparse
from PIL import Image

# ------------------------------------------------------------------------ #
parser = argparse.ArgumentParser()

# ------------------------------------------------------------------------ #
def search_files(dest_path):
  result = []

  for x in os.walk(dest_path):
    for y in glob.glob(os.path.join(x[0], '*.png')):
      result.append(y)

  return result


def main():
  parser.add_argument('-d', '--path', action='store', default='./mlx', help='select where get *.png')
  args = parser.parse_args()

  in_file_path = args.path

  images = []

  if len(in_file_path) > 0:
    scanned_files = search_files(in_file_path)
    #scanned_files.sort(key=int)

    frames = []
    
    # sort frames in numeric order... kinda...
    for file in scanned_files:
      # remove path and extension to left only name aka index
      frame_name = file[ len(os.path.dirname(file))+1 : -8]
      frames.append(frame_name)

    frames.sort(key=int)

    # now load all images
    for frame in frames:
      restored_frame_name = in_file_path + '/' + frame + '.thc.png'

      frame = Image.open(restored_frame_name)
      images.append(frame)

    gif_duration = 150 #len(scanned_files) FIXME: god damn! make some magic! you know... MAGIC!!

    # sorry for this...
    file_name = os.path.dirname(in_file_path) + '/' + '00000.gif'

    images[0].save(fp=file_name, format='GIF', append_images=images[1:],
           save_all=True, duration=gif_duration, loop=0)

  else:
    print('Dir not found:', in_file_path)


if __name__ == '__main__':
    main()