#
#
# threaded: Done in:  0.020416875
#

import glob
import os
import argparse
import concurrent.futures
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
  parser.add_argument('-d', '--path', action='store', default='./mlx/0000/', help='select where get *.png')
  args = parser.parse_args()

  in_file_path = args.path

  if len(in_file_path) > 0:
    scanned_files = search_files(in_file_path)
    
    # remove path and extension to left only name aka index
    frames = [file[ len(os.path.dirname(file))+1 : -8] for file in scanned_files]
    
    # sort frames in numeric order... kinda...
    frames.sort(key=int)

    restored_frame_names = [in_file_path + '/' + frame + '.thc.png' for frame in frames]

    # now load all images
    images = []

    with concurrent.futures.ThreadPoolExecutor() as executor:
      for result in executor.map(Image.open, restored_frame_names):
        images.append(result)

    gif_duration = 140 #len(scanned_files) FIXME: god damn! make some magic! you know... MAGIC!!

    # sorry for this...
    file_name = os.path.dirname(in_file_path) + '/' + '00000.gif'

    images[0].save(fp=file_name, format='GIF', append_images=images[1:],
           save_all=True, duration=gif_duration, loop=0)

  else:
    print(f'Dir {in_file_path} not found.')


if __name__ == '__main__':
    main()