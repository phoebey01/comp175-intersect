#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdexcept>
#include <vector>
#include "Algebra.h"


struct PV {
    Point p;
    Vector v;
};

typedef std::vector<PV> PVList;
typedef std::vector<unsigned> Face;
typedef std::vector<Face> Surface;

Face toTriangle(unsigned p1, unsigned p2, unsigned p3) {
    Face face;
    face.push_back(p1);
    face.push_back(p2);
    face.push_back(p3);
    return face;
}

#endif
