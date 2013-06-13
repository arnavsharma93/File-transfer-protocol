//libraries, namespaces, compiler calls

#include<stdio.h>
#include <armadillo>
#include "inc/vec3.h"
#include <GL/glut.h>
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)

using namespace arma;
using namespace std;
#define MAX 100
#define SGL_TRIANGLES 1
//classes
typedef struct Point
{
    Vec3 Position;
    Vec3 Color;
}Point;

class MAT
{
    public:
        mat matrix;
        void init()
        {
            matrix << 1.0 << 0.0 << 0.0 << 0.0 << endr
            << 0.0 << 1.0 << 0.0 << 0.0 << endr
            << 0.0 << 0.0 << 1.0 << 0.0 << endr
            << 0.0 << 0.0 << 0.0 << 1.0 << endr;
        }

};

class STACK
{
    public:
        mat stack[MAX];
        int top;
        void init()
        {
            top = -1;
        }
        void push(mat A)
        {
            if(top >= MAX)
            {
                printf("Limit exceeded as stack is full\n");
            }
            else
            {   
                top = top + 1;
                stack[top]=A;
            }
        }
        void pop()
        {
            if(top != -1)
                top = top -1;
            else
                printf("Stack empty\n");
        }
};


//global variables
//library
STACK stackMod; //stack of modelling matrix
MAT currMod;//modelling matrix
MAT currView;//modelling matrix
MAT currProj;//projection matrix
int VP_x;
int VP_y;
int VP_w;
int VP_h;
Point store[100000];
int storeSize;
double  gRed;
double  gGreen;
double  gBlue;


//supporting function definitions
mat makeTranslate(float tx, float ty, float tz);
mat makeScale(float sx, float sy, float sz);
mat makeRotate(float angle, float sx, float sy, float sz);
void allInit();
void crossProd(const Vec3 &a, const Vec3 &b,Vec3 &c);



//function definitions
//Modelling functions

void sglModRotate(float angle,float axisx,float axisy,float axisz);
void sglModPushMatrix();
void sglModLoadIdentity();
void sglModPopMatrix();
void sglModTransalte(float tx, float ty, float tz);
void sglModScale(float sx, float sy, float sz);
void sglModMatrix(float mat[16] );

//View functions
void sglViewLoadIdentity();
void sglViewTranslate(float tx, float ty, float tz);
void sglViewRotate(float angle, float axisx, float axisy, float axisz);
void sglViewMatrix(float mat[16]);
void sglLookAt(double ex,double ey,double ez,double cx,double cy,double cz,double upx,double upy,double upz);

//projection functions
void sglProjOrtho(float left, float right, float bottom, float top, float near, float far);
void sglProjFrustum(double left, double right,double bottom, double top, double near, double far);

//drawing functions
void sglBegin(int type);
void sglEnd();
void sglColour(float r, float g,float b);
void sglClear(float r,float g,float b);
void sglClearColour(float r,float g,float b);
void sglVertex(double x,double y,double z);
void sglShow();

//viewport functions
void sglViewport(int x, int y, GLsizei width, GLsizei height);

//supporting functions
void allInit()
{
    currMod.init();//init modelling matrix
    stackMod.init();//init modelling stack
    currView.init();//init view matrix
    currProj.init();//init projection matrix
    storeSize = 0;
    gGreen = 1;
    gRed = 1;
    gBlue = 1;
}


mat makeTranslate(float tx, float ty, float tz)
{
    MAT temp;
    temp.matrix << 1.0 << 0.0 << 0.0 << tx << endr
        << 0.0 << 1.0 << 0.0 << ty << endr
        << 0.0 << 0.0 << 1.0 << tz << endr
        << 0.0 << 0.0 << 0.0 << 1.0 << endr;
    return temp.matrix;
}   

