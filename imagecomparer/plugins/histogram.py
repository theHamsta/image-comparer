import matplotlib.pyplot as plt
import numpy as np
import os
import scipy.misc

name = "Histogram"
description = "Calculates histograms of the opened images"
shortcut = "Ctrl+H"
shortcut_left = ""
shortcut_right = ""
enable_processing_of_one_image = True
enable_processing_of_two_images = True


def process(img1, imgPath1, img2=None, imgPath2=None):
    try:

        plt.figure('Image Comparer - Histogram')
        plt.title('Histogram')
        legends = []
        plt.hist(img1.ravel(), bins=256, range=(0.0, 255.),
                 alpha=0.75, histtype='stepfilled')

        if imgPath1 and os.path.isfile(imgPath1):
            plt.hist(img1.ravel(), bins=256,
                     range=(0.0, 255.), alpha=0.75, histtype='stepfilled')
        legends.append(os.path.basename(imgPath1))

        if imgPath2 and os.path.isfile(imgPath2):
            plt.hist(img2.ravel(), bins=256,
                     range=(0.0, 255.), alpha=0.75, histtype='stepfilled')
        legends.append(os.path.basename(imgPath2))

        # plt.legend(legends)
        # plt.show()

    except Exception as e:
        print(e)


if __name__ == "__main__":
    process('/home/stephan/Bilder/carpe_diem2.jpg')
    # '/home/stephan/Bilder/dagobert.jpg')
