

#ifndef CAMERA_H
#define CAMERA_H

#include "Algebra.h"

class Camera {
public:
    Camera();
    ~Camera();
    void Reset();
    void Orient(Point& eye, Point& focus, Vector& up);
    void Orient(Point& eye, Vector& look, Vector& up);
    void SetViewAngle (double viewAngle);
    void SetNearPlane (double nearPlane);
    void SetFarPlane (double farPlane);
    void SetScreenSize (int screenWidth, int screenHeight);

    Matrix GetProjectionMatrix();
    Matrix GetModelViewMatrix();

    void RotateV(double angle);
    void RotateU(double angle);
    void RotateW(double angle);
    void Rotate(Point p, Vector axis, double degree);
    void Translate(const Vector &dir);

    Point GetEyePoint();
    Vector GetLookVector();
    Vector GetUpVector();
    double GetViewAngle();
    double GetNearPlane();
    double GetFarPlane();
    int GetScreenWidth();
    int GetScreenHeight();

    double GetFilmPlaneDepth();
    double GetScreenWidthRatio();

protected:
    // member vars
    Point eye;
    Vector ub, vb, wb; // u basis, v basis, w basis

    double heightAngle; // view angle
    double near;
    double far;

    int screenWidth;
    int screenHeight;

    // helpers
    void setUVW(const Vector &look, const Vector &up);
    double widthAngle();
    Matrix unhingeNorm();
    Matrix scaleNorm();
    Matrix rotateNorm();
    Matrix translateNorm();
};
#endif

