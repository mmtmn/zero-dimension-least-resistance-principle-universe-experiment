#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_DEPTH 3
#define NUM_POINTS 5 // Number of points to generate on the sphere
#define G 0.001f // Gravitational constant
#define TIME_STEP 0.1f // Time step for the simulation

typedef struct {
    float x, y, z;
    float vx, vy, vz; // Velocity components
} Point3D;

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 10.0f;
float cameraYaw = 0.0f, cameraPitch = 0.0f;
float speed = 0.1f;
Point3D* points = NULL; // Array of points
int pointIndex = 0; // Index to keep track of points

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

        // Initial velocity
        float vx = ((float)rand() / RAND_MAX - 0.5) * 0.1f;
        float vy = ((float)rand() / RAND_MAX - 0.5) * 0.1f;
        float vz = ((float)rand() / RAND_MAX - 0.5) * 0.1f;

        Point3D newPoint = {x, y, z, vx, vy, vz};
        points[pointIndex++] = newPoint;

        drawLine(point, newPoint);
        expand(newPoint, depth + 1);
    }
}

void updatePoints(Point3D* points, int numPoints) {
    #pragma omp parallel for
    for (int i = 0; i < numPoints; i++) {
        float fx = 0.0f;
        float fy = 0.0f;
        float fz = 0.0f;

        for (int j = 0; j < numPoints; j++) {
            if (i != j) {
                float dx = points[j].x - points[i].x;
                float dy = points[j].y - points[i].y;
                float dz = points[j].z - points[i].z;
                float distance = sqrt(dx * dx + dy * dy + dz * dz);
                if (distance > 0.01f) { // Avoid division by zero and very close distances
                    float force = (G / (distance * distance)) * (1.0f / distance);
                    fx += force * dx;
                    fy += force * dy;
                    fz += force * dz;
                }
            }
        }

        points[i].vx += fx * TIME_STEP;
        points[i].vy += fy * TIME_STEP;
        points[i].vz += fz * TIME_STEP;
    }

    for (int i = 0; i < numPoints; i++) {
        points[i].x += points[i].vx * TIME_STEP;
        points[i].y += points[i].vy * TIME_STEP;
        points[i].z += points[i].vz * TIME_STEP;
    }
}

void display(void) {
    static bool initialized = false;
    static int numPoints = NUM_POINTS * (int)pow(NUM_POINTS, MAX_DEPTH);

    if (!initialized) {
        points = (Point3D*)malloc(numPoints * sizeof(Point3D));
        Point3D start = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        expand(start, 0);
        initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Apply camera transformations
    gluLookAt(cameraX, cameraY, cameraZ, 
              cameraX + sin(cameraYaw * M_PI / 180.0), 
              cameraY + tan(cameraPitch * M_PI / 180.0), 
              cameraZ - cos(cameraYaw * M_PI / 180.0), 
              0.0, 1.0, 0.0);

    // Update points with gravity
    updatePoints(points, numPoints);

    // Draw points and lines
    for (int i = 0; i < numPoints; i++) {
        drawLine((Point3D){0.0, 0.0, 0.0}, points[i]); // Draw lines from the origin for simplicity
    }

    glutSwapBuffers();
    glutPostRedisplay(); // Ensure continuous updating
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            cameraX += speed * sin(cameraYaw * M_PI / 180.0);
            cameraZ -= speed * cos(cameraYaw * M_PI / 180.0);
            break;
        case 's':
            cameraX -= speed * sin(cameraYaw * M_PI / 180.0);
            cameraZ += speed * cos(cameraYaw * M_PI / 180.0);
            break;
        case 'a':
            cameraX -= speed * cos(cameraYaw * M_PI / 180.0);
            cameraZ -= speed * sin(cameraYaw * M_PI / 180.0);
            break;
        case 'd':
            cameraX += speed * cos(cameraYaw * M_PI / 180.0);
            cameraZ += speed * sin(cameraYaw * M_PI / 180.0);
            break;
        case 'q':
            cameraY -= speed;
            break;
        case 'e':
            cameraY += speed;
            break;
        case 27:
            exit(0);
    }
    glutPostRedisplay();
}

void mouseMotion(int x, int y) {
    static bool warp = false;
    if (warp) {
        warp = false;
        return;
    }

    int dx = x - 400;
    int dy = y - 300;

    cameraYaw += dx * 0.1f;
    cameraPitch -= dy * 0.1f;

    // Keep the pitch within limits
    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    // Warp pointer to the center of the window
    warp = true;
    glutWarpPointer(400, 300);

    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Expansion in 3D Space with Gravity");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouseMotion);
    glutWarpPointer(400, 300); // Initialize the mouse pointer to the center

    glutMainLoop();
    return 0;
}
