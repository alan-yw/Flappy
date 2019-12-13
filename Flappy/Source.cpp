/******************************************
*
* Official Name:  Wei Yang
*
* E-mail:  wyang19@syr.edu
*
* Final Project: Flappy
*
* Environment/Compiler:  Visual Studio 2015
*
* Date:  April 24, 2016
*
*******************************************/

#include <cmath>
#include <iostream>
#include <deque>
#include <time.h> 
#include <stdlib.h>
#include <fstream>
#include <string>

using namespace std;
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265
//Global variables
static int view = 0;
static int zoom = 0;
#define MAX_NUM_PARTICLES 1000
#define INITIAL_NUM_PARTICLES 100
#define INITIAL_POINT_SIZE 5.0
#define INITIAL_SPEED 1.0
//windows
//windows
static int window01, window02;
static GLsizei width01, height01; // window size.
static GLsizei width02, height02;
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate objects.
static float theta = 0.0; // Angle of the sun with the ground.
int closestName;
static float pointSize = 3.0; // Size of point
static int hits;
static unsigned int buffer[1024];
bool picking = false;
float yVelocity = 0, yMove = 0, zMove = 0, rotation = 0;
float X = 0, Y = 0, Z = 0, eyeX = 0, eyeZ = 0, angle = 0, angleY = 0;
static bool flappingflag = true;
static bool isJump = false;
static float wingsangle = 100;
static float pipes_trans_z = 0, trans_y = -600, trans_z = 0;
static bool flapdirection = false; //false + true - direction
static float jumpspeed = 6.5;
static float vertspeed = 0;
static float Max_Floor = 150;
static float fallingConstant = 4.9;
static float Min_Floor = -50;
static int num_pipe = 0;
static bool end_flag = false;
deque<float> pipes_height;
int score = 0;
static unsigned int texture[3]; // Array of texture indices.
static bool isFog = false; // Is fog on?
static int fogMode = GL_EXP2; // Fog mode.
static float fogDensity = 0.005; // Fog density.
static float fogStart = 0.0; // Fog start z value.
static float fogEnd = 100.0; // Fog end z value.
static long font = (long)GLUT_BITMAP_TIMES_ROMAN_24; // Font selection.
static bool timer_flag = false;
static float square_color[3] = { 0.7,0.7,0.0 };
int present_time;
int last_time;
int num_particles = INITIAL_NUM_PARTICLES;
float point_size = INITIAL_POINT_SIZE;
float speed = INITIAL_SPEED;
bool gravity = false; /* gravity off */
bool elastic = false; /* restitution off */
bool repulsion = false; /* repulsion off */
float coef = 1.0; /* perfectly elastic collisions */
float d2[MAX_NUM_PARTICLES][MAX_NUM_PARTICLES]; /* array for interparticle distances */

GLfloat colors[8][3] = { { 0.0, 0.0, 0.0 },{ 1.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0 },
{ 0.0, 0.0, 1.0 },{ 0.0, 1.0, 1.0 },{ 1.0, 0.0, 1.0 },{ 1.0, 1.0, 0.0 },
{ 1.0, 1.0, 1.0 } };

char* str0 = (char*)"Welcome to 3D Flappy Bird!";
char* str1 = (char*)"Brief Instruction:";
char* str2 = (char*)"1. Press S button to start or restart";
char* str3 = (char*)"2. Press Q button to pause the game";
char* str4 = (char*)"3. Press F button to enable the fog mode";
char* str5 = (char*)"4. Press N or M button to toggle the daily or night mode";
char* str6 = (char*)"5. Right click will have a menu can change color of the bird ";
char* str7 = (char*)"6. Press space or left click the mouse to jump";
char* str8 = (char*)"   Debug usage - Arrow key to turn around and move front and back";
char* str9 = (char*)"   Shift + Arrow key (up and down) to look up and down";


