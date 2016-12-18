#include "Source.h"

#pragma region Print Functions
/**
Printing functions
*/
template<typename T> void printElement(T t1, std::string t2)
{
	std::cout << std::setfill(' ');
	std::cout << '|' << std::left << std::setw(10) << t1 << std::setw(10) << t2 << '|' << std::endl;
}

template<typename T> void printElement(T t1, float t2)
{
	std::cout << std::setfill(' ');
	std::cout << '|' << std::left << std::setw(10) << t1 << std::setw(10) << t2 << '|' << std::endl;
}

void printThresholds(){
	if (TRANSFER == RGB){
		printElement("max_thr", rgb_max);
		printElement("min_thr", rgb_min);
	}
	else{
		if (VISUALISATION == AVG_VAL){
			printElement("min_thr", avg_min_thr);
		}
		else{
			printElement("min_thr", grey_threshold);
		}
	}

}

/*
Print variables in table format
*/
void printInfo(){
#ifdef __unix__
	system("clear");
#else
	system("cls");
#endif
	std::cout << '|' << std::setfill('=') << std::setw(20) << "" << '|' << std::endl;
	std::cout << '|' << std::left << std::setfill(' ') << std::setw(20) << "Visual Parameters" << '|' << std::endl;
	std::cout << '|' << std::left << std::setfill(' ') << std::setw(20) << " " << '|' << std::endl;
	printElement("Vis:", vis);
	printElement("Transfer:", tr);
	std::cout << '|' << std::setfill('-') << std::setw(20) << "" << '|' << std::endl;
	std::cout << '|' << std::left << std::setfill(' ') << std::setw(20) << "Thresholds" << '|' << std::endl;
	std::cout << '|' << std::left << std::setfill(' ') << std::setw(20) << " " << '|' << std::endl;
	printThresholds();
	std::cout << '|' << std::setfill('-') << std::setw(20) << "" << '|' << std::endl;
	printElement("Display:", timer);
	std::cout << '|' << std::setfill('=') << std::setw(20) << "" << '|' << std::endl;
}
#pragma endregion

#pragma region Colour Transfer Functions
/*
Rainbow mapping
with extended low and high bounds
*/
Vector4 transferFunc_RGB(float value){
	float r, g, b, a;
	value -= rgb_min;
	float scale = std::min(1.f, value / (rgb_max - rgb_min)) * 4;
	float portion = scale - floor(scale);

	if (value < 0){
		r = 0.;
		g = 0.;
		b = 0.;
		a = 0.f;
	}
	else{
		if (scale >= scale_4){
			r = 1.;
			g = 0.;
			b = 0.;
			a = 0.8f;
		}
		else if (scale >= scale_3){
			r = 1.;
			g = 1 - portion;
			b = 0.;
			a = 0.6f;
		}
		else if (scale >= scale_2){
			r = portion;
			g = 1.;
			b = 0.;
			a = 0.4f;
		}
		else if (scale >= scale_1){
			r = 0.;
			g = 1.;
			b = 1 - portion;
			a = 0.2f;
		}
		else{
			r = 0.;
			g = portion;
			b = 1.;
			a = 0.f;
		}
	}

	return Vector4(r, g, b, a);
}

/*
Grey mapping
*/
Vector4 transferFunc_Grey(float value){
	float a;

	if (value > grey_threshold)
		a = (value - grey_threshold) / (255. - grey_threshold);
	else
		a = 0.f;

	return Vector4(a, a, a, a);
}

/*
Returns the colour that corresponds to the given value and pre-set transfer function
*/
Vector3 getColour(float value){
	if (TRANSFER == GREY)
		return transferFunc_Grey(value);
	else
		return transferFunc_RGB(value);
}

/*
initial threshold values - reset
*/
void initialColorThreshold(){

	scale_1 = 1.f;
	scale_2 = 2.f;
	scale_3 = 3.f;
	scale_4 = 4.f;

	grey_threshold = 0;

	avg_max_thr = .5f;
	avg_min_thr = .0f;

	if (VISUALISATION == AVG_VAL){
		rgb_max = 110;
		rgb_min = 40;
		vis = "AVG_VAL";
	}
	else if (VISUALISATION == MAX_VAL){
		rgb_max = 220;
		rgb_min = 70;
		vis = "MAX_VAL";
	}
	else if (VISUALISATION == ACCUM_VAL){
		rgb_max = 220;
		rgb_min = 40;
		//grey_threshold = 90;
		vis = "ACC_VAL";
	}

	if (TRANSFER == GREY){
		tr = "GREY";
	}
	else{
		tr = "RGB";
	}
	opac_thr = .95f;
}
#pragma endregion

