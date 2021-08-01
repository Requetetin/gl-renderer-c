#include <iostream>
#include <vector>
#include <cstring>
#include "glm/glm.hpp"
#define byte uint8_t
using namespace std;
using namespace glm;

vector < vec3 > vertex;
vector < vec2 > uvs;
vector < vec3 > normals;
vector < unsigned int > vertexIndex, uvIndex, normalIndex;
int width, height;
byte bmpfileheader[14] = {'B', 'M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
byte bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
byte framebuffer[768][1024][3];
int vpX, vpY, vpWidth, vpHeight;

int filesize;

// Colors
byte BLACK[3] = {0, 0, 0};
byte WHITE[3] = {255, 255, 255};
byte RED[3] = {255, 0, 0};
byte GREEN[3] = {0, 255, 0};
byte BLUE[3] = {0, 0, 255};
byte clearColor[3] = {0, 0, 0};
byte color[3] = {255, 255, 255};



void fillHeader()
{
    filesize = 54 + 3 * width * height;
    bmpfileheader[ 2] = (byte)(filesize    );
    bmpfileheader[ 3] = (byte)(filesize>> 8);
    bmpfileheader[ 4] = (byte)(filesize>>16);
    bmpfileheader[ 5] = (byte)(filesize>>24);

    bmpinfoheader[ 4] = (byte)(       width    );
    bmpinfoheader[ 5] = (byte)(       width>> 8);
    bmpinfoheader[ 6] = (byte)(       width>>16);
    bmpinfoheader[ 7] = (byte)(       width>>24);
    bmpinfoheader[ 8] = (byte)(       height    );
    bmpinfoheader[ 9] = (byte)(       height>> 8);
    bmpinfoheader[10] = (byte)(       height>>16);
    bmpinfoheader[11] = (byte)(       height>>24);
    bmpinfoheader[20] = (byte)(        3 * width * height    );
    bmpinfoheader[21] = (byte)(        3 * width * height>> 8    );
    bmpinfoheader[22] = (byte)(        3 * width * height>>16    );
    bmpinfoheader[23] = (byte)(        3 * width * height>>24    );
}

void glClear()
{
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            framebuffer[i][j][0] = clearColor[2];
            framebuffer[i][j][1] = clearColor[1];
            framebuffer[i][j][2] = clearColor[0];
        }
    }
}


/*
*
* glInit
*
*/
void glInit()
{
    width = 1024;
    height = 768;
    vpWidth = 1024;
    vpHeight = 768;
    vpX = 512;
    vpY = 384;
    fillHeader();
    glClear();
}


void glVertex(float x, float y){
    int posx = x * vpWidth/2 + vpX;
    int posy = y * vpHeight/2 + vpY;
    framebuffer[posy][posx][0] = color [2];
    framebuffer[posy][posx][1] = color [1];
    framebuffer[posy][posx][2] = color [0];
}

void glLine(float x0, float y0, float x1, float y1)
{
    float dy = y1 - y0;
    float dx = x1 - x0;
    float m = dy / dx;
    bool negative = m < 0;
    dy = abs(y1 - y0);
    dx = abs(x1 - x0);
    bool steep = dy > dx;
    if(steep){
        swap(x0, y0);
        swap(x1, y1);
        dy = abs(y1 - y0);
        dx = abs(x1 - x0);
    }
    m = dy / dx;

    if(!negative){
        if (x1 > x0){
            for(float x = x0; x < x1; x = x + 0.001){
                float y = y1 - m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }else{
            for(float x = x0; x > x1; x = x - 0.001){
                float y = y1 - m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }
    }else{
        if (x1 > x0){
            for(float x = x0; x < x1; x = x + 0.001){
                float y = y1 + m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }else{
            for(float x = x0; x > x1; x = x - 0.001){
                float y = y1 + m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }
    }
}



/*
*
* glFinish
* Escribe las variables al archivo
* 
*/
void glFinish()
{
    FILE* file;
    file = fopen("b.bmp", "wb");
    fwrite(bmpfileheader, 1, 14, file);
    fwrite(bmpinfoheader, 1, 40, file);
    for(int i = 0; i < height; i++){ 
        for(int j = 0; j < width; j++){
            fwrite(framebuffer[i][j], 1, 3, file);
        }
    }
    fclose(file);
}

void glClearColor(float r, float g, float b)
{
    clearColor[0] = r * 255;
    clearColor[1] = g * 255;
    clearColor[2] = b * 255;
}

void glColor(float r, float g, float b)
{
    color[0] = r * 255;
    color[1] = g * 255;
    color[2] = b * 255;
}

void glViewPort(int x, int y, int width, int height)
{
    vpX = x;
    vpY = y;
    vpWidth = width;
    vpHeight = height;
}

void glObj(const char * path)
{

    FILE* file = fopen(path, "r");

    while( 1 ){
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
        break;

        if (strcmp(lineHeader, "v") == 0){
            vec3 localvertex;
            fscanf(file, "%f %f %f\n", &localvertex.x, &localvertex.y, &localvertex.z);
            vertex.push_back(localvertex);
            //cout << vertex[0].x << endl;
        }else if (strcmp(lineHeader, "vt") == 0){
            vec2 localuv;
            fscanf(file, "%f %f\n", &localuv.x, &localuv.y);
            uvs.push_back(localuv);
            //cout << uvs[0].x << endl;
        }else if (strcmp(lineHeader, "vn") == 0){
            vec3 localnormal;
            fscanf(file, "%f %f %f\n", &localnormal.x, &localnormal.y, &localnormal.z);
            normals.push_back(localnormal);
            //cout << normals[0].x << endl;
        }else if (strcmp(lineHeader, "f") == 0){
            unsigned int vertexI[3], uvI[3], normalI[3];
            fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexI[0], &normalI[0], &vertexI[1], &normalI[1], &vertexI[2], &normalI[2] );
            vertexIndex.push_back(vertexI[0]);
            vertexIndex.push_back(vertexI[1]);
            vertexIndex.push_back(vertexI[2]);
            normalIndex.push_back(normalI[0]);
            normalIndex.push_back(normalI[1]);
            normalIndex.push_back(normalI[2]);

            
        }
    }

    for(int i=0; i<vertexIndex.size(); i = i+3){
        glLine(vertex[vertexIndex[i]].x/15, (vertex[vertexIndex[i]].y/15)-0.5, vertex[vertexIndex[i+1]].x/15, (vertex[vertexIndex[i+1]].y/15)-0.5);
        glLine(vertex[vertexIndex[i+1]].x/15, (vertex[vertexIndex[i+1]].y/15)-0.5, vertex[vertexIndex[i+2]].x/15, (vertex[vertexIndex[i+2]].y/15)-0.5);
        glLine(vertex[vertexIndex[i+3]].x/15, (vertex[vertexIndex[i+3]].y/15)-0.5, vertex[vertexIndex[i]].x/15, (vertex[vertexIndex[i]].y/15)-0.5);
    }
    
}


int main()
{
    glInit();
    glObj("Samus.obj");
    //cout << vertexIndex.size() << endl << vertex.size() << endl;
    glFinish();
    return 0;
}
