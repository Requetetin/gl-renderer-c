#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#define byte uint8_t
using namespace std;
using namespace glm;

vector < vec3 > vertex;
vector < vec2 > uvs;
vector < vec3 > normals;
vector < unsigned int > vertexIndex, uvIndex, normalIndex;
mat4x4 model_mat;
mat4x4 view_mat;
mat4x4 projection_mat;
mat4x4 viewport_mat;
mat4x4 final_mat;
int width, height;
byte bmpfileheader[14] = {'B', 'M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
byte bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
byte framebuffer[768][1024][3];
float zbuffer[768][1024];
int vpX, vpY, vpWidth, vpHeight;
vec3 light = {0, 0, 1};
int textWidth, textHeight;
int normWidth, normHeight;
vector < vec3 > loadedTexture;
vector < vec3 > loadedNormal;
vec3 vertexTemp, va, vb, vc, na, nb, nc;
vec2 ta, tb, tc;
bool hasNormal = false;


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
    //cout << to_string(v0) << " and " << to_string(v1) << endl;
    float x = v0.y * v1.z - v0.z * v1.y;
    float y = v0.z * v1.x - v0.x * v1.z;
    float z = v0.x * v1.y - v0.y * v1.x;
    //cout << x << " " << y << " " << z << endl;
    return {
        x,
        y,
        z
    };
}

float dotProd(vec3 v0, vec3 v1) {
    //cout << to_string(v0) << " and " << to_string(v1) << endl;
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

void glClearColor(int r, int g, int b) {
    clearColor[0] = r;
    clearColor[1] = g;
    clearColor[2] = b;
}

void glColor(float r, float g, float b)
{
    color[0] = r * 255;
    color[1] = g * 255;
    color[2] = b * 255;
}

void glColor(int r, int g, int b)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
}

void glViewPort(int x, int y, int width, int height)
{
    vpX = x;
    vpY = y;
    vpWidth = width;
    vpHeight = height;
}

int circle(int x, int h, int y, int k) {
    return pow(pow(x-h, 2) + pow(y-k, 2), 0.5);
}

void shaderLines(float intensity) {
    int r = 226 * intensity;
    int g = 111 * intensity;
    int b = 62 * intensity;
    glColor(r, g, b);
    glLine(450, 485, 478, 392);
    glLine(477, 396, 582, 423);
    glLine(590, 401, 602, 365);
    glLine(613, 365, 623, 424);
    glLine(483, 394, 546, 364);
    glLine(546, 364, 502, 344);
    glLine(629, 439, 595, 432);
    glLine(585, 438, 588, 473);
    glLine(568, 430, 555, 436);
    glLine(540, 445, 570, 432);
    glLine(550, 326, 571, 316);
    glLine(536, 324, 490, 317);
    glLine(578, 311, 574, 304);
    glLine(566, 299, 542, 286);
    glLine(537, 284, 518, 284);
    glLine(501, 334, 510, 303);
    glLine(512, 306, 516, 258);
    glLine(491, 273, 515, 266);
    glLine(604, 391, 533, 371);
    glLine(573, 298, 591, 284);
    glLine(524, 459, 538, 425);
    glLine(399, 361, 356, 367);
}