mat makeRotate(float angle,float axisx,float axisy,float axisz)
{
    angle = DEG2RAD(angle);
    Vec3 U;
    U.x = axisx; U.y = axisy; U.z = axisz;
    U.Normalize();
    double d = sqrt(U.y*U.y + U.z*U.z);
    mat R_al;
    R_al << 1.0 << 0.0 << 0.0 << 0.0 << endr
        << 0.0 << U.z/d << -U.y/d << 0.0 << endr
        << 0.0 << U.z/d << U.z/d << 0.0 << endr
        << 0.0 << 0.0 << 0.0 << 1.0 << endr;
    mat R_bt;
    R_bt << d << 0.0 << -U.x << 0.0 << endr
        << 0.0 << 1.0 << 0.0 << 0.0 << endr
        << U.x << 0.0 << d << 0.0 << endr
        << 0.0 << 0.0 << 0.0 << 1.0 << endr;
    mat R_th;
    R_th << cos(angle) << -sin(angle) << 0.0 << 0.0 << endr
        << sin(angle) << cos(angle) << 0.0 << 0.0 << endr
        << 0.0 << 0.0 << 1.0 << 0.0 << endr
        << 0.0 << 0.0 << 0.0 << 1.0 << endr;

    mat R;
    R = R_al.i() * R_bt.i() * R_th * R_bt * R_al;
    return R; 
}
mat makeScale(float sx, float sy, float sz)
{
    MAT temp;
    temp.matrix << sx << 0.0 << 0.0 << 0.0 << endr
        << 0.0 << sy << 0.0 << 0.0 << endr
        << 0.0 << 0.0 << sz << 0.0 << endr
        << 0.0 << 0.0 << 0.0 << 1.0 << endr;
    return temp.matrix;
}
//drawing functions
void sglBegin(int type)
{
    if(type == SGL_TRIANGLES)
        glBegin(GL_TRIANGLES);

}
void sglEnd()
{
    glEnd();
}

void sglColour(float r, float g,float b)
{
    gRed = r;
    gGreen = g;
    gBlue = b;
}
void sglClear(float r,float g,float b)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gRed = r;
    gGreen = g;
    gBlue = b;
    storeSize = 0;

}
void sglClearColour(float r,float g,float b)
{
    glClearColor(r,g,b,1.0);
}
void sglShow()
{
    FILE *f1;
    f1 = fopen("output.txt", "wt");
    for(int i=0 ; i < storeSize ; i++)
        fprintf(f1, "%lf %lf %lf\n",store[i].Position.x, store[i].Position.y, store[i].Position.z);
}
void sglVertex(double x, double y, double z)
{
        mat P1;
        mat P2;
        Vec3 D,W;
        P1 << x << endr <<
            y << endr <<
            z << endr <<
            1.0 << endr;
        P2 = (currProj.matrix * currView.matrix * currMod.matrix) * P1;
        D.x = P2(0,0)/P2(3,0) ;
        D.y = P2(1,0)/P2(3,0);
        D.z = P2(2,0)/P2(3,0);
        W.x = ((D.x + 1) * (VP_w/2)) + VP_x;
        W.y = ((D.y + 1) * (VP_h/2)) + VP_y;
        W.z = D.z;
        W.x = (W.x / (VP_w/2)) - 1;
        W.y = (W.y / (VP_h/2)) - 1;
        store[storeSize++].Position = W;
        store[storeSize].Color.x = gRed;
        store[storeSize].Color.y = gGreen;
        store[storeSize].Color.z = gBlue;
        glColor3f(gRed, gGreen, gBlue);
        glVertex3f(W.x, W.y, W.z);
        return;
}
//viewport functions
void sglViewport(int x, int y, GLsizei width, GLsizei height)
{
    VP_x = x;    
    VP_y = y;    
    VP_w = width;    
    VP_h = height;    
}

