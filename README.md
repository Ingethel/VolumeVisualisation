# VolumeVisualisation

##Visualisation of volume data using the ray tracing rendering technique and uniform sampling.

The data was first expressed in the world as a box with 6 planes, on which we cast rays. Upon verifying a hit we recursed on that ray with a uniform step which was calculated by the angle of the ray with the plane. Respective 3D points were then converted into intensity values from our data and according to nearest-neighbor or trilinear sampling. The recursion stops when the point is no longer valid. OpenGL was used for rendering the output as a set of 2D points and Pthreads for acceleration.