#pragma region Visualisation Functions

/*
Used to update View and Projection Matrices
*/
void UpdateSeceneMatrices(){
	IVieMatrix = Matrix_4x4::Inverse(Matrix_4x4::ViewLookAt(camera->GetPosition(), camera->GetTarget(), Vector3::Vector3(0.f, 1.f, 0.f)));
	IProjMatrix = Matrix_4x4::Inverse(Matrix_4x4::Perspective(camera->GetFOV(), camera->GetNearClipPlane(), camera->GetFarClipPlane(), (float)WIDTH / (float)HEIGHT));
}

Vector3 MaxValue(Vector3 point, Vector3 step){
	unsigned char max = 0;

	while (head->Validate(point)){
		unsigned char val = head->Get(point);
		if (val > GLOBAL_MIN_VALUE)
			if (max < val)
				max = val;
		point += step;
	}

	return getColour(max);
}

Vector3 AvgValue(Vector3 point, Vector3 step){
	unsigned char avg_min = 255;
	unsigned char avg_max = 0;

	while (head->Validate(point)){
		unsigned char val = head->Get(point);
		if (val > GLOBAL_MIN_VALUE){
			if (avg_min > val)
				avg_min = val;
			if (avg_max < val)
				avg_max = val;
		}
		point += step;
	}

	if (avg_max == 0){
		return Vector3::Zero();
	}
	else{
		float a = (avg_min + avg_max) / 2.f;
		return getColour(a);
	}
}

Vector3 AccValue(Vector3 point, Vector3 step){
	float acc_opacity = 0, acc_colour = 0;

	while (head->Validate(point)){
		unsigned char val = head->Get(point);
		if (val > GLOBAL_MIN_VALUE){
			// assuming opacity = scalar/5 ___  to be fixed
			float currentValue = val / 255.f;
			float currentOpacity = currentValue/5;
			acc_opacity = currentOpacity + (1.0 - currentOpacity)*acc_opacity;
			acc_colour = acc_colour + (currentValue*currentOpacity)*(1 - acc_opacity);
			if (acc_opacity > opac_thr){
				break;
			}
		}
		point += step;
	}

	return getColour(acc_colour * 255);
}

/*
Return the appropriate colour for the given pixel, step size and the pre-set visualisation method
*/
Vector3 GetPointColour(Vector3 point, Vector3 step){
	if (VISUALISATION == AVG_VAL){
		return AvgValue(point, step);
	}
	else if (VISUALISATION == ACCUM_VAL){
		return AccValue(point, step);
	}
	else{
		return MaxValue(point, step);
	}
}

/*
Ray Tracing Visualisation of Volume Data
*/
Vector3 castRay(int x, int y, Matrix_4x4 IVieMatrix, Matrix_4x4 IProjMatrix){

	float pixelX = 2 * ((x + 0.5f) / WIDTH) - 1;
	float pixelY = -2 * ((y + 0.5f) / HEIGHT) + 1;

	Vector4 worldNear = IVieMatrix * IProjMatrix * Vector4::Vector4(pixelX, pixelY, -1, 1);
	Vector4 worldFar = IVieMatrix * IProjMatrix * Vector4::Vector4(pixelX, pixelY, 1, 1);

	Vector3 worldNearPos = Vector3::Vector3(worldNear.x, worldNear.y, worldNear.z) / worldNear.w;
	Vector3 worldFarPos = Vector3::Vector3(worldFar.x, worldFar.y, worldFar.z) / worldFar.w;
	Vector3 dir = Vector3::Normalize(worldFarPos - worldNearPos);

	Ray ray(worldNearPos, dir);

	INTERSECTION result = head->Intersect(ray);
	int xIndex, yIndex, zIndex;

	if (result.intersection){
		// initiale iterations for margin errors
		for (int tries = 0; tries < 3; tries++){
			if (head->Validate(result.point)){
				break;
			}
			else{
				result.point += result.step;
			}
		}
		return GetPointColour(result.point, result.step);
	}

	return Vector3::Zero();
}
#pragma endregion