// Process hit buffer to find record with smallest min-z value.
// Copied (almost) exactly from BallAndTorusPicking
void findClosestHit(int hits, unsigned int buffer[])
{
	unsigned int* ptr, minZ;

	minZ = 0xffffffff; // 2^32 - 1
	ptr = buffer;
	closestName = 0;
	for (int i = 0; i < hits; i++)
	{
		ptr++;
		if (*ptr < minZ)
		{
			minZ = *ptr;
			ptr += 2;
			closestName = *ptr;
			ptr++;
		}
		else ptr += 3;
	}
} // end findClosestHit

typedef struct particle
{
	int color;
	float position[3];
	float velocity[3];
	float mass;
} particle;

particle particles[MAX_NUM_PARTICLES]; /* particle system */

void myinit()
{
	int  i, j;

	/* set up particles with random locations and velocities */

	for (i = 0; i < num_particles; i++)
	{
		particles[i].color = i % 8;
		if (particles[i].color == 1) //red particles have more mass
			particles[i].mass = 10.0;
		else
			particles[i].mass = 1.0;

		for (j = 0; j < 3; j++)
		{
			particles[i].position[j] = 2.0 * ((float)rand() / RAND_MAX) - 1.0;
			particles[i].velocity[j] = speed * 2.0 * ((float)rand() / RAND_MAX) - 1.0;
		}
	}
	glPointSize(point_size);


	/* set clear color to grey */

	glClearColor(0.5, 0.5, 0.5, 1.0);
}

float forces(int i, int j)
{
	int k;
	float force = 0.0;
	if (gravity && j == 1) force = -1.0; /* simple gravity */
	if (repulsion) for (k = 0; k < num_particles; k++)  /* repulsive force */
	{
		if (k != i) force += 0.001 * (particles[i].position[j] - particles[k].position[j]) / (0.001 + d2[i][k]);
	}
	return(force);
}

void collision(int n)

/* tests for collisions against cube and reflect particles if necessary */

{
	int i;
	for (i = 0; i < 3; i++)
	{
		if (particles[n].position[i] >= 1.0)
		{
			particles[n].velocity[i] = -coef * particles[n].velocity[i];
			particles[n].position[i] = 1.0 - coef * (particles[n].position[i] - 1.0);
		}
		if (particles[n].position[i] <= -1.0)
		{
			particles[n].velocity[i] = -coef * particles[n].velocity[i];
			particles[n].position[i] = -1.0 - coef * (particles[n].position[i] + 1.0);
		}
	}
}

void myIdle()
{
	int i, j, k;
	float dt;
	present_time = glutGet(GLUT_ELAPSED_TIME);
	dt = 0.001 * (present_time - last_time);
	for (i = 0; i < num_particles; i++)
	{
		for (j = 0; j < 3; j++)
		{
			particles[i].position[j] += dt * particles[i].velocity[j];
			particles[i].velocity[j] += dt * forces(i, j) / particles[i].mass;
		}
		collision(i);
	}
	if (repulsion) for (i = 0; i < num_particles; i++) for (k = 0; k < i; k++)
	{
		d2[i][k] = 0.0;
		for (j = 0; j < 3; j++) d2[i][k] += (particles[i].position[j] -
			particles[k].position[j]) * (particles[i].position[j] -
				particles[k].position[j]);
		d2[k][i] = d2[i][k];
	}
	last_time = present_time;
	if (flappingflag == true)
	{
		if (wingsangle == 600) {
			flapdirection = false;
		}
		else if (wingsangle == -600) {
			flapdirection = true;
		}
		(flapdirection == true) ? wingsangle += 50 : wingsangle -= 50;

	}
	glutPostRedisplay();
}



