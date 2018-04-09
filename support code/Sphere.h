#ifndef SPHERE_H
#define SPHERE_H

#include "Geometry.h"
#include "Shape.h"
#include "Utility.h"
#include "Algebra.h"
#include <math.h>



class Sphere : public Shape {
public:
    Sphere() {};
    ~Sphere() {};

    double Intersect(Point eyePointP, Vector rayV, Matrix transformMatrix) {
                
        Vector eye = invert(transformMatrix) * Vector(eyePointP[0], eyePointP[1], eyePointP[2]);
        
        Vector dhat = invert(transformMatrix) * normalize(rayV);
        
        double A = dot(dhat, dhat);
        double B = 2 * dot(eye, dhat);
        double C = dot(eye, eye) - (0.5 * 0.5);
        
        double discriminate = (B*B) - (4*A*C);
        
        if (discriminate < 0 || (A==0)) {
            return -1;
        }
        
        double t0 = ((-B) + sqrt(discriminate)) / (2*A);
        double t1 = ((-B) - sqrt(discriminate)) / (2*A);
        
        return fmin(t0, t1);
        
    };

    Vector findIsectNormal(Point eyePoint, Vector ray, double dist) {
        Point intersect = eyePoint + (ray * dist);
        Vector norm = Vector(intersect[0], intersect[1], intersect[2]);
        norm.normalize();
        return norm;
    };

protected:
    PVList getPoints(int segX, int segY) {
        if ((int)pvs.size() != segX * (segY + 1)) {
            pvs.clear();
            PVList().swap(pvs); // free memory

            double ax = 2 * PI / segX;
            double ay = PI / segY;
            double r = DEFAULT_LENGTH; // radius

            for (int x = 0; x < segX; x++) {
                for (int y = 0; y < segY + 1; y++) {
                    Point p(
                        r * sin(y * ay) * cos(x * ax),
                        r * cos(y * ay),
                        r * sin(y * ay) * sin(x * ax)
                    );
                    Vector v(
                        r * sin(y * ay) * cos(x * ax),
                        r * cos(y * ay),
                        r * sin(y * ay) * sin(x * ax)
                    );
                    v.normalize();
                    PV pv = {p, v};

                    pvs.push_back(pv);
                }
            }
        }
        return pvs;
    }

    Surface getSurface(int segX, int segY) {
        if ((int)surface.size() != segX * (2 * segY - 2)) {
            surface.clear();
            Surface().swap(surface); // free memory

            for (int x = 0; x < segX; x++) { // line by line
                for (int y = 0; y < segY; y++) { // point per line
                    int li = x; // left line index
                    int ri = (x + 1) % segX; // right line index

                    if (y == 0) { // top
                        surface.push_back(toTriangle(
                            0,
                            toIndex(li, y + 1, segY + 1),
                            toIndex(ri, y + 1, segY + 1)
                        ));
                    } else if (y == segY - 1) { // bottom
                        surface.push_back(toTriangle(
                            segY,
                            toIndex(ri, y, segY + 1),
                            toIndex(li, y, segY + 1)
                        ));
                    } else {
                        surface.push_back(toTriangle(
                            toIndex(ri, y, segY + 1),
                            toIndex(li, y, segY + 1),
                            toIndex(ri, y + 1, segY + 1)
                        ));
                        surface.push_back(toTriangle(
                            toIndex(ri, y + 1, segY + 1),
                            toIndex(li, y, segY + 1),
                            toIndex(li, y + 1, segY + 1)
                        ));
                    }
                }
            }
        }
        return surface;
    }
};

#endif