void shaderClouds(int x, int y, float intensity) {
    //cout << sin(x) + 350 << endl;
    if (x < 10 * sin(0.07 * y) + 380 + rand() % 50) {
        int color = rand() % 10;
        if (color == 0) {
            int r = 42 * intensity;
            int g = 71 * intensity;
            int b = 70 * intensity;
            glColor(r, g, b);
        } else if (color == 1) {
            int r = 66 * intensity;
            int g = 50 * intensity;
            int b = 87 * intensity;
            glColor(r, g, b);
        } else if (color > 1 && color < 8) {
            return;
        } else {
            int r = 177 * intensity;
            int g = 146 * intensity;
            int b = 190 * intensity;
            glColor(r, g, b);
        }
    } else if (x > 8 * sin(0.08 * y) + 690 - rand() % 50) {
        int color2 = rand() % 10;
        if (color2 == 0) {
            int r = 220 * intensity;
            int g = 201 * intensity;
            int b = 217 * intensity;
            glColor(r, g, b);
        } else if (color2 == 1) {
            int r = 89 * intensity;
            int g = 70 * intensity;
            int b = 121 * intensity;
            glColor(r, g, b);
        } else if (color2 > 1 && color2 < 8) {
            return;
        } else {
            int r = 154 * intensity;
            int g = 110 * intensity;
            int b = 138 * intensity;
            glColor(r, g, b);
        }
    } else if (y < 5 * sin(0.2 * x) + 250 - rand() % 60 && x > 530 - rand() % 50) {
        int color3 = rand() % 10;
        if (color3 == 0) {
            int r = 220 * intensity;
            int g = 201 * intensity;
            int b = 217 * intensity;
            glColor(r, g, b);
        } else if (color3 == 1) {
            int r = 89 * intensity;
            int g = 70 * intensity;
            int b = 121 * intensity;
            glColor(r, g, b);
        } else if (color3 > 1 && color3 < 8) {
            return;
        } else {
            int r = 154 * intensity;
            int g = 110 * intensity;
            int b = 138 * intensity;
            glColor(r, g, b);
        }
    } else if (y > 6 * sin(0.06 * x) + 530 + rand() % 20) {
        int color4 = rand() % 10;
        if (color4 == 0) {
            int r = 220 * intensity;
            int g = 201 * intensity;
            int b = 217 * intensity;
            glColor(r, g, b);
        } else if (color4 == 1) {
            int r = 89 * intensity;
            int g = 70 * intensity;
            int b = 121 * intensity;
            glColor(r, g, b);
        } else if (color4 > 1 && color4 < 8) {
            return;
        } else {
            int r = 154 * intensity;
            int g = 110 * intensity;
            int b = 138 * intensity;
            glColor(r, g, b);
        }
    }
}

