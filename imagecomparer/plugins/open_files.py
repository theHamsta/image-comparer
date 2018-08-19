import pydicom
import argparse
import os
import numpy as np
import io

# import pydicom
# from pydicom.data import get_testdata_files
import pydicom.pixel_data_handlers.pillow_handler as pillow_handler


def open_files(path):

    dicom = pydicom.read_file(path)
    # print(type(dicom.PixelData))
    # with open('/tmp/output.jp2', 'wb') as f: f.write(np.frombuffer(
    #         dicom.PixelData, dtype=np.uint8, offset=0x10))

    try:
        img = dicom.pixel_array
    except:
        import jpeg_ls
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


if __name__ == "__main__":
    main()
