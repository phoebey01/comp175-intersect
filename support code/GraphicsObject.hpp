#ifndef __GRAPHICSOBJECT_HPP__
#define __GRAPHICSOBJECT_HPP__

#include "SceneData.h"
#include "Algebra.h"

/* GraphicsObject:
 *
 *   A single primitve, and its aggregate global transformation (*not* relative
 *   to its original parent).
 */


class GraphicsObject {
public:
    /* A ScenePrimitive and the SceneTransformations that are to be applied to
     * it, and the transformation of the object that it is a child of. */
    GraphicsObject(ScenePrimitive *prim,
                   const Matrix &xf);
    GraphicsObject(const GraphicsObject &G);

    const Matrix &transform() const;
    const ScenePrimitive &object() const;
    const Matrix &invTrans() const;
    void setInv(Matrix inv) {
        invMat = inv;
    }

private:
    Matrix xf;
    ScenePrimitive obj;
    
    /* New for assignment 4 */
    Matrix invMat;
};

inline const Matrix &GraphicsObject::transform() const { return xf; }
inline const ScenePrimitive &GraphicsObject::object() const { return obj; }
inline const Matrix &GraphicsObject::invTrans() const { return invMat; }

#endif /* __GRAPHICSOBJECT_HPP__ */
