#include <iostream>
#include <math.h>
#include <windows.h>

using namespace std;


#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURF_DIST .001
#define TAU 6.283185
#define PI 3.141592

void clearscreen()
{
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}


const int h=60, w=h, t=w*h;
//char scale[14] = " @%&#O*!;,.";
char scale[11] = " .:-=+*#%@";
//char scale[70] = " $@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,^`'.";

int frame = 0;

struct vec3{
    float x, y, z;

    float dot(vec3 a){return x*a.x + y*a.y + z*a.z;}

    vec3 cross(vec3 b){
        vec3 c(y*b.z - b.y*z,
           z*b.x - b.z*x,
           x*b.y - b.x*y);
        return c;

    }

    vec3():x(0), y(0), z(0){}
    vec3(float x, float y, float z):x(x), y(y), z(z){}
    vec3(float x):x(x), y(x), z(x){}
    vec3 toSph(){float r = sqrt(x*x+y*y+z*z);
                        return vec3(r, atan(y/x), acos(z/r));}

    vec3 rotx(float a){float c = cos(a), s = sin(a);
                        return vec3(x, c*y-s*z, s*y+c*z);}
    vec3 roty(float a){float c = cos(a), s = sin(a);
                        return vec3(c*x+s*z, y, c*z-s*x);}
    vec3 rotz(float a){float c = cos(a), s = sin(a);
                        return vec3(c*x-s*y, s*x+c*y, z);}
    vec3 rot(float a, float b, float c){
                        return vec3(x, y, z).rotx(a).roty(b).rotz(c);}

    vec3 t(float a, float b, float c){return vec3(x-a, y-b, z-c);}

    friend ostream& operator<<(ostream& os, const vec3& a);
};

vec3 rot(vec3 v, float a, float b, float c)
{
    return v.rotx(a).roty(b).rotz(c);

}

ostream& operator<<(ostream& os, const vec3& a)
{
    os <<'('<<a.x<<','<<a.y<<','<<a.z<<')';
    return os;
}
vec3 operator+(vec3 a, vec3 b)
{
    vec3 c(a.x + b.x, a.y+b.y, a.z+b.z);
    return c;
}
vec3 operator-(vec3 a, vec3 b)
{
    vec3 c(a.x - b.x, a.y-b.y, a.z-b.z);
    return c;
}
vec3 operator-(vec3 a)
{
    vec3 c(-a.x, -a.y, -a.z);
    return c;
}
vec3 operator*(float a, vec3& b)
{
    vec3 c(a * b.x , a*b.y , a*b.z);
    return c;
}

vec3 operator*(vec3& b, float a)
{
    return a*b;
}

float operator*(vec3 a, vec3 b)
{
    float c = a.x * b.x + a.y*b.y + a.z*b.z;
    return c;
}


float dot(vec3 a, vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z * b.z;
}

float length(vec3 a)
{
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

vec3 normalize(vec3 a)
{
    float l = length(a);
    vec3 c(a.x/l, a.y/l, a.z/l);
    return c;
}

vec3 cross(vec3 a, vec3 b)
{
    vec3 c(a.y*b.z - b.y*a.z,
           a.z*b.x - b.z*a.x,
           a.x*b.y - b.x*a.y);
    return c;
}


float map(float x, float a, float b, float c, float d)
{
    return (x-a)/(b-a)*(d-c)+c;
}

vec3 abs(vec3 a)
{

    return vec3(abs(a.x), abs(a.y), abs(a.z));

}

int sign(float a)
{
    if(a<0)
        return -1;
    return 1;

}

vec3 GetRayDir(vec3 uv, vec3 p, vec3 l, float z) {
    vec3
        f = normalize(l-p),//from origin to plane
        r = normalize(cross(vec3(0,1,0), f)),//right of plane
        u = normalize(cross(r,f)),//up of plane
        c = f*z,//focal dist
        i = c + uv.x*r + uv.y*u;//ray dir
    return normalize(i);
}

float sdSphere(vec3 p, float r)
{
    return length(p) - r;

}

float sdTorus( vec3 p, vec3 t )
{
  vec3 q = vec3(length(vec3(p.x, p.z, 0.))-t.x,p.y, 0.);
  return length(q)-t.y;
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p)-b;
    float z = 0.;
    return length(vec3(max(q.x,z), max(q.y,z), max(q.z,z)))+min(max(q.x, max(q.y, q.z)), z);

}

float min(float *objs, int n)
{
    float m = 999;
    for(int i = 0; i < n; i++)
        if(*(objs+i) < m)
            m = *(objs+i);

    return m;

}

struct Objs{

    float rotBall(vec3 p, vec3 c, float r, float rr, float v){
        return sdSphere(p.t(cos(frame*v)*rr, 0, sin(frame*1.*v)*rr) - c, r);
    }

};

Objs objs;