#pragma region Thread Methods
void *thread_work(void *arg){

	int currentIndex = 0;
	Vector3 colour;

	while (currentIndex < scene.size){

		pthread_mutex_lock(&lock);
		currentIndex = scene.nextIndex;
		scene.nextIndex++;
		pthread_mutex_unlock(&lock);

		int x = currentIndex % WIDTH;
		int y = currentIndex / WIDTH;

		colour = castRay(x, y, IVieMatrix, IProjMatrix);

//		scene.data[x][y][0] = colour.r() * 256;
//		scene.data[x][y][1] = colour.g() * 256;
//		scene.data[x][y][2] = colour.b() * 256;
		scene.pixel_r[currentIndex] = colour.r();
		scene.pixel_g[currentIndex] = colour.g();
		scene.pixel_b[currentIndex] = colour.b();
	}

	//	pthread_exit((void*)0);
	return NULL;
}

/*
Draw on screen given data
*/
void DrawData(OUTPUT &o){
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2.0);
	glBegin(GL_POINTS);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int index = x + y * WIDTH;
			glColor3f(o.pixel_r[index], o.pixel_g[index], o.pixel_b[index]);
			glVertex3f(y, x, 0);
		}
	}
	glEnd();
	
	glFlush();
//	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, o.data);
	glutSwapBuffers();
}

void initialise_thread_variables(){
	scene.size = WIDTH*HEIGHT;
	scene.pixel_r = (float*)malloc(WIDTH*HEIGHT*sizeof(float));
	scene.pixel_g = (float*)malloc(WIDTH*HEIGHT*sizeof(float));
	scene.pixel_b = (float*)malloc(WIDTH*HEIGHT*sizeof(float));

	pthread_mutex_init(&lock, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}
#pragma endregion

#pragma region RenderMethods
/*
Non-Threaded Visualisation
*/
void Draw() {
	std::clock_t start = std::clock();

	Vector3 colour;
	
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2.0);

	glBegin(GL_POINTS);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			colour = castRay(x, y, IVieMatrix, IProjMatrix);
			glColor3f(colour.r(), colour.g(), colour.b());
			glVertex3f(y, x, 0);
		}
	}
	glEnd();

	glFlush();
	glutSwapBuffers();

	timer = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) / 1000;
	printInfo();
}

/*
Threaded Visualisation
*/
void DrawThreads(){
	if (sceneChanged){
		std::clock_t start = std::clock();
		scene.nextIndex = 0;
		void *status;
		// start threads
		for (int i = 0; i < NUMTHREADS; i++){
			pthread_create(&callThd[i], &attr, thread_work, (void *)i);
		}
		// wait to finish
		for (int i = 0; i < NUMTHREADS; i++){
			pthread_join(callThd[i], &status);
		}
		sceneChanged = false;

		// draw output
		DrawData(scene);

		timer = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) / 1000;
		printInfo();
	}
}

void Render(){
	if (usingThreads)
		DrawThreads();
	else
		Draw();
}
#pragma endregion

#pragma region Input Functions

/*
Key handler
*/
void KeyEvent(unsigned char key, int x, int y) {

	switch (key) {
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
		break;
	case 'u': case 'l': case 'U': case 'L':
		last_pressed = key;
		break;
	case 'r': case 'R':
		initialColorThreshold();
		break;
	case 'a': case 'A':
		azimuth -= .1f;
		if (azimuth < 0)
			azimuth = 3.147 * 2;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		break;
	case 'd': case 'D':
		azimuth += .1f;
		if (azimuth > 3.147 * 2)
			azimuth = 0;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		break;
	case 's': case 'S':
		polar -= .1f;
		if (polar < 0)
			polar = 3.147 * 2;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		break;
	case 'w': case 'W':
		polar += .1f;
		if (polar > 3.147 * 2)
			polar = 0;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		break;
	}
	sceneChanged = true;
}