void shader(int x, int y, float intensity) {
    /* Rango:
        x: 324 - 708
        y: 179 - 563
       Diametro:
        384px
       Radio:
        192px 
       Centro:
        516, 371
    */
   //cout << x << " " << y << endl;
   int circle1 = circle(x, 477, y, 398);
   int circle2 = circle(x, 552, y, 362);
   int circle3 = circle(x, 499, y, 340);
   int circle4 = circle(x, 542, y, 330);
   int circle5 = circle(x, 607, y, 343);
   int circle6 = circle(x, 582, y, 423);
   int circle7 = circle(x, 519, y, 472);
   int circle8 = circle(x, 449, y, 486);
   int circle9 = circle(x, 475, y, 313);
   int circle10 = circle(x, 542, y, 329);
   int circle11 = circle(x, 407, y, 359);
   int circle12 = circle(x, 480, y, 278);
   int circle13 = circle(x, 584, y, 323);
   int circle14 = circle(x, 576, y, 314);
   int circle15 = circle(x, 437, y, 413);
   int circle16 = circle(x, 426, y, 379);
   int circle17 = circle(x, 570, y, 302);
   int circle18 = circle(x, 539, y, 285);
   int circle19 = circle(x, 592, y, 284);
   int circle20 = circle(x, 604, y, 276);
   int circle21 = circle(x, 628, y, 443);
   int circle22 = circle(x, 587, y, 459);
   int circle23 = circle(x, 588, y, 477);
   shaderLines(intensity);
   int r = 226 * intensity;
   int g = 111 * intensity;
   int b = 62 * intensity;
   if (circle1 == 8 || circle1 == 14 || circle1 == 26) {
       glColor(r, g, b);
   } else if (circle2 == 2 || circle2 == 6) {
       glColor(r, g, b);
   } else if (circle3 < 4 || circle3 == 7) {
       glColor(r, g, b);
   } else if (circle4 == 1 || circle4 == 4 || circle4 == 7) {
       glColor(r, g, b);
   } else if (circle5 == 2 || circle5 == 5 || circle5 == 7 || (circle5 > 10 && circle5 < 14) || circle5 == 21) {
       glColor(r, g, b);
   } else if (circle6 == 2 || circle6 == 6 || circle6 == 8 || circle6 == 11 || circle6 == 13 || circle6 == 22) {
       glColor(r, g, b);
   } else if (circle7 == 6 || circle7 == 12 || circle7 == 23) {
       glColor(r, g, b);
   } else if ((circle8 > 2 && circle8 < 6) || circle8 == 10) {
       glColor(r, g, b);
   } else if (circle9 == 2 || (circle9 > 3 && circle9 < 7) || circle9 == 8 || circle9 == 13 || circle9 == 15) {
       glColor(r, g, b);
   } else if (circle10 == 1 || circle10 == 3 || circle10 == 4 || circle10 == 7) {
       glColor(r, g, b);
   } else if (circle11 == 1 || circle11 == 2 || circle11 == 4 || (circle11 > 7 && circle11 < 11)) {
       glColor(r, g, b);
   } else if (circle12 < 7 || circle12 == 8 || circle12 == 11) {
       glColor(r, g, b);
   } else if (circle13 == 2 || circle13 == 4 || circle13 == 7) {
       glColor(r, g, b);
   } else if (circle14 == 2 || circle14 == 4) {
       glColor(r, g, b);
   } else if (circle15 == 3 || circle15 == 6) {
       glColor(r, g, b);
   } else if (circle16 < 3) {
       glColor(r, g, b);
   } else if (circle17 < 3 || circle17 == 5) {
       glColor(r, g, b);
   } else if (circle18 > 0 && circle18 < 4) {
       glColor(r, g, b);
   } else if (circle19 == 1 || circle19 == 3 || circle19 == 5 || circle19 == 9) {
       glColor(r, g, b);
   } else if (circle20 == 1 || circle20 == 3 || circle20 == 5) {
       glColor(r, g, b);
   } else if (circle21 == 5 || circle21 == 11 || circle21 == 19) {
       glColor(r, g, b);
   } else if (circle22 == 5) {
       glColor(r, g, b);
   } else if (circle23 == 5 || circle23 == 11 || circle23 == 18) {
       glColor(r, g, b);
   } else {
        int background = rand() % 10;
        int r1 = 55 * intensity;
        int g1 = 15 * intensity;
        int b1 = 19 * intensity;
        int r2 = 29 * intensity;
        int g2 = 15 * intensity;
        int b2 = 6 * intensity;
        int r3 = 66 * intensity;
        int g3 = 29 * intensity;
        int b3 = 36 * intensity;
        if (background != 0) {
            int thisRand = rand() % 2;
            if (thisRand == 0) {
                glColor(r1, g1, b1);
            } else {
                glColor(r2, g2, b2);
            }   
        } else {
            glColor(r3, g3, b3);
        }
   }
   shaderClouds(x, y, intensity);
}

