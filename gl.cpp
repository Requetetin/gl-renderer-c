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

void glVertex(int x, int y){
    framebuffer[y][x][0] = color[2];
    framebuffer[y][x][1] = color[1];
    framebuffer[y][x][2] = color[0];
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

void glLine(int x0, int y0, int x1, int y1)
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
            for(int x = x0; x < x1; x++){
                int y = y1 - m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }else{
            for(int x = x0; x > x1; x--){
                int y = y1 - m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }
    }else{
        if (x1 > x0){
            for(int x = x0; x < x1; x ++){
                int y = y1 + m * (x1 - x);
                if(steep){
                    glVertex(y, x);
                }else{
                    glVertex(x, y);
                }
            }
        }else{
            for(int x = x0; x > x1; x--){
                int y = y1 + m * (x1 - x);
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

void glObj(const char * path, float scaleX, float scaleY, float translateX, float translateY)
{

    FILE* file = fopen(path, "r");

    while( 1 ){
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
        break;

        if (strcmp(lineHeader, "v") == 0){
            //cout << "Vertice" << endl;
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
            //cout << "normal" << endl;
            vec3 localnormal;
            fscanf(file, "%f %f %f\n", &localnormal.x, &localnormal.y, &localnormal.z);
            normals.push_back(localnormal);
            //cout << normals[0].x << endl;
        }else if (strcmp(lineHeader, "f") == 0){
            //cout << "cara" << endl;
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

    vector < vec3 >  transformVertex;
    for(int i=0; i<vertex.size(); i++){
        vec3 vertexTemp;
        vertexTemp.x = vertex[i].x * scaleX + translateX;
        vertexTemp.y = vertex[i].y * scaleY + translateY;
        transformVertex.push_back(vertexTemp);
        //cout << vertexTemp.x << endl;
    }

    for(int i=0; i<vertexIndex.size(); i = i+3){
        int ia = vertexIndex[i] - 1;
        int ib = vertexIndex[i+1] - 1;
        int ic = vertexIndex[i+2] - 1;
        
        vec3 va = transformVertex[ia];
        vec3 vb = transformVertex[ib];
        vec3 vc = transformVertex[ic];
        //cout << va.x << " " << vb.x << " " << vc.x << endl;
        glLine(va.x, va.y, vb.x, vb.y);
        glLine(vb.x, vb.y, vc.x, vc.y);
        glLine(vc.x, vc.y, va.x, va.y);
    }
    
}

void glFill(int x, int y){
    if(framebuffer[y][x][0] == clearColor[2] 
    && framebuffer[y][x][1] == clearColor[1] 
    && framebuffer[y][x][2] == clearColor[0] 
    && (x < 1024 && x >-1 && y > -1 && y < 768)){
        //cout << "x: " << x << ", y: " << y << endl;
        glVertex(x, y);
        glFill(x, y+1);
        glFill(x, y-1);
        glFill(x+1, y);
        glFill(x-1, y);
    }
}

void drawShapes(){
    vector <vector <int>> shape1 = {{165, 380}, {185, 360}, {180, 330}, {207, 345}, {233, 330}, {230, 360}, {250, 380}, {220, 385}, {205, 410}, {193, 383}};
    vector <vector <int>> shape2 = {{321, 335}, {288, 286}, {339, 251}, {374, 302}};
    vector <vector <int>> shape3 = {{377, 249}, {411, 197}, {436, 249}};
    vector <vector <int>> shape4 = {{413, 177}, {448, 159}, {502, 88}, {553, 53}, {535, 36}, {676, 37}, {660, 52}, {750, 145}, {761, 179}, {672, 192}, {659, 214}, {615, 214}, {632, 230}, {580, 230}, {597, 215}, {552, 214}, {517, 144}, {466, 180}};
    vector <vector <int>> shape5 = {{682, 175}, {708, 120}, {735, 148}, {739, 170}};

    for(int i=0; i < shape1.size(); i++){
        glLine(shape1[i][0], shape1[i][1], shape1[(i+1)%10][0], shape1[(i+1)%10][1]);
    }
    for(int i=0; i < shape2.size(); i++){
        glLine(shape2[i][0], shape2[i][1], shape2[(i+1)%4][0], shape2[(i+1)%4][1]);
    }
    for(int i=0; i < shape3.size(); i++){
        glLine(shape3[i][0], shape3[i][1], shape3[(i+1)%3][0], shape3[(i+1)%3][1]);
    }
    for(int i=0; i < shape4.size(); i++){
        glLine(shape4[i][0], shape4[i][1], shape4[(i+1)%18][0], shape4[(i+1)%18][1]);
    }
    for(int i=0; i < shape5.size(); i++){
        glLine(shape5[i][0], shape5[i][1], shape5[(i+1)%4][0], shape5[(i+1)%4][1]);
    }

}


int main()
{
    glInit();
    /* Shapes fill */
    /*
    drawShapes();
    glFill(200, 380);
    glFill(300, 300);
    glFill(411, 220);
    glFill(600, 200);
    */
    /* Square viewport for a good view of the models */
    /*
    glViewPort(512, 364, 768, 768);
    glObj("Cube.obj", 0.5, 0.5, 0, 0);
    glObj("Samus.obj", 0.067, 0.067, 0, -0.5);
    */
    glFinish();
    return 0;
}