void writeBitmapString(void* font, char* string)
{
	char* c;
	for (c = string; *c != '\0'; c++)
	{

		glutBitmapCharacter(font, *c);
	}
}
void writescore(void* font, char* string)
{
	char* c;
	for (c = string; *c != '\0'; c++)
	{

		glutBitmapCharacter(font, *c);
	}
	std::string str = to_string(score);
	char* C;
	for (C = (char*)str.c_str(); *C != '\0'; C++)
	{
		glutBitmapCharacter(font, *C);
	}

}
void writeString(void* font, char* string)
{
	char* c;
	for (c = string; *c != '\0'; c++)
	{
		glLineWidth(3.0);
		glutStrokeCharacter(font, *c);
	}
}

float Max(float coord01, float coord02) {
	if (coord01 > coord02)
		return coord01;
	return coord02;


}
float Min(float coord01, float coord02) {
	if (coord01 > coord02)
		return coord02;
	return coord01;

}

//modified algorithm from https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection


void Collision_detection(void) {
	float x = 0;
	float y_top = Max(20 + pipes_height[0], yMove);
	float y_bottom = Min(pipes_height[0], yMove);

	float z = 30 - pipes_trans_z;
	float distance_top = sqrtf((y_top - yMove) * (y_top - yMove));
	float distance_bottom = sqrtf((y_bottom - yMove) * (y_bottom - yMove));
	if (distance_top < 2 || distance_bottom < 2) {
		timer_flag = false;
		end_flag = true;
		pipes_trans_z = 0;
	}

}

void Timer(int value)
{
	if (timer_flag) {
		while (pipes_height.size() < 5)
		{
			float pipeH = rand() % 14 - 7;
			pipes_height.push_back(pipeH);
		}
		if (isJump == true)
		{
			vertspeed = jumpspeed;
			isJump = false;
		}
		if (yMove > 60) {
			yMove = 60;
		}
		else if (yMove <= -60) {
			yMove = -60;
			timer_flag = false;

		}
		else
			yMove += vertspeed * .1;

		if (pipes_trans_z < 30) {
			pipes_trans_z += 0.1;
		}
		else {
			pipes_height.pop_front();
			pipes_trans_z = 0;
			score += 1;
		}
		if (pipes_trans_z > 20) {
			Collision_detection();

		}
		vertspeed -= fallingConstant * .1;
		glutPostRedisplay();
	}



	glutTimerFunc(10, Timer, 0);
}

void drawlawn()
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glBlendFunc(GL_ONE, GL_ZERO);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-200, -100, 200);
	glTexCoord2f(1.0, 0.0); glVertex3f(200, -100, 200);
	glTexCoord2f(1.0, 1.0); glVertex3f(200, -100, -200);
	glTexCoord2f(0.0, 1.0); glVertex3f(-200, -100, -200);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Begin globals.
// Initial control points.
static float controlPoints[6][4][3] =
{
	{ { -3.0, -30, 5.0 },{ -0.25, 5.0, 5.0 },{ 0.25, -30, 5.0 },{ 3.0, -30, 5.0 } },
	{ { -3.0, -30, 3.0 },{ -0.25, 5.0, 3.0 },{ 0.25, -30, 3.0 },{ 3.0, -30, 3.0 } },
	{ { -3.0, -30, 1.0 },{ -0.25, 5.0, 1.0 },{ 0.25, -30, 1.0 },{ 3.0, -30, 1.0 } },
	{ { -3.0, -30, -1.0 },{ -0.25, 5.0, -1.0 },{ 0.25, -30, -1.0 },{ 3.0, -30, -1.0 } },
	{ { -3.0, -30, -3.0 },{ -0.25, 5.0, -3.0 },{ 0.25, -30, -3.0 },{ 3.0, -30, -3.0 } },
	{ { -3.0, -30, -5.0 },{ -0.25, 5.0, -5.0 },{ 0.25, -30, -5.0 },{ 3.0, -30, -5.0 } },
};

