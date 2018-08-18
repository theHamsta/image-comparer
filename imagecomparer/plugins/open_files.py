import pydicom
import argparse
import pyconrad.autoinit
import os
import openjpeg
from PIL import Image
import numpy as np
import io
import jpeg_ls

# import pydicom
# from pydicom.data import get_testdata_files
import pydicom.pixel_data_handlers.pillow_handler as pillow_handler
# import numpy.testing as npt

# test_files = get_testdata_files('JPEG2000.dcm')

# for tf in test_files:

pydicom.config.image_handlers = [pillow_handler, ]
#     test_image_pil = pydicom.read_file(tf).pixel_array


def open_files(path):

    dicom = pydicom.read_file(path)
    # print(type(dicom.PixelData))
    # with open('/tmp/output.jp2', 'wb') as f: f.write(np.frombuffer(
    #         dicom.PixelData, dtype=np.uint8, offset=0x10))

    try:
        img = dicom.pixel_array
    except:
        img = jpeg_ls.decode(np.frombuffer(
            dicom.PixelData, dtype=np.uint8, offset=0x10))

    # img = Image.frombuffer(np.frombuffer(
    #     dicom.PixelData, dtype=np.uint8, offset=0x10), 'jpeg2000')

    return img.astype(np.float32)


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('file_path', help="")

    args = parser.parse_args()
    args.file_path = os.path.expanduser(args.file_path)

    array = open_dicom(args.file_path)
    pyconrad.imshow(array)


if __name__ == "__main__":
    main()
