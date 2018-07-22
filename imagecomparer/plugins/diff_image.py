import matplotlib.pyplot as plt
import numpy as np
import os
import scipy.misc
# import easygui

name = "Difference Image"
description = "Calculates"
shortcut = "Ctrl+D"
shortcut_left = ""
shortcut_right = ""
enable_processing_of_one_image = False
enable_processing_of_two_images= True


def process(imgPath1, imgPath2=None):
	try:

		if imgPath1 and os.path.isfile(imgPath1):
			if imgPath2 and os.path.isfile(imgPath2):
				filename1 = os.path.basename(imgPath1)
				filename2 = os.path.basename(imgPath2)

				# plt.figure('Image Comparer - Difference Image')
				# plt.title(imgPath1 + ' - ' + imgPath1)
				img1 = scipy.misc.imread(imgPath1, flatten=True)
				img2 = scipy.misc.imread(imgPath2, flatten=True)
				if img1.shape != img2.shape:
					# easygui.msgbox("Image Dimensions do not match!", title="simple gui")
					img2 = scipy.misc.imresize(img2, img1.shape)

				diff_img = img1-img2
				max_val = np.max(diff_img)
				min_val = np.min(diff_img)

				# f, (ax1, ax2) = plt.subplots(2)
				plt.figure('Difference')
				plt.title(filename1 + ' - ' + filename2)
				plt.imshow(diff_img)
				plt.figure('Histogramm of Difference')
				plt.title(filename1 + ' - ' + filename2)
				plt.hist(diff_img.ravel(), bins=256, range=(min_val,max_val),histtype='stepfilled')

				plt.show()
	except Exception as e:
		print(e)

	return "Huhu"


if __name__ == "__main__":
	process('/home/stephan/Bilder/boyband2.jpg',
			'/home/stephan/Bilder/dagobert.jpg')