void drawBezierSurface()
{
	// Specify and enable the Bezier surface.
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 6, controlPoints[0][0]);
	glEnable(GL_MAP2_VERTEX_3);


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// Draw the Bezier surface using a mesh approximation.
	glColor3f(0.0, 0.0, 0.0);
	glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
	glEvalMesh2(GL_LINE, 0, 20, 0, 20);

	glLineWidth(1.0);
}

void drawsky()
{
	float alpha; // Blending parameter.
	glEnable(GL_TEXTURE_2D);
	//front sky
	glPushMatrix();
	//glBlendFunc(GL_ONE, GL_ZERO); // Specify blending parameters to overwrite background.
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Specify blending parameters to mix skies.
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, -200.0, -200.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(200.0, -200.0, -200.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(200.0, 200.0, -200.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, 200.0, -200.0);
	glEnd();
	if (theta <= 90.0) alpha = theta / 90.0;
	else alpha = (180.0 - theta) / 90.0;
	glColor4f(1.0, 1.0, 1.0, alpha);

	// disabling the depth test allows this as both have the same z-values
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Specify blending parameters to mix skies.
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, -200.0, -200.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(200.0, -200.0, -200.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(200.0, 200.0, -200.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, 200.0, -200.0);
	glEnd();


	glPopMatrix();
	glColor4f(1.0, 1.0, 1.0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	//left sky
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, -200.0, -200.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-200.0, -200.0, 200.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-200.0, 200.0, 200.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, 200.0, -200.0);
	glEnd();
	glPopMatrix();
	//right sky
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(200.0, -200.0, -200.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(200.0, -200.0, 200.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(200.0, 200.0, 200.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(200.0, 200.0, -200.0);
	glEnd();
	glPopMatrix();
	//celling sky
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, 200.0, -200.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(200.0, 200.0, -200.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(200.0, 200.0, 200.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, 200.0, 200.0);
	glEnd();
	glPopMatrix();
	//back sky
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-200.0, -200.0, 200);
	glTexCoord2f(1.0, 0.0); glVertex3f(200.0, -200.0, 200);
	glTexCoord2f(1.0, 1.0); glVertex3f(200.0, 200.0, 200);
	glTexCoord2f(0.0, 1.0); glVertex3f(-200.0, 200.0, 200);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}
void drawbird() {

	glRotatef(90, 0, 1, 0);
	glColor3f(square_color[0], square_color[1], square_color[2]);
	glTranslatef(0, trans_y / 50, 0);

	float R = 5;
	glPushMatrix();
	glScalef(1.0, 1.0, 1.0);
	glutSolidSphere(2, 100, 100);

	glColor3f(0, 0, 0);
	glScalef(1, 0.1, 1);
	glTranslatef(0, -800, 0);
	glutSolidSphere(2, 20, 20);
	glPopMatrix();
	glColor3f(0.5, 0.5, 0.5);
	//right wing
	glPushMatrix();
	glTranslatef(0, 0, 1.0);
	glRotatef(-(wingsangle / 10), 1, 0, 0);
	glScalef(0.2, 0.03, 0.45);
	glutSolidSphere(5, 100, 100);
	glPopMatrix();

	//left wing
	glPushMatrix();
	glTranslatef(0, 0, -1.0);
	glRotatef(wingsangle / 10, 1, 0, 0);
	glScalef(0.2, 0.03, 0.45);
	glutSolidSphere(5, 100, 100);
	glPopMatrix();
	//eye
	glColor3f(0.8, 0.8, 0.8);
	glPushMatrix();
	glTranslatef(1.2, .8, .5);
	glutSolidSphere(0.8, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(1.2, .8, -.5);
	glutSolidSphere(0.8, 30, 30);
	glPopMatrix();
	//eyeball
	glColor3f(0.0, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(1.8, 1.0, -.4);
	glutSolidSphere(0.2, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(1.8, 1.0, .4);
	glutSolidSphere(0.2, 30, 30);
	glPopMatrix();
	//mouth
	glColor3f(0.7, 0.1, 0.1);
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glPushMatrix();
	glTranslatef(1.4, 0, 0.3);
	glScalef(1, 1.2, 1);
	glutSolidTorus(0.2, 0.9, 30, 60);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(1.4, 0, .1);
	glScalef(1, 1.2, 1);
	glutSolidTorus(0.2, 0.9, 30, 60);
	glPopMatrix();
	glPopMatrix();
}


void drawpipes(void) {
	glPushMatrix();

	glPushMatrix();
	glTranslatef(0, 50, 0);
	glColor3f(0.1, 0.8, 0.1);
	glRotatef(90, 1, 0, 0);
	glScalef(1, 1, 100);
	glutSolidTorus(0.4, 2.5, 60, 100);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 10, 0);
	glColor3f(0.1, 0.8, 0.1);
	glRotatef(90, 1, 0, 0);
	glScalef(1, 1, 5);
	glutSolidTorus(0.4, 3, 60, 100);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -50, 0);
	glColor3f(0.1, 0.8, 0.1);
	glRotatef(90, 1, 0, 0);
	glScalef(1, 1, 100);
	glutSolidTorus(0.4, 2.5, 60, 100);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -10, 0);
	glColor3f(0.1, 0.8, 0.1);
	glRotatef(90, 1, 0, 0);
	glScalef(1, 1, 5);
	glutSolidTorus(0.4, 3, 60, 100);
	glPopMatrix();

	glPopMatrix();
}




// Struct of bitmap file.
struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char* data;
};

// Routine to read a bitmap file.
// Works only for uncompressed bmp files of 24-bit color.
BitMapFile* getBMPData(string filename)
{
	BitMapFile* bmp = new BitMapFile;
	unsigned int size, offset, headerSize;

	// Read input file name.
	ifstream infile(filename.c_str(), ios::binary);

	// Get the starting point of the image data.
	infile.seekg(10);
	infile.read((char*)& offset, 4);

	// Get the header size of the bitmap.
	infile.read((char*)& headerSize, 4);

	// Get width and height values in the bitmap header.
	infile.seekg(18);
	infile.read((char*)& bmp->sizeX, 4);
	infile.read((char*)& bmp->sizeY, 4);

	// Allocate buffer for the image.
	size = bmp->sizeX * bmp->sizeY * 24;
	bmp->data = new unsigned char[size];

	// Read bitmap data.
	infile.seekg(offset);
	infile.read((char*)bmp->data, size);

	// Reverse color from bgr to rgb.
	int temp;
	for (int i = 0; i < size; i += 3)
	{
		temp = bmp->data[i];
		bmp->data[i] = bmp->data[i + 2];
		bmp->data[i + 2] = temp;
	}

	return bmp;
}


// Load external textures.
void loadExternalTextures()
{
	// Local storage for bmp image data.
	BitMapFile* image[3];

	// Load the textures.
	image[0] = getBMPData("Textures/grass.bmp");
	image[1] = getBMPData("Textures/sky.bmp");
	image[2] = getBMPData("Textures/night.bmp");
	// Bind grass image to texture index[0]. 
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);

	// Bind sky image to texture index[1]
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[1]->sizeX, image[1]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[1]->data);

	// Bind night image to texture index[2]
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[2]->sizeX, image[2]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[2]->data);
}

void setup01(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	srand(time(NULL));
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	// Create light components
	GLfloat ambientLight[] = { 0.3f, 0.3f, 0.f, 1.0f };
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat position01[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Assign created components to GL_LIGHT0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position01);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1);

	float matShine[] = { 20.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);


	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
	// Create texture index array and load external textures.
	glGenTextures(2, texture);
	loadExternalTextures();

	// Turn on OpenGL texturing.
	glEnable(GL_TEXTURE_2D);

	// Specify how texture values combine with current surface color values.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	while (pipes_height.size() < 5)
	{
		float pipeH = rand() % 14 - 7;
		pipes_height.push_back(pipeH);
	}
	//blending
	glEnable(GL_BLEND); // Enable blending.


}

