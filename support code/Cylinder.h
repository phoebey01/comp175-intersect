#ifndef CYLINDER_H
#define CYLINDER_H

#include <iostream>
#include "Shape.h"
#include "Utility.h"
#include "Geometry.h"

class Cylinder : public Shape {
public:
    Cylinder() {};
    ~Cylinder() {};
    
    double Intersect(Point eyePointP, Vector rayV, Matrix transformMatrix) {
        
        Vector eye = invert(transformMatrix) * Vector(eyePointP[0], eyePointP[1], eyePointP[2]);
        
        Vector dhat = invert(transformMatrix) * normalize(rayV);
        
        double t = 1000000;
        double A = dhat[0] * dhat[0] + dhat[2] * dhat[2];
        double B = 2*eye[0]*dhat[0] + 2*eye[2]*dhat[2];
        double C = eye[0] * eye[0] + eye[2]*eye[2] - 0.25;
        
        double discriminate = (B*B) - (4*A*C);
        
        if (discriminate < 0 || (A==0)) {
            return -1;
        }
        
        double t0 = ((-B) - sqrt(discriminate)) / (2*A);
        double t1 = ((-B) + sqrt(discriminate)) / (2*A);
        
        double y0 = eye[1] + t0*dhat[1];
        double y1 = eye[1] + t1*dhat[1];
        if (t0 > 0.0 && y0 > -0.5 && y0 < 0.5) {
            t = t0;
        }
        if (t1 > 0.0 && y1 > -0.5 && y1 < 0.5 && t1 < t0) {
            t = t1;
        }
        
        // For the cylinder cap
        t0 = (0.5 - eye[1]) / dhat[1];
        double x = eye[0] + t0 * dhat[0];
        double z = eye[2] + t0 * dhat[2];
        if (x > -0.5 && (x*x + z*z < 0.25) && t0 < t) {
            t = t0;
        }
        
        t0 = (-0.5 - eye[1]) / dhat[1];
        x = eye[0] + t0 * dhat[0];
        z = eye[2] + t0 * dhat[2];
        if(x > -0.5 && (x*x + z*z < 0.25) && t0 < t) {
            t = t0;
        }
        if (t == 1000000)
            return -1;
        return t;
        
    };

    Vector findIsectNormal(Point eyePoint, Vector ray, double dist) {
        Point intersect = eyePoint + (ray * dist);
        
        //caps
        float offset = 0.00000005;
        if(fabs(intersect[1] + 0.5) < offset)
            return Vector(0.0, -1.0, 0.0);
        if(fabs(intersect[1] - 0.5) < offset)
            return Vector(0.0, 1.0, 0.0);
        
        //tube
        Vector norm = Vector(intersect[0], 0.0, intersect[2]);
        norm.normalize();
        return norm;
    };

protected:
    PVList getPoints(int segX, int segY) {
        if ((int)pvs.size() != segX * (segY + 1) + 2) {
            pvs.clear();
            PVList().swap(pvs); // free memory

            double ax = 2 * PI / segX;
            double r = DEFAULT_LENGTH; // radius

            for (int x = 0; x < segX; x++) {
                for (int y = 0; y < segY + 1; y++) {
                    Point p(
                        r * cos(x * ax),
                        DEFAULT_LENGTH - y / (double)segY,
                        r * sin(x * ax)
                    );
                    Vector v(
                        r * cos(x * ax),
                        0,
                        r * sin(x * ax)
                    );
                    v.normalize();
                    PV pv = {p, v};

                    pvs.push_back(pv);
                }
            }

            PV top = {Point(0, 0.5f, 0), Vector(0, 1, 0)};
            PV bot = {Point(0, -0.5f, 0), Vector(0, -1, 0)};
            pvs.push_back(top);
            pvs.push_back(bot);
        }
        return pvs;
    }

    Surface getSurface(int segX, int segY) {
        if ((int)surface.size() != segX * (2 * segY + 2)) {
            surface.clear();
            Surface().swap(surface); // free memory

            for (int x = 0; x < segX; x++) {
                for (int y = 0; y < segY + 1; y++) {
                    int li = x;
                    int ri = (x + 1) % segX;

                    if (y == 0) { // extra triangle to top
                        surface.push_back(toTriangle(
                            segX * (segY + 1), // top index
                            toIndex(li, y, segY + 1),
                            toIndex(ri, y, segY + 1)
                        ));
                    }

                    if (y == segY) { // triangle to bot only
                        surface.push_back(toTriangle(
                            segX * (segY + 1) + 1, // bot index
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