void triangle(vec3 A, vec3 B, vec3 C, vec2 tA, vec2 tB, vec2 tC) {
    vector <float> xs;
    vector <float> ys;
    xs.push_back(A.x);
    xs.push_back(B.x);
    xs.push_back(C.x);
    ys.push_back(A.y);
    ys.push_back(B.y);
    ys.push_back(C.y);

    //cout << to_string(A) << " " << to_string(B) << " " << to_string(C) << " " << endl;  
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

    //cout << xmin << " " << ymin << " " << xmax << " " << ymax << endl;
    vec3 baryCoords;
    //cout << to_string(B - A) << " " << to_string(C - A) << endl;
    vec3 normal = norm(crossProd((B - A), (C - A)));
    //cout << to_string(normal) << endl;
    float intensity = dotProd(normal, light);
    float w, v, u, z, tx, ty, nx, ny, nz;
    int x, y, textPos, r, g, b;
    for (int i=xmin; i<xmax+1; i++) {
        for (int j=ymin; j<ymax+1; j++) {
            //cout << to_string(A) << " " << to_string(B) << " " << to_string(C) << " " << endl;
            baryCoords = barycentric(A, B, C, {i, j});
            w = baryCoords.x;
            v = baryCoords.y;
            u = baryCoords.z;
            //cout << "bary " << baryCoords.x << " " << baryCoords.y << " " << baryCoords.z << endl;
            if (w < 0 || v < 0 || u < 0) {
                continue;
            }
            if (intensity < 0) {
                intensity = 0;
            } else if (intensity > 1) {
                intensity = 1;
            }
            //cout << intensity << endl;


            tx = tA.x * w + tB.x * u + tC.x * v;
            ty = tA.y * w + tB.y * u + tC.y * v;
            
            x = tx * textWidth;
            y = ty * textHeight;
            //cout << x << " " << y << endl;

            textPos = y * textWidth + x;
            //cout << textPos << endl;
            //cout << loadedTexture[textPos].x << " " << loadedTexture[textPos].y << " " << loadedTexture[textPos].z << endl;

            r = loadedTexture[textPos].x * intensity;
            g = loadedTexture[textPos].y * intensity;
            b = loadedTexture[textPos].z * intensity;


            if (hasNormal) {
                nx = na.x * w + nb.x * u + nc.x * v;
                ny = na.y * w + nb.y * u + nc.y * v;
                nz = na.z * w + nb.z * u + nc.z * v;
                r *= (nx * 0.5 + 0.5);
                g *= (ny * 0.5 + 0.5);
                b *= (nz * 0.5 + 0.5);

                //cout << r << " " << g << " " << b << endl;
            }

            //cout << r << " " << g << " " << b << endl;

            glColor(r, g, b);



            z = A.z * w + B.z * v + C.z * u;
            if (i > 1023 || j > 767 || i < 0 || j < 0) {
                //cout << "hi" << endl;
                continue;
            }
            if (z > zbuffer[i][j]) {
                if (i < 0 || j < 0 || i > 1023 || j > 768) {
                    continue;
                }
                //cout << i << " " << j << " " << intensity << endl;
                glVertex(i, j);
                zbuffer[i][j] = z;
            }
        }
    }
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

    //cout << xmin << " " << ymin << " " << xmax << " " << ymax << endl;
    vec3 baryCoords;
    vec3 normal = norm(crossProd((B - A), (C - A)));
    float intensity, z;
    intensity = dotProd(normal, light);
    for (int i=xmin; i<xmax+1; i++) {
        for (int j=ymin; j<ymax+1; j++) {
            baryCoords = barycentric(A, B, C, {i, j});
            //cout << "bary " << baryCoords.x << " " << baryCoords.y << " " << baryCoords.z << endl;
            if (baryCoords.x < 0 || baryCoords.y < 0 || baryCoords.z < 0) {
                continue;
            }

            if (intensity < 0) {
                intensity = 0;
            } else if (intensity > 1) {
                intensity = 1;
            }
            //cout << intensity << endl;

            glColor(intensity, intensity, intensity);

            z = A.z * baryCoords.x + B.z * baryCoords.y + C.z * baryCoords.z;
            if (z > zbuffer[i][j]) {
                //cout << i << " " << j << " " << intensity << endl;
                //shader(i, j, intensity);
                if (i < 0 || j < 0 || i > 1023 || j > 768) {
                    continue;
                }
                glVertex(i, j);
                zbuffer[i][j] = z;
            }
        }
    }

}

