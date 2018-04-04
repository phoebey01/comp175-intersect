#ifndef CONE_H
#define CONE_H

#include "Utility.h"
#include "Geometry.h"
#include "Shape.h"

class Cone : public Shape {
public:
    Cone() {};
    ~Cone() {};

    void draw() {
    };

    void drawNormal() {
    };

    double Intersect(Point eyePointP, Vector rayV, Matrix transformMatrix) {
        return 0;
    };

    Vector findIsectNormal(Point eyePoint, Vector ray, double dist) {
        Vector v;
        return v;
    };

protected:
	PVList getPoints(int segX, int segY) {
		if ( int(pvs.size()) != segX * (segY + 1) + 1){
			// free memory
			pvs.clear();
			PVList().swap(pvs);

			double r = DEFAULT_LENGTH;
			double angle_x = 2 * PI / segX;

			PVList line;

			for (int i=0; i<segY+1; i++){
				Point p(
					r * (i/ float(segY)),
					DEFAULT_LENGTH - (i/float(segY)),
					0
				);
				Vector v(1.0,.5f,0.0);
				v.normalize();
				PV pv = {p, v};
				line.push_back(pv);
                
			}

			// rotate the line around y
			for (int i=0; i<segX; i++){
				Matrix mtx = rotY_mat(i*angle_x);
				PVList sideLine = rotatePVs(mtx, line);
				pvs.insert(pvs.end(), sideLine.begin(), sideLine.end());
			}

            PV bottom = {Point(0,-DEFAULT_LENGTH,0), Vector(0, -1, 0)};
            pvs.push_back(bottom);
		}

		return pvs;
	};

	Surface getSurface(int segX, int segY) {

		if ( int(surface.size()) != segX * ((segY-1)*2 + 2) ){	

			surface.clear();
			Surface().swap(surface);

			for (int i=0; i<segX; i++){
                for (int j=0; j<segY+1; j++){

                    int ri = (i+1)%segX; // right line index

                    if (j==0){ // tip
                        surface.push_back(toTriangle(
                            toIndex(i, j, segY + 1),
                            toIndex(i, j + 1, segY + 1),
                            toIndex(ri, j + 1, segY + 1)
                        ));
                    }
                    else if (j == segY){ //bottom
                        surface.push_back(toTriangle(
                            toIndex(i, j, segY + 1),
                            toIndex(ri, j, segY + 1),
                            int( pvs.size()-1 )
                        ));
                    }
                    else {
                        surface.push_back(toTriangle(
                            toIndex(i, j, segY + 1),
                            toIndex(i, j + 1, segY + 1),
                            toIndex(ri, j + 1, segY + 1)
                        ));
                        surface.push_back(toTriangle(
                            toIndex(i, j, segY + 1),
                            toIndex(ri, j + 1, segY + 1),
                            toIndex(ri, j, segY + 1)
                        ));
                    }

                }
            }
		}

        return surface;
	}

};

#endif
