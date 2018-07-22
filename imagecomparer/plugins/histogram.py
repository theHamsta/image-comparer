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
enable_processing_of_two_images= True


def process(imgPath1, imgPath2=None):
    try:
        img1 = None
        img2 = None

        plt.figure('Image Comparer - Histogram')
        plt.title('Histogram')
        legends = []
        if imgPath1 and os.path.isfile(imgPath1):
            img1 = scipy.misc.imread(imgPath1, flatten=True)
            plt.hist(img1.ravel(), bins=256*10,
                     range=(np.min(img1),np.max(img1)), alpha=0.75, histtype='stepfilled')
            legends.append(os.path.basename(imgPath1))

        if imgPath2 and os.path.isfile(imgPath2):
            img2 = scipy.misc.imread(imgPath2, flatten=True)
            plt.hist(img2.ravel(), bins=256*10,
                     range=(np.min(img2),np.max(img2)), alpha=0.75, histtype='stepfilled')
            legends.append(os.path.basename(imgPath2))

        plt.legend(legends)
        if img1 is not None or img2 is not None:
            plt.show()

    except Exception as e:
        print(e)

    return "Huhu"


if __name__ == "__main__":
    process('/home/stephan/Bilder/carpe_diem2.jpg')
            # '/home/stephan/Bilder/dagobert.jpg')
