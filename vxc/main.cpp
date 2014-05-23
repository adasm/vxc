#include "base.h"
#include "log.h"
#include "renderer.h"
#include "vm.h"
#include "editor.h"

int last = timeGetTime();

void display(void)
{
	int now = timeGetTime();
	int delta = now - last;
	last = now;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gR.drawQuad(gR.demoShader, vec4(1, 0, 0, 1), gR.texture);
	gE.draw(delta * 0.001f);

	glutSwapBuffers();
	glutPostRedisplay();
}

void keyPressed(unsigned char key, int x, int y)
{
	gE.keyPressed(key, x, y);
}

void keySpecial(int key, int x, int y)
{
	gE.keySpecial(key, x, y);
}

int main(int argc, char** argv)
{
	gR.init();
	gR.load();
	gVM.init();
	gE.init();	
	gE.loadFile();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(keySpecial);
	gR.loop();
	return 0;
}