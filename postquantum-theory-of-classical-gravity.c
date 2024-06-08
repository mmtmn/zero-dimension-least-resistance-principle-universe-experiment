// hypothesis of Jonathan Oppenheim
// code of mmtmn


#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define NUM_QUANTUM_SYSTEMS 1000
#define G 0.001f
#define TIME_STEP 0.01f
#define SPACETIME_FLUCTUATION_SCALE 1e-5f

typedef struct {
    float x, y, z;
    float vx, vy, vz;
    bool isQuantum;
} System;

float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 50.0f;
float cameraYaw = 0.0f, cameraPitch = 0.0f;
float speed = 0.1f;
System* systems = NULL;
int numSystems = 0;

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
}

void drawPoint(System s) {
    glPushMatrix();
    glTranslatef(s.x, s.y, s.z);
    if (s.isQuantum) {
        glColor3f(0.0, 1.0, 0.0); // Green for quantum systems
    } else {
        glColor3f(1.0, 0.0, 0.0); // Red for classical systems
    }
    glutSolidSphere(0.1, 10, 10);
    glPopMatrix();
}

void initializeSystems(void) {
    systems = (System*)malloc(NUM_QUANTUM_SYSTEMS * sizeof(System));
    for (int i = 0; i < NUM_QUANTUM_SYSTEMS; i++) {
        float x = ((float)rand() / RAND_MAX - 0.5) * 20.0f;
        float y = ((float)rand() / RAND_MAX - 0.5) * 20.0f;
        float z = ((float)rand() / RAND_MAX - 0.5) * 20.0f;

        float vx = ((float)rand() / RAND_MAX - 0.5) * 0.1f;
        float vy = ((float)rand() / RAND_MAX - 0.5) * 0.1f;
        float vz = ((float)rand() / RAND_MAX - 0.5) * 0.1f;

        bool isQuantum = (rand() % 2) == 0;

        System s = {x, y, z, vx, vy, vz, isQuantum};
        systems[numSystems++] = s;
    }
}

void applySpacetimeFluctuations(System* systems, int numSystems) {
    #pragma omp parallel for
    for (int i = 0; i < numSystems; i++) {
        if (systems[i].isQuantum) {
            systems[i].x += ((float)rand() / RAND_MAX - 0.5) * SPACETIME_FLUCTUATION_SCALE;
            systems[i].y += ((float)rand() / RAND_MAX - 0.5) * SPACETIME_FLUCTUATION_SCALE;
            systems[i].z += ((float)rand() / RAND_MAX - 0.5) * SPACETIME_FLUCTUATION_SCALE;
        }
    }
}

void updateSystems(System* systems, int numSystems) {
    #pragma omp parallel for
    for (int i = 0; i < numSystems; i++) {
        systems[i].x += systems[i].vx * TIME_STEP;
        systems[i].y += systems[i].vy * TIME_STEP;
        systems[i].z += systems[i].vz * TIME_STEP;
    }
}

void display(void) {
    static bool initialized = false;

    if (!initialized) {
        initializeSystems();
        initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(cameraX, cameraY, cameraZ, 
              cameraX + sin(cameraYaw * M_PI / 180.0), 
              cameraY + tan(cameraPitch * M_PI / 180.0), 
              cameraZ - cos(cameraYaw * M_PI / 180.0), 
              0.0, 1.0, 0.0);

    applySpacetimeFluctuations(systems, numSystems);
    updateSystems(systems, numSystems);

    for (int i = 0; i < numSystems; i++) {
        drawPoint(systems[i]);
    }

    glutSwapBuffers();
    glutPostRedisplay();
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

    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    warp = true;
    glutWarpPointer(400, 300);

    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Postquantum Theory of Classical Gravity");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouseMotion);
    glutWarpPointer(400, 300);

    glutMainLoop();
    return 0;
}

