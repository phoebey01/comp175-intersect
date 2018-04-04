#ifndef CUBE_H
#define CUBE_H

#include "Utility.h"
#include "Geometry.h"
#include "Shape.h"

class Cube : public Shape {
public:
    Cube() {};
    ~Cube() {};
    
    double Intersect(Point eyePointP, Vector rayV, Matrix transformMatrix) {
        return 0;
    };

    Vector findIsectNormal(Point eyePoint, Vector ray, double dist) {
        Vector v;
        return v;
    };

protected:
    PVList getPoints(int segX, int segY) {
        if ((int)pvs.size() != 6 * (segX + 1) * (segY + 1)) {
            pvs.clear();
            PVList().swap(pvs); // free memory

            /* only calculate first face*/
            PVList xyFace;

            for (int x = 0; x < segX + 1; x++) {
                for (int y = 0; y < segY + 1; y++) {
                    Point p(
                        -DEFAULT_LENGTH + x / (double)segX,
                        DEFAULT_LENGTH - y / (double)segY,
                        DEFAULT_LENGTH
                    );
                    Vector v(0.0f, 0.0f, 0.5f);
                    PV pv = {p, v};
                    xyFace.push_back(pv);
                }
            }

            /* rotate around y axis for 4 faces */
            for (int a = 0; a < 4; a++) {
                Matrix mtx = rotY_mat(DEG_TO_RAD(a * 90));
                PVList sidepvs = rotatePVs(mtx, xyFace);
                pvs.insert(pvs.end(), sidepvs.begin(), sidepvs.end());
            }

            /* rotate around x axis for top bot face*/
            for (int a = -1; a <= 1; a++) {
                if (a == 0) continue; // don't double count 0
                Matrix mtx = rotX_mat(DEG_TO_RAD(a * 90));
                PVList sidepvs = rotatePVs(mtx, xyFace);
                pvs.insert(pvs.end(), sidepvs.begin(), sidepvs.end());
            }
        }
        return pvs;
    }

    Surface getSurface(int segX, int segY) {
        if ((int)surface.size() != 12 * segX * segY) {
            surface.clear();
            Surface().swap(surface); // free memory

            for (int s = 0; s < 6; s++) { // side
                for (int x = 0; x < segX; x++) {
                    for (int y = 0; y < segY; y++) {
                        int li = x, ri = x + 1;

                        surface.push_back(toTriangle(
                            toSideIndex(s, ri, y, segX + 1, segY + 1),
                            toSideIndex(s, li, y, segX + 1, segY + 1),
                            toSideIndex(s, ri, y + 1, segX + 1, segY + 1)
                        ));
                        surface.push_back(toTriangle(
                            toSideIndex(s, ri, y + 1, segX + 1, segY + 1),
                            toSideIndex(s, li, y, segX + 1, segY + 1),
                            toSideIndex(s, li, y + 1, segX + 1, segY + 1)
                        ));
                    }
                }
            }
        }
        return surface;
    }
};

#endif
