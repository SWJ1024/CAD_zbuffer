#include "zbuffer.h"
#include <iostream>
#include <gl/glut.h>
#include <time.h>
#include <conio.h>

zbuffer z;
int winWidth = 1000, winHeight = 720;
extern int type;


void myInit(void) {
	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, winWidth, 0, winHeight);
}

void displayFcn() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS);
	for (int i = 0; i < winWidth; i++) {
		for (int j = 0; j < winHeight; j++) {
			if (z.buffer[j][i] != -1) {
				int id = z.buffer[j][i];
				glColor3f(z.F[id]._color[0], z.F[id]._color[1], z.F[id]._color[2]);
				glVertex2f(i, j);
			}
		}
	}
	glEnd();
	glFlush();
}

int main(int argc, char* argv[]) {
	z.init(winHeight, winWidth);
	z.CreatPolygonTableAndEdgeTable();

	clock_t start = clock();
	std::cout << "开始时间：" << start << std::endl;

	//z.sacnline();
	z.zufferscanline();

	clock_t end = clock();
	std::cout << "结束时间：" << end << std::endl << "算法执行持续时间：" << end - start << "毫秒" << std::endl;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("zbuffer");
	myInit();
	glutDisplayFunc(displayFcn);
	glutMainLoop();
	return 0;
}
