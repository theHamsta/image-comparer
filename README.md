# CUDA Super Resolution
CUDA Super Resolution is a GPU-accelerated demonstrator for Super Resolution using non-uniform interpolation. It can be used as a command-line program or with an optional GUI (graphical user interface).

## Requirements

These are the requirements for building the project. Note that you will need development versions for most of the packages (on Ubuntu packages that end in -dev)

- CMake for building (version >= 2.8, used 3.5.2)
- Boost (version >= 1.36, libboost-all-dev in Ubuntu)
    - Required components: `boost_system, boost_filesystem, boost_program_options, boost_date_time`
    - Optional components: `boost_log`
- OpenCV (version >= 3.1, used revision: 644c0dd5f41732a782a4d1ae27024731c0c3fc36)
- CUDA  (any version >=3.0 should do the job, used 8.0)
- Thrust (should be included in CUDA Toolkit, libthrust-dev on Ubuntu)
- OpenGL (for gDel2D, maybe necessary to install libxmu-dev libxi-dev)

Optional:

- doxygen for generation of documentation
- graphviz for graphs in documentation
- CTK for fancy popup settings (http://www.commontk.org)
- MATLAB Runtime (optional to support NBSRF, flag WITH_MATLAB_SISR, https://www.mathworks.com/products/compiler/mcr.html)
- libtiff-dev (required by tiff stack support in imagecomparer)

Additional requirements for building the GUI:

- QT (version >= 5.0, used version 5.6.1)
- ffmpeg (avcodec-dev avformat-dev avdevice-dev avutil-dev avfilter-dev avformat-dev swscale-dev, used version 3.0.5)
- video4linux2 (also OpenCV should be built with it)

Tested on the following OS:

- Ubuntu 16.10

But it should also be compatible with Windows and MacOS after adapting /gui/source/YuvFrameSource.cpp. This file currently only looks for the video4linux2 on Linux. Search the file for `dshow` which should be the equivalent on Windows.  I never tested it so maybe you need to adapt it (identifying string for webcam must be set accordingly in the setting files. Currently, `/dev/video0` is used if this values is empty which chooses the first available device on Linux).

**IMPORTANT ADVICE:**

Having a OpenCV version built with libtbb (WITH_TBB), caused various crashes on my PC during the performance of color transforms. This problem was not occuring using pthreads.

## Hardware Requirements
As the software is based on CUDA, it requires a CUDA-capable graphics card from Nvidia.
It should have at least compute capability 3.0 (https://en.wikipedia.org/wiki/CUDA#GPUs_supported), as some special instructions, especially in `NniKernels.cu`, are only available since then.

## Image Comparer
"Image Comparer" is a nice little auxiliary application (at least I like it) that can be used independently from the SR demonstrator.
It uses the widgets that were developed for the webcam demonstrator and is very handy to spot differences of two images of the same size.
Use it if you are developing an image processing algorithm when you have the correct that the algorithm should generate.
The displayed images update automatically if the files change on hard disk.

## Build instructions

    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release  ..
    ccmake . # optional, to set options like compute capability, or activate the documentation
    make -j<number of processors of your PC>

`ccmake .` should be used to set the variable `COMPUTE_CAPABILTY` to the compute capability  of your GPU for optimum performance (https://en.wikipedia.org/wiki/CUDA#GPUs_supported).

![screenshot ccmake](readme.md_images/ccmake_compute_capability.png)

You can also change some switches in this file, for instance compile without Matlab, CTK or GUI. Then type:
The binaries can then be found in one of three subfolders that will be created: `algorithm` for the command-line program, `gui` for the demonstrator application and `imagecomparer`.
Or you can install them via

    sudo make install

The documentation can also be built if doxygen is installed, with nice graphs (requires graphviz). Activate the switch `WITH_DOCUMENTATION` using `ccmake .`

![screenshot ccmake build with documenation](readme.md_images/ccmake_buidl_with_documentation.png)

## Usage (command-line)
To superresolve the video file `SampleVideo_360x240_2mb.mp4` in this repository, increasing its resolution by a factor of 2.5, use:

    cudasuperresolution --upscale 2.5 SampleVideo_360x240_2mb.mp4

For more information on program usage call:

    cudasuperresolution --help

YUV-sequences cannot be opened. However, you can convert them with ffmpeg into png-sequences:

    mkdir PeopleOnStreet
    ffmpeg -pixel_format yuv420p -video_size 2560x1600 -i PeopleOnStreet_2560x1600_30_crop.yuv -f image2 people\ on\ the\ street/image-%3d.png

Also the Y-Channel can be converted to grayscale PNG:

    ffmpeg -pixel_format yuv420p -video_size 832x480 -framerate 60 -i BQMall_832x480_60.yuv -filter_complex "extractplanes=y[y]" -pix_fmt gray -map [y] bqmall-%04d.png

If you want to test a webcam setup, you may find it useful to capture a lovelessly encoded webcam stream to find interesting frames by opening it later in `superresolution_gui`:

	ffmpeg -f v4l2 -i /dev/video0  -codec:v libx264 -qp 0 lossless.mp4

## Usage (GUI)
See documentation of GUI (-> Build instructions).

Webcam parameters (e.g. auto focus) can be changed in real-time on Linux using the programs qv4l2 or v4l2ucp (accessible via `Settings`->`Wecam Settings`)
 
## Third-party software included in this project
- cotire.cmake (compile time reducer): https://github.com/sakra/cotire#cotire
- gDel2D: http://www.comp.nus.edu.sg/~tants/gdel3d.html
- helper_math.h form CUDA Toolkit (is part of the samples: `samples/matrixMul/common/inc/helper_math.h`)
- CUDA BM3d: https://github.com/DawyD/bm3d-gpu
- FindFFMPEG.cmake: https://github.com/robotology/ycm/blob/master/find-modules/FindFFMPEG.cmake
- Breeze icon theme: https://github.com/KDE/breeze-icons
- QProgressIndicator: https://github.com/mojocorp/QProgressIndicator
- QDarkStyleSheet: https://github.com/ColinDuquesnoy/QDarkStyleSheet
- TinyTiff: https://github.com/jkriege2/TinyTIFF.git
The documentation can also be built if doxygen is installed, with nice graphs (requires graphviz). Activate the switch `WITH_DOCUMENTATION` in the top-level `CMakeLists.txt`.

See the above files for full license information.