//projection functions
void sglProjOrtho(float l, float r, float b, float t, float n, float f)
{
    currProj.matrix << (2.0 / (r - l)) << 0.0 << 0.0 << - (r + l)/(r - l) << endr
        << 0.0 << 2.0/(t - b) << 0.0 << -(t + b) / (t - b) << endr
        << 0.0 << 0.0 << -2.0 /(f - n)  <<  -(f + n)/(f - n) << endr 
        << 0.0 << 0.0 << 0.0 << 1.0 << endr;
}

void sglProjFrustum(double l, double r,double b, double t, double n, double f)
{
    currProj.matrix << (2*n)/(r - l) << 0.0 << (r + l)/(r - l) << 0.0 << endr
        << 0.0 << (2 * n)/(t - b) << (t + b)/(t - b)<< 0.0 << endr
        << 0.0 << 0.0 << -(f + n)/(f - n) << (-2*f*n)/(f - n) << endr 
        << 0.0 << 0.0 << -1.0 << 0.0 << endr;
}

//View Functions
void sglViewLoadIdentity()
{
    currView.init();//init modelling matrix
}

void sglViewTranslate(float tx, float ty, float tz)
{
    mat temp = makeTranslate(tx, ty, tz);
    currView.matrix = currView.matrix * temp;
}

void sglViewRotate(float angle, float axisx, float axisy, float axisz)
{
    mat R = makeRotate(angle, axisx, axisy, axisz);
    currView.matrix =  currView.matrix * R; 
}

void sglViewMatrix(float mat[16])
{
    currView.matrix << mat[0] << mat[1] << mat[2] << mat[3] << endr
       <<  mat[4] << mat[5] << mat[6] << mat[7] << endr
       << mat[8] << mat[9] << mat[10] << mat [11] << endr
       << mat[12] << mat[13] << mat[14] << mat [15] << endr;
}

void sglLookAt(double ex,double ey,double ez,double cx,double cy,double cz,double upx,double upy,double upz)
{
    Vec3 forward, side, up;
    MAT m;
    forward.x = cx - ex;
    forward.y = cy - ey;
    forward.z = cz - ez;

    up.x = upx; 
   up.y = upy;
    up.z = upz;
    
    forward.Normalize();
    // side = forward * up
    side = crossprod(forward, up);
    side.Normalize();
    // up = side * forward
    up = crossprod(side, forward);

    m.init();
    m.matrix(0,0) = side.x;
    m.matrix(1,0) = side.y;
    m.matrix(2,0) = side.z;

    m.matrix(0,1) = up.x;
    m.matrix(1,1) = up.y;
    m.matrix(2,1) = up.z;


    m.matrix(0,2) = -forward.x;
    m.matrix(1,2) = -forward.y;
    m.matrix(2,2) = -forward.z;

    currView.matrix = currView.matrix * m.matrix; 
    sglViewTranslate(-ex, -ey, -ez);
}

//Modelling functions
void sglModMatrix(float mat[16])
{
    currMod.matrix << mat[0] << mat[1] << mat[2] << mat[3] << endr <<
        mat[4] << mat[5] << mat[6] << mat[7] << endr <<
        mat[8] << mat[9] << mat[10] << mat [11] << endr <<
        mat[12] << mat[13] << mat[14] << mat [15] << endr;
}
void sglModLoadIdentity()
{
    currMod.init();//init modelling matrix
}

void sglModTranslate(float tx, float ty, float tz)
{
    mat temp = makeTranslate(tx, ty, tz);
    currMod.matrix = currMod.matrix * temp; 

}
void sglModScale(float sx, float sy, float sz)
{
    mat temp = makeScale(sx, sy, sz);
    currMod.matrix = currMod.matrix * temp; 
}

void sglModRotate(float angle,float axisx,float axisy,float axisz)
{
    mat R = makeRotate(angle, axisx, axisy, axisz);
    currMod.matrix = currMod.matrix * R;  
}

void sglModPushMatrix()
{
    stackMod.push(currMod.matrix);
}

void sglModPopMatrix()
{
    currMod.matrix = stackMod.stack[stackMod.top]; 
    stackMod.pop();
}




