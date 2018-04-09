#ifndef SHAPE_H
#define SHAPE_H

#include <GL/glui.h>
#include "Algebra.h"
#include "Geometry.h"
#include "Utility.h"

#define DEFAULT_LENGTH 0.5f

class Shape {
public:
    Shape() {};
    ~Shape() {};

    void setSegments(int x, int y) {
        m_segmentsX = x;
        m_segmentsY = y;
    }

    void draw() {
        PVList pvs = getPoints(m_segmentsX, m_segmentsY);
        Surface surface = getSurface(m_segmentsX, m_segmentsY);
        

        for (Surface::iterator it = surface.begin(); it < surface.end(); it++) {
            Face tri = *it;

            glBegin(GL_TRIANGLES);
            for (int i = 0; i < 3; i++) {
                PV pv = pvs[tri[i]];
                glNormal3dv(pv.v.unpack());
                glVertex3dv(pv.p.unpack());
            }
            glEnd();
        }

    }

    void drawNormal() {
        PVList pvs = getPoints(m_segmentsX, m_segmentsY);

        for (PVList::iterator it = pvs.begin(); it < pvs.end(); it++) {
            PV pv = *it;

            glBegin(GL_LINES);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3dv(pv.p.unpack());
            glVertex3dv((pv.p + pv.v / 15.0f).unpack());
            glEnd();
        }
    }

    virtual double Intersect(Point eyePointP, Vector rayV, Matrix transformMatrix) = 0;
    virtual Vector findIsectNormal(Point eyePoint, Vector ray, double dist) = 0;

protected:
    void normalizeNormal (float x, float y, float z) {
        normalizeNormal (Vector(x, y, z));
    };

    void normalizeNormal (Vector v) {
        v.normalize();
        glNormal3dv(v.unpack());
    };

    virtual PVList getPoints(int segX, int segY) { return pvs; };
    virtual Surface getSurface(int segX, int segY) { return surface; };

    int m_segmentsX, m_segmentsY;
    PVList pvs;
    Surface surface;
};

#endif