#include "GraphicsObject.hpp"

GraphicsObject::GraphicsObject(ScenePrimitive *prim, const Matrix &xf) {
    obj = *prim;
    this->xf = xf;
}

GraphicsObject::GraphicsObject(const GraphicsObject &G) {
    obj = G.obj;
    xf = G.xf;
}
