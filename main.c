#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tga.h"
#include "model.h"

typedef int Vector[3];
typedef double Mat4x4[4][4];
typedef double Mat4x1[4];
 
void swap(int *a, int *b);
int abs(int a);
double d_abs(double a);
int Round(double a);
int c_length(int a, int b, int c, int *s);
void product_vec3(Vec3 A, Vec3 B, Vec3 *W);
double v_length(Vec3 A);
double product_dot(Vec3 A, Vec3 B);
double intension (Vec3* v0, Vec3* v1, Vec3* v2, Vec3 light);
void normal_vec3(Vec3* A, double l);
void product_mat(Mat4x4 A, Mat4x1 B, Mat4x1* C);


void line (tgaImage *image, 
           int x0, int y0,
           int x1, int y1,
           tgaColor color);
void triangle(tgaImage *image, Model* model, Vector a, Vector b, Vector c, Vec3 UVa, Vec3 UVb, Vec3 UVc, double I, int* zbuffer);

int main(int argc, char **argv)
{
    int rv = 0;
    if (argc < 4) {
        fprintf(stderr, "Usage: %s directory to obj or outfile \n ", argv[0]);
        return -1;
    }
    tgaImage * image = tgaNewImage(1000, 1000, RGB);
    Model *model = loadFromObj(argv[1]);
	int DiffuseMap = loadDiffuseMap(model, argv[2]); printf("%d", DiffuseMap);
    int i,j;
    double I;
    double coef = 3.0;
    double r = -1/coef;
	Vec3 h = {0.0,1.0,0.0};
	normal_vec3(&h,v_length(h));
	Vec3 e = {1.0,0.0,0.0};
	normal_vec3(&e,v_length(e));
	Vec3 c = {0.0,0.5,0.0};
	Vec3 l = {0.0,0.0,0.0};
	Vec3 light = { 0.0, 0.0, 100.0 };
	normal_vec3(&light,v_length(light));
	product_vec3(e,h,&l);
	normal_vec3(&l,v_length(l));
    Mat4x4 vw1 = {
               {e[0], h[0], l[0], 0.0},
               {e[1], h[1], l[1], 0.0},
               {e[2], h[2], l[2], 0.0},
               {0.0, 0.0, 0.0, 1.0}
               };
    Mat4x4 vw2 = {
               {1.0, 0.0, 0.0, -c[0]},
               {0.0, 1.0, 0.0, -c[1]},
               {0.0, 0.0, 1.0, -c[2]},
               {0.0, 0.0, 0.0, 1.0}
               };
    Mat4x4 a = {
               {1.0, 0.0, 0.0, 0.0},
               {0.0, 1.0, 0.0, 0.0},
               {0.0, 0.0, 1.0, 0.0},
               {0.0, 0.0,   r, 1.0}
               };
    Mat4x1 b, V,newV = {0.0,0.0,0.0};
    Vector A, B, C;
    int* z_buffer = malloc(image->width*image->height*sizeof(int));
        for (i = 0; i < image->height; ++i) {
            for (j = 0; j < image->width; ++j) {
                z_buffer[j + i*image->width] = -10000;
            }
        } 
    for(j = 0; j < model->nface; ++j) {
	Vec3 *v0 = getVertex(model, j, 0);
	Vec3 *v1 = getVertex(model, j, 1);
	Vec3 *v2 = getVertex(model, j, 2);
	Vec3 *uv0 = getDiffuseUV(model, j, 0);
	Vec3 *uv1 = getDiffuseUV(model, j, 1);
	Vec3 *uv2 = getDiffuseUV(model, j, 2);
	Vec3 UVa, UVb, UVc;
	UVa[0] = (*uv0)[0];
	UVa[1] = (*uv0)[1];
	UVa[2] = (*uv0)[2];
	UVb[0] = (*uv1)[0];
	UVb[1] = (*uv1)[1];
    	UVb[2] = (*uv1)[2];
	UVc[0] = (*uv2)[0];
   	UVc[1] = (*uv2)[1];
   	UVc[2] = (*uv2)[2];

        for(i = 0; i < 3; ++i) {
            b[i] = (*v0)[i];
        }
        b[3] = 1;

	product_mat(vw2,b,&V);
	product_mat(vw1,V,&b);
        product_mat(a, b, &V);

        A[0] = (V[0]/V[3] + 1)*image->width/2;
		A[1] = (V[1]/V[3] + 1)*image->height / 2;
		A[2] = (V[2]/V[3] + 1)*255/2;
        for(i = 0; i < 3; ++i) {
            b[i] = (*v1)[i];
        }
        b[3] = 1;

	product_mat(vw2,b,&V);
	product_mat(vw1,V,&b);
        product_mat(a, b, &V);

		B[0] = (V[0]/V[3] + 1)*image->width / 2;
		B[1] = (V[1]/V[3] + 1)*image->height / 2;
		B[2] = (V[2]/V[3] + 1) *255/2;
        for(i = 0; i < 3; ++i) {
            b[i] = (*v2)[i];
        }
        b[3] = 1;

	product_mat(vw2,b,&V);
	product_mat(vw1,V,&b);
        product_mat(a, b, &V);

		C[0] = (V[0]/V[3] + 1)*image->width / 2;
		C[1] = (V[1]/V[3] + 1)*image->height / 2;
        C[2] = (V[2]/V[3] + 1)*255/2; 
        I = intension(v0, v1, v2, light);  
		triangle(image, model, A, B, C,UVa, UVb, UVc, d_abs(I), z_buffer);
    }
    tgaFlipVertically(image);
    if (-1 == tgaSaveToFile(image, argv[3])) {
        perror("tgaSateToFile");
        rv = -1;
    }
    free(z_buffer);
    tgaFreeImage(image); 
	freeModel(model);
    return rv;
}