// Drawing routine.
void drawScene01(void)
{
	float fogColor[4] = { 0.5, 0.5, 0.5, 1.0 };
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Write text in isolated (i.e., before gluLookAt) translate block.
	if (isFog) glEnable(GL_FOG);
	else glDisable(GL_FOG);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogi(GL_FOG_MODE, fogMode);
	glFogf(GL_FOG_DENSITY, fogDensity);
	gluPerspective(90.0, width01 / height01, 0.1, 400.0);

	if (flappingflag) {
		glutIdleFunc(myIdle);
	}


	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	drawsky();
	//partial system

	int i;
	glBegin(GL_POINTS); /* render all particles */
	for (i = 0; i < num_particles; i++)
	{
		glColor3fv(colors[particles[i].color]);
		glVertex3fv(particles[i].position);
	}
	glEnd();
	if (!end_flag) {
		gluLookAt(X - 10 * sin((PI / 180.0) * angle),
			0.0,
			Z - cos((PI / 180.0) * angle),
			X - 11 * sin((PI / 180.0) * angle),
			Y,
			Z - 2 * cos((PI / 180.0) * angle),
			0.0,
			1.0,
			0.0);
		glPushMatrix();
		glTranslatef(0, yMove, -30);
		drawbird();
		glPopMatrix();
	}

	else {
		gluLookAt(0,
			0.0,
			0,
			0,
			-10,
			-1,
			0.0,
			1.0,
			0.0);
		glPushMatrix();
		glTranslatef(0, -50, 0);
		glRotatef(90, 1, 0, 0);
		drawbird();
		glPopMatrix();
	}

	drawBezierSurface();
	drawlawn();
	for (num_pipe = 0; num_pipe < 4; num_pipe++) {

		glPushMatrix();
		glTranslatef(0, pipes_height[num_pipe], -55 - 30 * (num_pipe)+pipes_trans_z);
		drawpipes();
		glPopMatrix();
	}


	glLoadIdentity();
	glPushMatrix();
	if (theta > 60)
		glColor3f(1.0, 1.0, 1.0);
	else
		glColor3f(0.0, 0.0, 0.0);
	glRasterPos3f(-1.0, 3.0, -5.0);
	writescore((void*)font, (char*)"Score: ");
	glPopMatrix();



	glutSwapBuffers();
}

