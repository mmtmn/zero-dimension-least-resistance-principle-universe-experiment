#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_DEPTH 3
#define NUM_POINTS 100 // Number of points to generate on the sphere

typedef struct {
    float x, y, z;
} Point3D;

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
}

void drawLine(Point3D p1, Point3D p2) {
    glBegin(GL_LINES);
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
}

void expand(Point3D point, int depth) {
    if (depth >= MAX_DEPTH) return;

    #pragma omp parallel for
    for (int i = 0; i < NUM_POINTS; i++) {
        float theta = ((float)rand() / RAND_MAX) * 2.0 * M_PI; // Angle around the Z-axis
        float phi = ((float)rand() / RAND_MAX) * M_PI;        // Angle from the Z-axis

        // Convert spherical coordinates to Cartesian coordinates
        float x = point.x + sin(phi) * cos(theta);
        float y = point.y + sin(phi) * sin(theta);
        float z = point.z + cos(phi);

        Point3D newPoint = {x, y, z};

        drawLine(point, newPoint);
        expand(newPoint, depth + 1);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(5.0, 5.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    Point3D start = {0.0, 0.0, 0.0};
    expand(start, 0);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Expansion in 3D Space");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
