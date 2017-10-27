#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

void initilizeWindow(void) {
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Simple Box");
};

void setDisplayMode(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

void draw(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 1.0, 1.0);
	glBegin(GL_POLYGON);
		glVertex3f(0.25, 0.25, 0.0);
		glVertex3f(0.75, 0.25, 0.0);
		glVertex3f(0.75, 0.75, 0.0);
		glVertex3f(0.25, 0.75, 0.0);
	glEnd();
	glFlush();
};

int main(int argc, char** argv)
{	
	glutInit(&argc, argv);
	initilizeWindow();
	setDisplayMode();
	glutDisplayFunc(draw);
	glutMainLoop();
	return 0;
}