void drawScene02(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glColor3f(0.2, 0.2, 0.6);
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-15, 80, 0);
	glScalef(0.06, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str0);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, 60, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str1);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, 40, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str2);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, 20, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str3);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, 0, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str4);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, -20, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str5);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, -40, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str6);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, -60, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str7);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, -80, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str8);
	glPopMatrix();
	glPushMatrix();
	glLineWidth(1.0);
	glTranslatef(-95, -100, 0);
	glScalef(0.04, 0.1, 0.1);
	writeString(GLUT_STROKE_MONO_ROMAN, str9);
	glPopMatrix();
	glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize01(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0, w / h, 0.1, 400.0);
	width01 = w;
	height01 = h;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void resize02(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-100, 180, -120, 100, 0, 200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}



void setup02(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
}


void top_menu(int id)
{
	if (id == 1) exit(0);
}

void color_menu(int id)
{
	if (id == 2)
	{
		square_color[0] = 0.7; square_color[1] = 0.7; square_color[2] = 0.0;
	}
	if (id == 3)
	{
		square_color[0] = 0.0; square_color[1] = 0.0; square_color[2] = 1.0;
	}
	glutPostRedisplay();
}
void makeMenu(void)
{
	// The sub-menu is created first (because it should be visible when the top
	// menu is created): its callback function is registered and menu entries added.
	int sub_menu01;
	sub_menu01 = glutCreateMenu(color_menu);
	glutAddMenuEntry("Yello", 2);
	glutAddMenuEntry("Blue", 3);

	// The top menu is created: its callback function is registered and menu entries,
	// including a submenu, added.
	glutCreateMenu(top_menu);
	glutAddSubMenu("Color for the bird", sub_menu01);
	glutAddMenuEntry("Quit", 1);

	// The menu is attached to a mouse button.
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//modified from mouseMotion.cpp
// Mouse callback routine.
void mouseControl(int button, int state, int x, int y)
{

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		isJump = !isJump;

	}
	glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
	float tempx = (float)x * 200 / width01;
	float tempy = ((float)height01 - y) * 200 / height01;
	cout << "X : " << tempx << " Y : " << tempy << endl;
	glutPostRedisplay();
}

