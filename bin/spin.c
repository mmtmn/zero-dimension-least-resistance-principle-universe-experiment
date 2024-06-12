#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_DEPTH 3 // Adjusted for testing
#define NUM_POINTS 20 // Adjusted for testing

typedef struct {
    float x, y, z;
} Point3D;

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 10.0f;
float cameraYaw = 0.0f, cameraPitch = 0.0f;
float cameraSpeed = 0.1f;
int lastMouseX, lastMouseY;

int keys[256];

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
    lastMouseX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    lastMouseY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    memset(keys, 0, sizeof(keys));
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

    float lookX = sin(cameraYaw) * cos(cameraPitch);
    float lookY = sin(cameraPitch);
    float lookZ = -cos(cameraYaw) * cos(cameraPitch);

    gluLookAt(cameraX, cameraY, cameraZ,
              cameraX + lookX, cameraY + lookY, cameraZ + lookZ,
              0.0, 1.0, 0.0);

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

void keyboardDown(unsigned char key, int x, int y) {
    keys[key] = 1;
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = 0;
}

void updateCameraPosition() {
    float lookX = sin(cameraYaw) * cos(cameraPitch);
    float lookZ = -cos(cameraYaw) * cos(cameraPitch);

    if (keys['w']) {
        cameraX += lookX * cameraSpeed;
        cameraZ += lookZ * cameraSpeed;
    }
    if (keys['s']) {
        cameraX -= lookX * cameraSpeed;
        cameraZ -= lookZ * cameraSpeed;
    }
    if (keys['a']) {
        cameraX += lookZ * cameraSpeed;
        cameraZ -= lookX * cameraSpeed;
    }
    if (keys['d']) {
        cameraX -= lookZ * cameraSpeed;
        cameraZ += lookX * cameraSpeed;
    }
    glutPostRedisplay();
}

void mouseMovement(int x, int y) {
    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    lastMouseX = x;
    lastMouseY = y;

    cameraYaw += deltaX * 0.002f;
    cameraPitch -= deltaY * 0.002f;

    if (cameraPitch > M_PI_2 - 0.1f) cameraPitch = M_PI_2 - 0.1f;
    if (cameraPitch < -M_PI_2 + 0.1f) cameraPitch = -M_PI_2 + 0.1f;

    glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

void idle() {
    updateCameraPosition();
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
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMovement);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