void line (tgaImage *image, 
           int x0, int y0,
           int x1, int y1,
           tgaColor color)
{  
    int flag = 0;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    if (dx < dy) {
	    swap(&x0,&y0);
	    swap(&x1,&y1);
	    swap(&dx,&dy);
        flag = 1;
    }
    if (x0 > x1) {
        swap(&x0,&x1);
	    swap(&y0,&y1);
    }
    int e = 0;
    int de = 2*abs(y1 - y0);
    int i;
    int x = x0;
    int y = y0;	
    for (i = 0;i <= dx; ++i) {
	if (e >= dx) {
        e -= 2*dx;
	    if (y1 < y0) { 	
            y -= 1;
	    } else {
	        y += 1;
	    }
        }
	if (flag) {
	    tgaSetPixel(image, y, x, color);
	} else {
	    tgaSetPixel(image, x, y, color);
	}
	x += 1;
	e += de;
    }
}

void triangle(tgaImage *image, Model* model, Vector a, Vector b, Vector c, Vec3 UVa, Vec3 UVb, Vec3 UVc, double I, int* zbuffer) {
    int i0 = 0;
    int i1 = c_length(a[1], b[1], c[1], &i0);
    int j0 = 0;
    int j1 = c_length(a[0], b[0], c[0], &j0);
    Vector P;
	Vec3 Puv, X, Y, W;
    tgaColor col;
    int i,j;
    double U = 0;
    double V = 0;
    for (i = i0; i <= i1; ++i) {
       P[1] = i; 
       for (j = j0; j <= j1; ++j) {
           P[0] = j;
           X[0] = a[0] - P[0]; 
           X[1] = b[0] - a[0]; 
           X[2] = c[0] - a[0];
           Y[0] = a[1] - P[1]; 
           Y[1] = b[1] - a[1]; 
           Y[2] = c[1] - a[1]; 
           product_vec3(X, Y, &W);
           U = W[1]/W[0];
           V = W[2]/W[0];
           if ((U < 0) || (V < 0) || ((1 - U - V) < 0)) {
               continue;
           }
           P[2] = (int)((1 - U - V)*a[2] + U*b[2] + V*c[2] + 0.5);
		   Puv[0] = (1 - U - V)*UVa[0] + U*UVb[0] + V*UVc[0];
		   Puv[1] = (1 - U - V)*UVa[1] + U*UVb[1] + V*UVc[1];
               if (P[2] > zbuffer[j + i*image->width]) {
                   zbuffer[j + i*image->width] = P[2];
		   col=getDiffuseColor(model, &Puv);
                   tgaSetPixel(image, P[0], P[1], tgaRGB(I*Red(col), I*Green(col), I*Blue(col)) );
               }
       }
    }
}
void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int c_length(int A, int B, int C, int *s) {
    int a = A;
    int b = B;
    int c = C;
    if (a >= b) {
        swap(&a, &b);
    }
    if (a >= c) {
        swap(&a, &c);
    }
    if (b >= c) {
        swap(&b, &c);
    }
    *s = a;
    return c ;
}

int abs(int a) {
    return (a > 0) ? a : -a;
}

double d_abs(double a) {
    return (a > 0) ? a : -a;
}

int Round(double a) {
    int b = a;
    if ((a - b) >= 0.5) {
        return (b + 1);
    } else {
	return b;
    }
}

void product_vec3(Vec3 A, Vec3 B, Vec3* W) {
    (*W)[0] = A[1]*B[2] - A[2]*B[1];
    (*W)[1] = B[0]*A[2] - A[0]*B[2];
    (*W)[2] = A[0]*B[1] - A[1]*B[0];
}

double product_dot(Vec3 A, Vec3 B) {
    double dot = A[0]*B[0] + A[1]*B[1] + A[2]*B[2];
    return dot;
} 

double v_length(Vec3 A) {
    double length = sqrt(A[0]*A[0] + A[1]*A[1] + A[2]*A[2]);
    return length;
}
double intension (Vec3* v0, Vec3* v1, Vec3* v2, Vec3 light) {
    Vec3 AB, AC;
    AB[0] = (*v1)[0] - (*v0)[0];
    AB[1] = (*v1)[1] - (*v0)[1];
    AB[2] = (*v1)[2] - (*v0)[2];
    AC[0] = (*v2)[0] - (*v0)[0];
    AC[1] = (*v2)[1] - (*v0)[1];
    AC[2] = (*v2)[2] - (*v0)[2];
    Vec3 W;
    product_vec3(AB, AC, &W);
    W[0] = -W[0];
    W[1] = -W[1];
    W[2] = -W[2];
    normal_vec3(&W, v_length(W));
    return product_dot(light, W); 
}

void normal_vec3(Vec3* A, double l) {
    (*A)[0] = (*A)[0]/l;
    (*A)[1] = (*A)[1]/l;
    (*A)[2] = (*A)[2]/l;
}

void product_mat(Mat4x4 A, Mat4x1 B, Mat4x1* C) {
    int i,j;
    Mat4x1 V = {0.0, 0.0, 0.0, 0.0};
    for(i=0; i < 4; ++i) {
        for(j=0; j < 4; ++j) {
            V[i] = V[i] + A[i][j]*B[j];
        }
        (*C)[i] = V[i];
    }
}


 