float Scene1(vec3 p){

    float objsl[10];
    int n = 4;
    objsl[0] = sdBox(p.t(0, 0, 0), vec3(0.3, 0.85, 0.3));
    objsl[1] = sdBox(p.t(0, 1.5 + 0.2*sin(frame/2.), 0).rot(1, frame/20., 0), vec3(0.3));
    objsl[2] = objs.rotBall(p.t(0, 0.2, 0), vec3(), 0.3, 0.8, 0.2);
    objsl[3] = sdTorus(p.rot(sin(frame/4.)*0.2, 0, sin(frame/4.)*0.2).t(0, -0.2, 0), vec3(1, 0.2, 0));

    return min(objsl, n);

}

float Scene2(vec3 p)
{
    float d = sdBox(p.rot(frame/20., frame/20., 0), vec3(0.5));

    return d;
}

float GetDist(vec3 p)
{

    float d = Scene1(p);
    return d;

}


vec3 GetNormal(vec3 p)
{
    float d = GetDist(p);
    float eps = 0.00001;
    return normalize(vec3(d-GetDist(vec3(p.x-eps, p.y, p.z)),
                d-GetDist(vec3(p.x, p.y-eps, p.z)),
                d-GetDist(vec3(p.x, p.y, p.z-eps))));

}


float RayMarch(vec3 ro, vec3 rd)
{

    float dO=0.;

    for(int i=0; i<MAX_STEPS; i++) {
    	vec3 p = ro + rd*dO;
        float dS = GetDist(p);
        dO += dS;
        if(dO>MAX_DIST) return -1.;
    }
    return dO;
}







vec3 ro = vec3(3, 1, 3);
vec3 dir = -normalize(ro);

float mainImage(int &x, int &y)
{
    float pixelf;
    //vec3(3, sin(frame/100.)*5., 0);
    vec3 look = ro + dir;

    //float w = iResolution.x, h = iResolution.y;
    vec3 uv = vec3((x-.5*(w-1.))/(h-1.),(y-.5*(h-1.))/(h-1.), 0.);

    vec3 rd = GetRayDir(uv, ro, look, 1.);

    vec3 light = normalize(vec3(20, 10, 10));

    float d = RayMarch(ro, rd);

    if(d<0.)
        pixelf = 0.;
    else{

        vec3 p = ro + d*rd;
        vec3 n = GetNormal(p);


    // Output to screen
        pixelf = map(dot(n, light), -1, 1, 0.1 , 1);

        }

}

void keys()
{
    vec3 up(0, 1, 0);
    if(GetKeyState(VK_UP) & 0x8000)
        {
            vec3 r = normalize(cross(vec3(0,1,0), dir)),//right of plane
            u = cross(dir, r);//up of plane
            dir = normalize(dir+u*0.1);
        }


    if(GetKeyState(VK_DOWN) & 0x8000)
        {
            vec3 r = normalize(cross(vec3(0,1,0), dir)),//right of plane
            u = cross(dir, r);//up of plane
            dir = normalize(dir-u*0.1);
        }
        //dir = dir.rotz(-0.1*sign(dot(dir, vec3(1,0,0)))).rotx(0.1*sign(dot(dir, vec3(0,0,1))));

    if(GetKeyState(VK_LEFT) & 0x8000)
        {
            dir = dir.roty(-0.1);
        }

    if(GetKeyState(VK_RIGHT) & 0x8000)
        {
            dir = dir.roty(0.1);
        }



    if(GetKeyState('W') & 0x8000)
        ro = ro + dir*0.1;

    if(GetKeyState('S') & 0x8000)
        ro = ro - dir*0.1;

    if(GetKeyState(VK_SPACE) & 0x8000)
        ro.y += 0.1;
    if(GetKeyState(VK_SHIFT) & 0x8000)
        ro.y -= 0.1;

    if(GetKeyState('A') & 0x8000)
    {
        vec3 rr = cross(dir, up);
        ro = ro + rr*0.1;
    }
    if(GetKeyState('D') & 0x8000)
    {
        vec3 rr = cross(dir, up);
        ro = ro - rr*0.1;
    }


}

char buffer[2*t + h + 2];

int main()
{
    char pixel;
    int gxy = 0, gb = 0;
    for(int y=0; y < h; y++)
        {
            for(int x=0; x < w; x++)
                {
                    pixel = scale[int(mainImage(x, y)*9)%9];
                    gxy = y*w + x;
                    gb = 2*gxy + y + 1;
                    buffer[gb] = pixel;
                    buffer[gb+1] = pixel;
                    //buffer[3*x+1] = pixel;
                    //buffer[x] = pixel;
                }
            buffer[gb+2] = '\n';

        }

    clearscreen();
    fwrite(buffer, sizeof(char), sizeof(buffer), stderr);

    keys();
    frame += 1;

    return main();
}


/*
//Line by line buffer
char buffer[int(2*w+2)];
pixel = scale[int(mainImage(x, y)*9)%9];
                    buffer[2*x] = pixel;
                    buffer[2*x+1] = pixel;
                    //buffer[3*x+1] = pixel;
                }
            buffer[2*w+1] = '\n';
*/

