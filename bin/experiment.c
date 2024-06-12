#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_DEPTH 3 // Adjusted for testing
#define NUM_POINTS 50 // Adjusted for testing
#define GRAVITY_ZONE_RADIUS 5.0f
#define MAX_SPEED 0.0005f
#define STRONG_FORCE_CONSTANT 0.001f
#define SPEED_OF_LIGHT 299792458 // Speed of light in meters per second

typedef struct {
    float x, y, z;
    float vx, vy, vz; // Velocity components
    float mass; // Mass of the point
} Point3D;

typedef struct Node {
    Point3D point;
    struct Node* children[NUM_POINTS];
    int depth;
} Node;

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
float cameraYaw = 0.0f, cameraPitch = 0.0f;
float cameraSpeed = 0.1f;
int lastMouseX, lastMouseY;

int keys[256];

Node* createNode(Point3D point, int depth);
void generatePoints(Node* node);
void drawNode(Node* node);
void drawLine(Point3D p1, Point3D p2);
void updateNode(Node* node, Node* root);

Node* root;

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    srand(time(NULL)); // Initialize random seed only once
    Point3D start = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0}; // Initialize with mass = 1.0
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

        float mass = ((float)rand() / RAND_MAX) * 10.0f; // Random mass between 0 and 10

        Point3D newPoint = {x, y, z, 0.0f, 0.0f, 0.0f, mass};
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

void applyForces(Node* node, Node* other) {
    // Calculate distance between nodes
    float dx = other->point.x - node->point.x;
    float dy = other->point.y - node->point.y;
    float dz = other->point.z - node->point.z;
    float distance = sqrt(dx*dx + dy*dy + dz*dz);

    // Apply strong nuclear force if within gravity zone
    if (distance < GRAVITY_ZONE_RADIUS) {
        float force = STRONG_FORCE_CONSTANT / (distance * distance);
        node->point.vx += force * dx / distance;
        node->point.vy += force * dy / distance;
        node->point.vz += force * dz / distance;
    }
}

void updateVelocity(Node* node) {
    // Limit speed
    float speed = sqrt(node->point.vx * node->point.vx + node->point.vy * node->point.vy + node->point.vz * node->point.vz);
    if (speed > MAX_SPEED) {
        node->point.vx = (node->point.vx / speed) * MAX_SPEED;
        node->point.vy = (node->point.vy / speed) * MAX_SPEED;
        node->point.vz = (node->point.vz / speed) * MAX_SPEED;
    }

    // Update position
    node->point.x += node->point.vx;
    node->point.y += node->point.vy;
    node->point.z += node->point.vz;
}

float calculateEnergy(Point3D point) {
    // E = mc^2
    return point.mass * SPEED_OF_LIGHT * SPEED_OF_LIGHT;
}

void updateNode(Node* node, Node* root) {
    if (node->depth >= MAX_DEPTH) return;

    // Apply forces with respect to all other nodes in the structure
    #pragma omp parallel for
    for (int i = 0; i < NUM_POINTS; i++) {
        if (node->children[i] != NULL) {
            applyForces(node, root);
            updateVelocity(node->children[i]);
            updateNode(node->children[i], root);

            // Calculate and print energy for each node (for demonstration)
            float energy = calculateEnergy(node->children[i]->point);
            printf("Node at depth %d has energy: %e Joules\n", node->children[i]->depth, energy);
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
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.001, 100.0);
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
    updateNode(root, root);
    glutPostRedisplay();
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
