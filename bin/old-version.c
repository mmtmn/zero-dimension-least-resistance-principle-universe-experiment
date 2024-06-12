#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Include this header for memset
#include <time.h>

#define MAX_DEPTH 3 // Adjusted for testing
#define NUM_POINTS 200 // Adjusted for testing

typedef struct {
    float x, y, z;
} Point3D;

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 10.0f;
float cameraYaw = 0.0f, cameraPitch = 0.0f;
float cameraSpeed = 0.1f;
int lastMouseX, lastMouseY;

int keys[256];

typedef struct Node {
    Point3D point;
    struct Node* children[NUM_POINTS];
    int depth;
} Node;

Node* createNode(Point3D point, int depth);
void generatePoints(Node* node);
void drawNode(Node* node);
void drawLine(Point3D p1, Point3D p2);

Node* root;

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    srand(time(NULL)); // Initialize random seed only once
    Point3D start = {0.0, 0.0, 0.0};
    root = createNode(start, 0);
    generatePoints(root);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
    lastMouseX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    lastMouseY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    memset(keys, 0, sizeof(keys));
}

Node* createNode(Point3D point, int depth) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->point = point;
    node->depth = depth;
    for (int i = 0; i < NUM_POINTS; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void generatePoints(Node* node) {
    if (node->depth >= MAX_DEPTH) return;

    for (int i = 0; i < NUM_POINTS; i++) {
        float theta = ((float)rand() / RAND_MAX) * 2.0 * M_PI; // Angle around the Z-axis
        float phi = ((float)rand() / RAND_MAX) * M_PI;        // Angle from the Z-axis

        // Convert spherical coordinates to Cartesian coordinates
        float x = node->point.x + sin(phi) * cos(theta);
        float y = node->point.y + sin(phi) * sin(theta);
        float z = node->point.z + cos(phi);

        Point3D newPoint = {x, y, z};
        node->children[i] = createNode(newPoint, node->depth + 1);
        generatePoints(node->children[i]);
    }
}

void drawLine(Point3D p1, Point3D p2) {
    glBegin(GL_LINES);
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
}

void drawNode(Node* node) {
    if (node->depth >= MAX_DEPTH) return;

    for (int i = 0; i < NUM_POINTS; i++) {
        if (node->children[i] != NULL) {
            drawLine(node->point, node->children[i]->point);
            drawNode(node->children[i]);
        }
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

    drawNode(root);

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
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