vec3 transform(vec3 vertex) {
    //cout << vertex.x << endl;
    vec4 tempVec = {vertex.x, vertex.y, vertex.z, 1};
    //cout << to_string(tempVec) << endl;
    //cout << "viewport " << to_string(viewport_mat) << endl << "projection " << to_string(projection_mat) << endl << "view " << to_string(view_mat) << endl << "model " << to_string(model_mat) << endl;
    //cout << to_string(viewport_mat) << endl;
    
    final_mat = projection_mat * view_mat * model_mat;
    //cout << to_string(final_mat) << endl;

    vec4 transformedVertex = final_mat * tempVec;
    //vec4 transformedVertex = model_mat * view_mat * projection_mat * viewport_mat * tempVec;
    
    //cout << endl << to_string(projection_mat * viewport_mat) << endl;
    //cout << endl << to_string(projection_mat * view_mat * model_mat) << endl;
    //cout << endl << to_string(viewport_mat * projection_mat * view_mat * model_mat) << endl;
    //cout << endl << to_string(viewport_mat * projection_mat * view_mat * model_mat * tempVec) << endl;
    //cout << to_string(transformedVertex) << endl;
    float x = transformedVertex[0] / transformedVertex[3];
    float y = transformedVertex[1] / transformedVertex[3];
    float z = transformedVertex[2] / transformedVertex[3];
    //cout << x << " y: " << y << " z: " << z << endl;
    vec3 finalVertex = {
        x,
        y,
        z
    };

    //cout << to_string(finalVertex) << endl;

    return finalVertex;
}

