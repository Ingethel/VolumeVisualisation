#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>

#include <GL/glut.h>
#include <pthread.h>

#include "Matrix.h"
#include "Volume.h"
#include "Camera.h"

#define GLUT_KEY_ESCAPE 27
#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#define GLUT_WHEEL_DOWN 4
#endif

#define NUMTHREADS 8

// window dimensions
#define WIDTH 500
#define HEIGHT 500

typedef struct{
	float *pixel_r;
	float *pixel_g;
	float *pixel_b;
//	GLubyte data[WIDTH][HEIGHT][3];
	int size;
	int nextIndex;
} OUTPUT;

OUTPUT scene;
pthread_t callThd[NUMTHREADS];
pthread_attr_t attr;
pthread_mutex_t lock;

static Camera* camera = NULL;
static int mouse_left_pressed = 0;
static int mouse_right_pressed = 0;
static int last_x = -1;
static int last_y = -1;
Matrix_4x4 IVieMatrix, IProjMatrix;
static float azimuth = 0.f, polar = 3.5f, radius = 300.f;

// file data
static Volume* head = NULL;

// flags for visualisation approach
const int MAX_VAL = 0;
const int AVG_VAL = 1;
const int ACCUM_VAL = 2;
// current visualisation
static int VISUALISATION = 2;
static std::string vis = "MAX_VAL";

// flags for colour function
const int GREY = 0;
const int RGB = 1;
// current colour function
static int TRANSFER = 1;
static std::string tr = "GREY";

// thresholds for colour functions
float grey_threshold, avg_max_thr, avg_min_thr, rgb_max, rgb_min, scale_1, scale_2, scale_3, scale_4, opac_thr;
unsigned char GLOBAL_MIN_VALUE = 50;
// threshold_to_be_changed flag
unsigned char last_pressed = 0;

// display lag
static float timer;

static int usingThreads = 1;

bool sceneChanged = true;