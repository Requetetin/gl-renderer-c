#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
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
float zbuffer[768][1024];
int vpX, vpY, vpWidth, vpHeight;
vec3 light = {0, 0, 1};

int filesize;

// Colors
byte BLACK[3] = {0, 0, 0};
byte WHITE[3] = {255, 255, 255};
byte RED[3] = {255, 0, 0};
byte GREEN[3] = {0, 255, 0};
byte BLUE[3] = {0, 0, 255};
byte clearColor[3] = {0, 0, 0};
byte color[3] = {255, 255, 255};


vec3 crossProd(vec3 v0, vec3 v1) {
    return {
        v0.y * v1.z - v0.z * v1.y,
        v0.z * v1.x - v0.x * v1.z,
        v0.x * v1.y - v0.y * v1.x
    };
}

float dotProd(vec3 v0, vec3 v1) {
    return v0.x * v1.x + v0.y + v1.y + v0.z * v1.z;
}

float vecLength(vec3 v) {
    return pow(v.x * v.x + v.y * v.y + v.z * v.z, 0.5);
}

vec3 norm(vec3 v) {
    float length = vecLength(v);

    if (!length) {
        return {0, 0, 0};
    }

    return {
        v.x/length, v.y/length, v.z/length
    };
}

vec3 doubleCross(vec3 A, vec3 B, vec3 C) {
    return crossProd(
        {B.x - A.x, B.y - A.y, B.z - A.z},
        {C.x - A.x, C.y - A.y, C.z - A.z}
    );
}

vec3 barycentric(vec2 A, vec2 B, vec2 C, vec2 P) {
    vec3 cross = crossProd(
        {
            B.x - A.x, C.x - A.x, A.x - P.x
        }, 
        {
            B.y - A.y, C.y - A.y, A.y - P.y
        }
    );
    //cout << cross.x << " " << cross.y << " " << cross.z << endl;

    if (abs(cross.z) < 1) {
        return {-1, -1, -1};
    }

    float u = cross.x / cross.z;
    float v = cross.y / cross.z;
    float w = 1 - (cross.x + cross.y) / cross.z;
    return {w, v, u};
}

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

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++){
            zbuffer[i][j] = -INFINITY;
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

void triangle(vec3 A, vec3 B, vec3 C) {
    vector <float> xs;
    vector <float> ys;
    xs.push_back(A.x);
    xs.push_back(B.x);
    xs.push_back(C.x);
    ys.push_back(A.y);
    ys.push_back(B.y);
    ys.push_back(C.y);

    if (A.x < 1) {
        A.x = A.x * vpWidth/2 + vpX;
        B.x = B.x * vpWidth/2 + vpX;
        C.x = C.x * vpWidth/2 + vpX;
        A.y = A.y * vpHeight/2 + vpY;
        B.y = B.y * vpHeight/2 + vpY;
        C.y = C.y * vpHeight/2 + vpY;
    }

    int xmin = *min_element(xs.begin(), xs.end()) * vpWidth/2 + vpX;
    int ymin = *min_element(ys.begin(), ys.end()) * vpHeight/2 + vpY;
    int xmax = *max_element(xs.begin(), xs.end()) * vpWidth/2 + vpX;
    int ymax = *max_element(ys.begin(), ys.end()) * vpHeight/2 + vpY;

    for (int i=xmin; i<xmax+1; i++) {
        for (int j=ymin; j<ymax+1; j++) {
            vec3 baryCoords = barycentric(A, B, C, {i, j});
            //cout << "bary " << baryCoords.x << " " << baryCoords.y << " " << baryCoords.z << endl;
            if (baryCoords.x < 0 || baryCoords.y < 0 || baryCoords.z < 0) {
                continue;
            }

            vec3 normal = norm(doubleCross(A, B, C));

            float intensity = dotProd(normal, light);

            if (intensity < -1) {
                intensity = -1;
            } else if (intensity > 1) {
                intensity = 1;
            }

            glColor(intensity, intensity, intensity);

            float z = A.z * baryCoords.x + B.z * baryCoords.y + C.z * baryCoords.z;
            if (z > zbuffer[i][j]) {
                glVertex(i, j);
                zbuffer[i][j] = z;
            }
        }
    }

}

void glObj(const char * path, float scaleX, float scaleY, float scaleZ, float translateX, float translateY, float translateZ)
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
        vertexTemp.z = vertex[i].z * scaleZ + translateZ;
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
        //cout << va.z << " " << vb.z << " " << vc.z << endl;
        triangle(va, vb, vc);
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
    vector <ivec2> shape1 = {ivec2(165, 380), ivec2(185, 360), ivec2(180, 330), ivec2(207, 345), ivec2(233, 330), ivec2(230, 360), ivec2(250, 380), ivec2(220, 385), ivec2(205, 410), ivec2(193, 383)};
    vector <ivec2> shape2 = {ivec2(321, 335), ivec2(288, 286), ivec2(339, 251), ivec2(374, 302)};
    vector <ivec2> shape3 = {ivec2(377, 249), ivec2(411, 197), ivec2(436, 249)};
    vector <ivec2> shape4 = {ivec2(413, 177), ivec2(448, 159), ivec2(502, 88), ivec2(553, 53), ivec2(535, 36), ivec2(676, 37), ivec2(660, 52),
                            ivec2(750, 145), ivec2(761, 179), ivec2(672, 192), ivec2(659, 214), ivec2(615, 214), ivec2(632, 230), ivec2(580, 230),
                            ivec2(597, 215), ivec2(552, 214), ivec2(517, 144), ivec2(466, 180)};
    vector <ivec2> shape5 = {ivec2(682, 175), ivec2(708, 120), ivec2(735, 148), ivec2(739, 170)};
    for(int i=0; i < shape1.size(); i++){
        glLine(shape1[i].x, shape1[i].y, shape1[(i+1)%10].x, shape1[(i+1)%10].y);
    }
    for(int i=0; i < shape2.size(); i++){
        glLine(shape2[i].x, shape2[i].y, shape2[(i+1)%4].x, shape2[(i+1)%4].y);
    }
    for(int i=0; i < shape3.size(); i++){
        glLine(shape3[i].x, shape3[i].y, shape3[(i+1)%3].x, shape3[(i+1)%3].y);
    }
    for(int i=0; i < shape4.size(); i++){
        glLine(shape4[i].x, shape4[i].y, shape4[(i+1)%18].x, shape4[(i+1)%18].y);
    }
    for(int i=0; i < shape5.size(); i++){
        glLine(shape5[i].x, shape5[i].y, shape5[(i+1)%4].x, shape5[(i+1)%4].y);
    }

}

void setLight(int x, int y, int z) {
    light.x = x;
    light.y = y;
    light.z = z;
}

int main()
{
    glInit();
    /* Shapes fill */
    
    /*drawShapes();
    glFill(200, 380);
    glFill(300, 300);
    glFill(411, 220);
    glFill(600, 200);*/
    
    /* Square viewport for a good view of the models */
    
    //glViewPort(512, 364, 768, 768);
    setLight(0, 0, 5);
    glObj("Samus.obj", 0.067, 0.067, 500, 0, -0.5, 0);
    
    glFinish();
    return 0;
}