void loadModelMatrix(vec3 translation, vec3 scaling, vec3 rotation) {
    mat4 translation_mat, xrotation_mat, yrotation_mat, zrotation_mat, rotation_mat, scale_mat;
    translation_mat = mat4(
        1, 0, 0, translation.x,
        0, 1, 0, translation.y,
        0, 0, 1, translation.z,
        0, 0, 0, 1
    );

    float a = rotation.x;
    xrotation_mat = mat4(
        1, 0, 0, 0,
        0, cos(a), -sin(a), 0,
        0, sin(a), cos(a), 0,
        0, 0, 0, 1
    );

    a = rotation.y;
    yrotation_mat = mat4(
        cos(a), 0, sin(a), 0,
        0, 1, 0, 0,
        -sin(a), 0, cos(a), 0,
        0, 0, 0, 1
    );

    a = rotation.z;
    zrotation_mat = mat4(
        cos(a), -sin(a), 0, 0,
        sin(a), cos(a), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    rotation_mat = xrotation_mat * yrotation_mat * zrotation_mat;

    scale_mat = mat4(
        scaling.x, 0, 0, 0,
        0, scaling.y, 0, 0,
        0, 0, scaling.z, 0,
        0, 0, 0, 1
    );

    model_mat = translation_mat * rotation_mat * scale_mat;
}

void loadProjectionMatrix(float coeff) {
    projection_mat = mat4(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, coeff, 1  
    );
}

void loadViewportMatrix(int x, int y) {
    viewport_mat = mat4(
      width/2, 0, 0, x + width/2,
      0, height/2, 0, y + height/2,
      0, 0, 128, 128,
      0, 0, 0, 1  
    );

    //cout << to_string(viewport_mat) << endl;
}

void loadViewMatrix(vec3 x, vec3 y, vec3 z, vec3 center) {
    mat4x4 m = mat4 (
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        0, 0, 0, 1
    );

    //cout << to_string(m) << endl;

    mat4x4 o = mat4 (
        1, 0, 0, -center.x,
        0, 1, 0, -center.y,
        0, 0, 1, -center.z,
        0, 0, 0, 1
    );

    //cout << to_string(o) << endl;

    view_mat = m * o;

    //cout << to_string(view_mat) << endl;
}

void lookAt(vec3 eye, vec3 center, vec3 up) {
    //cout << to_string(eye) << " " << to_string(center) << " " << to_string(eye - center);
    vec3 z = norm(eye - center);
    //cout << to_string(z) << endl;

    vec3 x = norm(crossProd(up, z));
    //cout << to_string(x) << endl;

    vec3 y = norm(crossProd(z, x));
    //cout << to_string(y) << endl;

    glm::mat4 CameraMatrix = glm::lookAt(
        eye, // the position of your camera, in world space
        center,   // where you want to look at, in world space
        up        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
    );
    //cout << "glm " << to_string(CameraMatrix) << endl;

    loadViewMatrix(x, y, z, center);
    loadProjectionMatrix(-1/length(eye - center));
    loadViewportMatrix(0, 0);
}

void glObj(const char * path, int type, vec3 translate, vec3 scale, vec3 rotation)
{

    FILE* file = fopen(path, "r");

    vec3 localvertex, localnormal;
    vec2 localuv;
    int vertexI[3], uvI[3], normalI[3];
    while( 1 ){
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
        break;
        if (strcmp(lineHeader, "v") == 0){
            //cout << "Vertice" << endl;
            fscanf(file, "%f %f %f %f\n", &localvertex.x, &localvertex.y, &localvertex.z);
            vertex.push_back(localvertex);
            //cout << vertex[0].x << endl;
        }else if (strcmp(lineHeader, "vt") == 0){
            fscanf(file, "%f %f\n", &localuv.x, &localuv.y);
            uvs.push_back(localuv);
            //cout << uvs[0].x << endl;
        }else if (strcmp(lineHeader, "vn") == 0){
            //cout << "normal" << endl;
            fscanf(file, "%f %f %f\n", &localnormal.x, &localnormal.y, &localnormal.z);
            normals.push_back(localnormal);
            //cout << normals[0].x << endl;
        }else if (strcmp(lineHeader, "f") == 0){
            //cout << "cara" << endl;
            if (type == 1) {
                fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexI[0], &uvI[0], &normalI[0], &vertexI[1], &uvI[1], &normalI[1], &vertexI[2], &uvI[2], &normalI[2] );
                vertexIndex.push_back(vertexI[0]);
                vertexIndex.push_back(vertexI[1]);
                vertexIndex.push_back(vertexI[2]);
                uvIndex.push_back(uvI[0]);
                uvIndex.push_back(uvI[1]);
                uvIndex.push_back(uvI[2]);
                normalIndex.push_back(normalI[0]);
                normalIndex.push_back(normalI[1]);
                normalIndex.push_back(normalI[2]);
            } else {
                fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexI[0], &normalI[0], &vertexI[1], &normalI[1], &vertexI[2], &normalI[2] );
                vertexIndex.push_back(vertexI[0]);
                vertexIndex.push_back(vertexI[1]);
                vertexIndex.push_back(vertexI[2]);
                normalIndex.push_back(normalI[0]);
                normalIndex.push_back(normalI[1]);
                normalIndex.push_back(normalI[2]);
            }

            
        }
    }

    fclose(file);
    
    loadModelMatrix(translate, scale, rotation);

    vector < vec3 >  transformVertex;
    int ia, ib, ic, ja, jb, jc, ka, kb, kc;
    //cout << vertex.size();
    for(int i=0; i<vertex.size(); i++){
        //cout << "Pre " << to_string(vertex[i]) << endl;
        vertexTemp = transform(vertex[i]);
        //cout << to_string(vertexTemp) << endl;
        transformVertex.push_back(vertexTemp);
        // cout << i << endl;
    }
    if (!loadedTexture.size()) {
        for(int i=0; i<vertexIndex.size(); i = i+3){
            //cout << i << endl;
            ia = vertexIndex[i] - 1;
            ib = vertexIndex[i+1] - 1;
            ic = vertexIndex[i+2] - 1;
            
            va = transformVertex[ia];
            vb = transformVertex[ib];
            vc = transformVertex[ic];
            triangle(va, vb, vc);
        }
    } else {
        //cout << vertexIndex.size();
        for(int i=0; i<vertexIndex.size(); i = i+3){
            //cout << i << endl;
            ia = vertexIndex[i] - 1;
            ib = vertexIndex[i+1] - 1;
            ic = vertexIndex[i+2] - 1;
            
            va = transformVertex[ia];
            vb = transformVertex[ib];
            vc = transformVertex[ic];

            //cout << vertexIndex[i] -1 << endl;
            //cout << ia << " " << to_string(va) << endl;

            ja = uvIndex[i] - 1;
            jb = uvIndex[i+1] - 1;
            jc = uvIndex[i+2] - 1;

            ta = uvs[ja];
            tb = uvs[jb];
            tc = uvs[jc];

            if (loadedNormal.size()) {
                ka = normalIndex[i] - 1;
                kb = normalIndex[i+1] - 1;
                kc = normalIndex[i+2] - 1;
                na = normals[ka];
                nb = normals[kb];
                nc = normals[kc];
                hasNormal = true;
            }

            triangle(va, vb, vc, ta, tb, tc);
        }
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

void texture(const char * path) {
    loadedTexture.clear();
    long lsize;
    byte * buffer;
    FILE* file = fopen(path, "rb");
    byte header[54];
    fread(header, sizeof(byte), 54, file);
    int headerOffset = *(int*) &header[14] + 14;
    int width = *(int*) &header[18];
    int height = *(int*) &header[22];
    textWidth = width;
    textHeight = height;
    byte* texture = new byte [width * height * 3];
    fseek(file, headerOffset, SEEK_SET);
    fread(texture, sizeof(byte), width * height * 3, file);
    vec3 pixelColor;
    byte b, g, r;
    int y, x;
    for (int i=0; i < width * height * 3; i += 3) {
        b = texture[i];
        g = texture[i+1];
        r = texture[i+2];
        y = i / width;
        x = i % width;
        pixelColor.x = r;
        pixelColor.y = g;
        pixelColor.z = b;
        loadedTexture.push_back(pixelColor);
    }
    fclose(file);
}

void loadNormal(const char * path) {
    loadedNormal.clear();
    long lsize;
    byte * buffer;
    FILE* file = fopen(path, "rb");
    byte header[54];
    fread(header, sizeof(byte), 54, file);
    int headerOffset = *(int*) &header[14] + 14;
    int width = *(int*) &header[18];
    int height = *(int*) &header[22];
    normWidth = width;
    normHeight = height;
    byte* normal = new byte [width * height * 3];
    fseek(file, headerOffset, SEEK_SET);
    fread(normal, sizeof(byte), width * height * 3, file);
    vec3 pixelColor;
    byte b, g, r;
    int y, x;
    for (int i=0; i < width * height * 3; i += 3) {
        b = normal[i];
        g = normal[i+1];
        r = normal[i+2];
        y = i / width;
        x = i % width;
        pixelColor.x = r;
        pixelColor.y = g;
        pixelColor.z = b;
        loadedNormal.push_back(pixelColor);
    }
    fclose(file);
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
    //glClearColor(1, 0, 0);
    //glClear();
    setLight(0, 0, 5);
     // Samus obj load
    
    

    //glObj("Book2.obj", 0.25, 0.25, 100, 0, 0, 0, 1);
    //glObj("sphere.obj", 0.5, 0.5, 5000, 0, 0, 0, 1);
    
    
    
    vec3 translate = {0, 0, 0};
    vec3 scale = {0.01, 0.01, 50};
    vec3 rotation = {0, 0, 0};
    vec3 eye = {0, 0, 5};
    vec3 center = {0, 0, 0};
    vec3 up = {0, 1, 0};
    lookAt(eye, center, up);
    glClearColor(3, 165, 252);
    glClear();
    //glObj("Samus.obj", 2, translate, scale, rotation);

    //Blanco
    texture("white.bmp");
    glObj("among.obj", 1, {0, 1, -0.001}, scale, rotation);

    //Verde
    texture("green.bmp");
    glObj("among.obj", 1, {7, 3, 0}, scale, {0, 0, 0});

    //Amarillo
    texture("yellow.bmp");
    glObj("among.obj", 1, {-5, 0, 0}, scale, rotation);

    //Azul
    texture("blue.bmp");
    glObj("among.obj", 1, {0, -1, 0.0005}, scale, rotation);

    //Rojo
    //Aplicado mapa normal
    texture("among.bmp");
    loadNormal("normal.bmp");
    glObj("among.obj", 1, {-7, 3, 0}, scale, rotation);
    //cout << to_string(model_mat) << endl;
    cout << "done";
    
    //cout << to_string(loadedTexture[0]);

    

    glFinish();
    return 0;
}
