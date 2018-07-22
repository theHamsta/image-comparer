import matplotlib.pyplot as plt
import numpy as np
import os
import scipy.misc
import easygui

name = "Dummy Plugin"
description = "Calculates"
shortcut = "Ctrl+T"
shortcut_left = ""
shortcut_right = ""
enable_processing_of_one_image = True
enable_processing_of_two_images = True


def process(img1, img2=None):
    try:
        plt.figure()
        plt.show()
        # if img1:
        #     plt.figure(str(img1.shape))
        #     plt.show()
        # if img2:
        #     plt.figure(str(img2.shape))
        #     plt.show()

    except Exception as e:
        print(e)
        easygui.msgbox(e)

    return "Huhu"


if __name__ == "__main__":
    process('/home/stephan/Bilder/boyband2.jpg',
            '/home/stephan/Bilder/dagobert.jpg')