/*
Key handler
*/
void KeyEventSpecial(int key, int x, int y) {
	switch (key) {
		// exit
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
		break;
		// change threshold
	case GLUT_KEY_UP:
		if (TRANSFER == RGB){
			if (last_pressed == 'u' || last_pressed == 'U'){ rgb_max = (rgb_max >= 255.f) ? 255.f : rgb_max + 1.f; }
			else if (last_pressed == 'l' || last_pressed == 'L'){ rgb_min = (rgb_min >= rgb_max) ? rgb_max : rgb_min + 1.f; }
		}
		else if (TRANSFER == GREY){
			if (VISUALISATION == AVG_VAL){
				avg_min_thr = (avg_min_thr >= .5f) ? .5f : avg_min_thr + 0.01;
			}
			else{
				grey_threshold = (grey_threshold >= 255.f) ? 255.f : grey_threshold + 1.f;
			}
		}
		break;
	case GLUT_KEY_DOWN:
		if (TRANSFER == RGB){
			if (last_pressed == 'u' || last_pressed == 'U'){ rgb_max = (rgb_max <= rgb_min) ? rgb_min : rgb_max - 1.f; }
			else if (last_pressed == 'l' || last_pressed == 'L'){ rgb_min = (rgb_min <= 0.f) ? 0.f : rgb_min - 1.f; }
		}
		else if (TRANSFER == GREY){
			if (VISUALISATION == AVG_VAL){
				avg_min_thr = (avg_min_thr <= 0.f) ? 0.f : avg_min_thr - 0.01;
			}
			else{
				grey_threshold = (grey_threshold <= 0.f) ? 0.f : grey_threshold - 1.f;
			}
		}
		break;
		// change visualisation
	case GLUT_KEY_F1:
		VISUALISATION = MAX_VAL;
		vis = "MAX_VAL";
		break;
	case GLUT_KEY_F2:
		VISUALISATION = AVG_VAL;
		vis = "AVG_VAL";
		break;
	case GLUT_KEY_F3:
		VISUALISATION = ACCUM_VAL;
		vis = "ACCUM_VAL";
		break;
		// change transfer function
	case GLUT_KEY_F10:
		TRANSFER = GREY;
		tr = "GREY";
		break;
	case GLUT_KEY_F11:
		TRANSFER = RGB;
		tr = "RGB";
		break;
	}
	sceneChanged = true;
}

/*
OpenGL mouse events for camera movement
*/
void MouseMoveEvent(int x, int y) {
	if (mouse_left_pressed) {
		int diff_x = x - last_x;
		int diff_y = y - last_y;

		azimuth += diff_x*0.05;
		polar += diff_y*0.05;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		last_x = x;
		last_y = y;
	}
	sceneChanged = true;
}

/*
OpenGL mouse events for camera movement
*/
void MouseEvent(int button, int state, int x, int y) {

	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_UP) { mouse_left_pressed = 0; }
		if (state == GLUT_DOWN) { mouse_left_pressed = 1; last_x = x; last_y = y; }
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_UP) { mouse_right_pressed = 0; }
		if (state == GLUT_DOWN) { mouse_right_pressed = 1; last_x = x; last_y = y; }
		break;

	case GLUT_WHEEL_UP:
		radius -= 1;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		break;
	case GLUT_WHEEL_DOWN:
		radius += 1;
		camera->SetPosition_Sperical(radius, azimuth, polar);
		UpdateSeceneMatrices();
		break;
	}
	sceneChanged = true;
}
#pragma endregion

/*
Idle
*/
void Update() {
	glutPostRedisplay();
}

void clean(){
	std::cout << "...Deallocating memory..." << std::endl;
	std::clock_t start = std::clock();
	delete head;
	delete camera;
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&lock);
	std::cout << "Done " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 100) / 100 << " s" << std::endl;
}

void readFile(){
	std::cout << "...Reading file..." << std::endl;
	std::clock_t start = std::clock();
	head = new Volume("head");
	head->SetSamplingMethod(Volume::Trilinear);
	std::cout << "Done " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 100) / 100 << " s" << std::endl;
}

void initParam(int argc, char **argv){
	// initialise needed thresholds
	initialColorThreshold();

	initialise_thread_variables();
	// define window parameter and other opengl stuff
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Volume Visualisation");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIDTH, HEIGHT, 0, -512, 512);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	camera = new Camera(
		Vector3::Zero(),
		Vector3(head->GetWidth() / 2, head->GetHeight() / 2, head->GetDepth() / 2),
		15.f, 1.f, 10000.f);
	camera->SetPosition_Sperical(radius, azimuth, polar);
	UpdateSeceneMatrices();
}

int main(int argc, char **argv) {
	readFile();
	initParam(argc, argv);
	glutMouseFunc(MouseEvent);
	glutMotionFunc(MouseMoveEvent);
	glutKeyboardFunc(KeyEvent);
	glutSpecialFunc(KeyEventSpecial);
	glutDisplayFunc(Render);
	glutIdleFunc(Update);
	glutMainLoop();
	atexit(clean);
};
