#include <iostream>
#include <ctime>
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "GL/freeglut.h"	//These are for graphics
#include "GL/gl.h"
#include "GL/glu.h"

using namespace std;
bool run = true;
bool flag = false;
int screenWidth = 1024;
int screenHeight = 768;

memory* systemMemory = new memory;
cpu* core = new cpu();
ppu* video = new ppu;
int ppuCycles;							//Number of ppu cycles to run

void display();
void reshape_window(GLsizei w, GLsizei h);
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);
void specialUp(int key, int x, int y);
void specialDown(int key, int x, int y);
void setupTexture();

time_t startTime, endTime;
double seconds;

int main(int argc, char *args[])
{
	systemMemory->loadMemory();
	core->setPCStart(systemMemory);

	// Setup OpenGL
	glutInit(&argc, args);          
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(screenWidth, screenHeight);
    	glutInitWindowPosition(100, 100);
	glutCreateWindow("NES Emulator by Dartht33bagger");
	
	glutDisplayFunc(display);
	glutIdleFunc(display);
    	glutReshapeFunc(reshape_window);        

	//Controls
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp); 
	glutSpecialFunc(specialDown);
	glutSpecialUpFunc(specialUp);

	setupTexture();			

	glutMainLoop(); 


	delete systemMemory, core, video;	//Cleanup
	
	return 0;
}


// Setup Texture
void setupTexture()
{
	// Clear screen
	for(int y = 0; y < 224; ++y)		
		for(int x = 0; x < 256; ++x)
			video->screenData[y][x][0] = video->screenData[y][x][1] = video->screenData[y][x][2] = 0xF0;

	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 224, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)video->screenData);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	time(&startTime);
}

void updateTexture()
{	
	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, 256, 224, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)video->screenData);

	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,	0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d(screenWidth, 0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d(screenWidth, screenHeight);
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,	screenHeight);
	glEnd();
}


void display()
{
	if(run)
	{
	time(&endTime);
	seconds = difftime(endTime, startTime);

	if(seconds == 7) flag = true;

	if(flag)
	{
		systemMemory->dump();
		std::cout << "DONE" << std::endl;
		run = false;
	}
	
	ppuCycles = core->emulateCycle(systemMemory, video);

	//3 PPU cycles per CPU cycle
	for(int i = 0; i < (ppuCycles * 3); i++)
		video->emulateCycle(systemMemory);



	if(video->bufferVblank)	//Time to render a frame!
	{
		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT);
    
		updateTexture();

		// Swap buffers!
		glutSwapBuffers();    

		video->bufferVblank = false;
	}
	}
}

void reshape_window(GLsizei w, GLsizei h)
{
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
    	gluOrtho2D(0, w, h, 0);        
    	glMatrixMode(GL_MODELVIEW);
    	glViewport(0, 0, w, h);

	// Resize quad
	screenWidth = w;
	screenHeight = h;
}

void keyboardDown(unsigned char key, int x, int y)
{
	if(key == 27)    // esc
		exit(0);

	if(key == 'w') systemMemory->RAM[0x4016] |= 0x10;	//Up
	if(key == 'a') systemMemory->RAM[0x4016] |= 0x40;	//Down
	if(key == 's') systemMemory->RAM[0x4016] |= 0x20;	//Left
	if(key == 'd') systemMemory->RAM[0x4016] |= 0x80;	//Right
	if(key == 32) systemMemory->RAM[0x4016] |= 0x08;	//Enter key, start in NES
	if(key == 'l') systemMemory->RAM[0x4016] |= 0x40;	//Select
}

void keyboardUp(unsigned char key, int x, int y)
{
	if(key == 'w') systemMemory->RAM[0x4016] &= ~0x10;	//Up
	if(key == 'a') systemMemory->RAM[0x4016] &= ~0x40;	//Down
	if(key == 's') systemMemory->RAM[0x4016] &= ~0x20;	//Left
	if(key == 'd') systemMemory->RAM[0x4016] &= ~0x80;	//Right
	if(key == 32) systemMemory->RAM[0x4016] &= ~0x08;	//Enter key, start in NES
	if(key == 'l') systemMemory->RAM[0x4016] &= ~0x40;	//Select
}

void specialUp(int key, int x, int y)
{
	if(key == GLUT_KEY_UP) systemMemory->RAM[0x4016] |= 0x01; //Up arrow, A in NES
	if(key == GLUT_KEY_LEFT) systemMemory->RAM[0x4016] |= 0x02; //Left arrow, B in NES
}

void specialDown(int key, int x, int y)
{
	if(key == GLUT_KEY_UP) systemMemory->RAM[0x4016] &= ~0x01; //Up arrow, A in NES
	if(key == GLUT_KEY_LEFT) systemMemory->RAM[0x4016] &= ~0x02; //Left arrow, B in NES
}
