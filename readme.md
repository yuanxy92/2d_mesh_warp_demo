# 2D Mesh Warp Render Demo

## How to run

I only test this code on macOS 10.12 + Xcode(Clang) and Visual Studio 2013. Only OpenCV3.2 library is used. I think it can work with OpenCV2 with small modification.

The main function has two parameters, input data path and ouput result path.

Up to now, only CPU is used. The code contains two parts: warp and render. The render part is very inefficient and only for research purpose. The warp part can be accelerated obviously by gpu.

If you run the code successfully, you can get the result in ./kunshan_render/result .

## Input

three kinds of files are used as input

1. images: the wide-angle one is named as ref.jpg, and the remaining images are named using numbers.
2. meshes: %d_mesh_after.txt, contain the information of 2D mesh
3. info.txt: basic information of the mantis camera. resolution scale between the wide angle camera and the others, the numbers of the camera and the positions of the long focal length cameras (in wide angle camera coordinate).

I only use 13 cameras now (one wide angle camera, the middle row and the bottom row). Since the most regions of the images captured by top row cameras are sky, my current algorithm is not capable to handle this well.
There is still some artifacts on the warping result of 4.jpg.