void mousePassiveMotion(int x, int y)
{
	/*
	float tempx = (float)x * 200 / width01;
	float tempy = ((float)height01 - y) * 200 / height01;
	*/

	glutPostRedisplay();
}

void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 32:
		isJump = !isJump;
		break;
	case 'f':
	case 'F':
		isFog = !isFog;
		break;
	case 'n':
	case 'N':
		if (theta < 90.0) theta += 1.0;
		if (theta > 90.0) theta -= 1.0;
		break;
	case 'm':
	case 'M':
		if ((theta <= 90.0) && (theta > 0.0)) theta -= 1.0;
		if ((theta > 90.0) && (theta < 180.0))theta += 1.0;
		break;
	case 'q':
	case 'Q':
		timer_flag = !timer_flag;
		break;
	case 's':
	case 'S':
		yMove = 0;
		vertspeed = 0;
		score = 0;
		end_flag = false;
		timer_flag = true;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyInput(int key, int x, int y)
{

	float tempxVal = X, tempzVal = Z, tempAngle = angle;

	// Compute next position.
	if (key == GLUT_KEY_LEFT) tempAngle = angle + 5.0;
	if (key == GLUT_KEY_RIGHT) tempAngle = angle - 5.0;
	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT && key == GLUT_KEY_UP)
	{
		if (angleY >= PI / 2) {
			angleY = PI / 2;
		}
		else
		{
			angleY += PI / 180;
		}
	}
	else if (key == GLUT_KEY_UP)
	{
		tempxVal = X - sin(angle * PI / 180.0);
		tempzVal = Z - cos(angle * PI / 180.0);
	}
	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT && key == GLUT_KEY_DOWN)
	{
		if (angleY <= -PI / 2)
		{
			angleY = -PI / 2;
		}
		else
		{
			angleY -= PI / 180;
		}
	}
	else if (key == GLUT_KEY_DOWN)
	{
		tempxVal = X + sin(angle * PI / 180.0);
		tempzVal = Z + cos(angle * PI / 180.0);
	}

	// Angle correction.
	if (tempAngle > 360.0) tempAngle -= 360.0;
	if (tempAngle < 0.0) tempAngle += 360.0;

	X = tempxVal;
	Y = sin(angleY) * 10;
	Z = tempzVal;
	angle = tempAngle;

	glutPostRedisplay();
}



// Main routine.
int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	//First window
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	window01 = glutCreateWindow("3D Flappy Bird");
	setup01();
	glutDisplayFunc(drawScene01);
	glutReshapeFunc(resize01);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	glutMouseFunc(mouseControl);
	glutMotionFunc(mouseMotion);
	// Register the mouse motion callback function.
	glutPassiveMotionFunc(mousePassiveMotion);

	myinit();
	makeMenu(); // Create menu.
	Timer(1);


	glutInitWindowSize(600, 400);
	glutInitWindowPosition(750, 100);
	window02 = glutCreateWindow("Instruction");
	setup02();
	glutDisplayFunc(drawScene02);
	glutReshapeFunc(resize02);
	glutMainLoop();

	return 0;
}