#if _MSC_VER
#define _HAS_STD_BYTE 0
#include <Windows.h>
#endif

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include "renderer/clientcontroller.h"
#include "network/client/implementation/client.h"
#include "omnidirectional_layer/implementation/BlockDecoder.h"
#include <fstream>

#include <GL/gl.h>
#include <GL/glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;

static ClientController *s_Controller = NULL;
static unsigned char *s_data;

using namespace std;

GLuint glInitTexture(char *filename)
{
    GLuint t = 0;

    int textureWidth = 2048;
    int textureHeight = 2048;
    int nrChannels = 0;

    const int rowPitch = textureWidth * 4;
    // Just allocate a system memory buffer here for simplicity

    //unsigned char *data = stbi_load(fs::path("/home/sbelleno/_ICOV/CODE/adt.git/build/bin/P29_8888x4444.jpg").c_str(), &textureWidth, &textureHeight, &nrChannels, 0);

    unsigned char *data = new unsigned char[rowPitch * textureHeight];

    s_data = data;

    s_Controller->getClient().getDecoder().setTexture(data, textureWidth, textureHeight, rowPitch);

    cout << "loaded: " << textureWidth << "*" << textureHeight << std::endl;

    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    s_Controller->update();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_data);
    return t;
}

// GLuint glInitTexture(char *filename)
// {
//     GLuint t = 0;

//     int textureWidth = 32;
//     int textureHeight = 32;
//     int nrChannels = 0;

//     const int rowPitch = textureWidth * 1;

//     int pixel_count = rowPitch * textureHeight;

//     // Just allocate a system memory buffer here for simplicity

//     //unsigned char *data = stbi_load(fs::path("/home/sbelleno/_ICOV/CODE/adt.git/build/bin/P29_8888x4444.jpg").c_str(), &textureWidth, &textureHeight, &nrChannels, 0);

//     char* data = new char[pixel_count];

//     cout << "read file" << endl;

//     ifstream fin;
//     fin.open("test.icov", ios::binary | ios::in);
//     fin.read(data, pixel_count);
//     fin.close();

//     s_data = (unsigned char *)data;

//     cout<<"loaded: " << textureWidth << "*" << textureHeight << std::endl;

//     glGenTextures(1, &t);
//     glBindTexture(GL_TEXTURE_2D, t);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight, 0,  GL_RED, GL_UNSIGNED_BYTE, s_data);
//     return t;
// }

void drawImage(GLuint file,
               float x,
               float y,
               float w,
               float h,
               float angle)
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBindTexture(GL_TEXTURE_2D, file);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_data);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 32, 32, 0, GL_RED, GL_UNSIGNED_BYTE, s_data);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(1.0, -1.0, 0.0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

GLuint texture;
void render()
{
    s_Controller->update();

    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    const double w = glutGet(GLUT_WINDOW_WIDTH);
    const double h = glutGet(GLUT_WINDOW_HEIGHT);

    drawImage(texture, -1.0f, 0.0f, 8.0f, 4.0f, 0.0);
    glFlush();
}

int main(int argc, char *argv[])
{
    s_Controller = new ClientController();
    s_Controller->start();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(600, 600);
    glutCreateWindow("ICOV TEST");
    glutDisplayFunc(render);
    glutIdleFunc(glutPostRedisplay);

    texture = glInitTexture("decoder_icov");

    glutMainLoop();

    return 0;
}

// void displayMe(void)
// {
//     glClear(GL_COLOR_BUFFER_BIT);

//     glEnable(GL_TEXTURE_2D);

//     glBegin(GL_POLYGON);
//         glVertex3f(-1.0, -1.0, 0.0);
//         glVertex3f(1.0, -1.0, 0.0);
//         glVertex3f(1.0, 1.0, 0.0);
//         glVertex3f(-1.0, 1.0, 0.0);
//     glEnd();
//     glDisable(GL_TEXTURE_2D);
//     glFlush();
// }

// int main(int argc, char** argv)
// {
//     glutInit(&argc, argv);

//     glutInitDisplayMode(GLUT_RGBA);
//     glutInitWindowSize(300, 300);
//     glutInitWindowPosition(100, 100);
//     glutCreateWindow("ICOV test");
//     glutDisplayFunc(displayMe);

//     glutInitWindowPosition(300, 100);
//     glutCreateWindow("win2");   // Create a window 2
//     glutDisplayFunc(displayMe);   // Register display callback

//     glutMainLoop();
//     return 0;
// }
