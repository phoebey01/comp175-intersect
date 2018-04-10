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
        
        double t = 1000000;
        Vector eye = invert(transformMatrix) * Vector(eyePointP[0], eyePointP[1], eyePointP[2]);
        
        Vector dhat = invert(transformMatrix) * normalize(rayV);
        
        double A = dhat[0]*dhat[0] + dhat[2]*dhat[2] - 0.25*dhat[1]*dhat[1];
        //double B = 2*eye[0]*dhat[0] + 2*eye[2]*dhat[2] - 0.5*eye[1]*dhat[1] + 0.25*dhat[1];
        double B = 2*eye[0]*dhat[0] + 2*eye[2]*dhat[2] + 0.5*dhat[1]*(0.5 - eye[1]);
        //double C = eye[0]*eye[0] + eye[2]*eye[2] - 0.25*eye[1]*eye[1] - 0.0625 + 0.25*eye[1];
        double C = eye[0]*eye[0] + eye[2]*eye[2] - 0.25*(pow(0.5 - eye[1], 2));
        
        double discriminate = (B*B) - (4*A*C);
        
        if (discriminate < 0 || (A==0)) {
            return -1;
        }
        
        double t0 = ((-B) - sqrt(discriminate)) / (2*A);
        double t1 = ((-B) + sqrt(discriminate)) / (2*A);
        
        double y0 = eye[1] + t0 * dhat[1];
        double y1 = eye[1] + t1 * dhat[1];
        
        if (t0 > 0.0 && y0 > -0.5 && y0 < 0.5) {
            t = t0;
        }
        if (t1 > 0.0 && y1 > -0.5 && y1 < 0.5 && t1 < t0) {
            t = t1;
        }
        
        //For the cap
        t0 = (-0.5 - eye[1]) / dhat[1];
        double a = eye[0] + t0 * dhat[0];
        double b = eye[2] + t0 * dhat[2];
        if (a > -0.5 && (a*a + b*b < 0.25) && t0 < t) {
            t = t0;
        }
        if (t == 1000000) return -1;
        return t;
    };

    Vector findIsectNormal(Point eyePoint, Vector ray, double dist) {
        Point intersect = eyePoint + (ray * dist);
        
        // cap
        float offset = 0.00000005;
        if (intersect[1] + 0.5 < offset)
            return Vector(0.0, -1.0, 0.0);
        
        Vector norm = Vector(intersect[0], 0.0, intersect[2]);
        norm.normalize();
        norm[1] = 0.5;
        norm.normalize();
        return norm;
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
