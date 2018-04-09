#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Algebra.h"

enum CameraMode { CAMERA_PERSPECTIVE_MODE, CAMERA_ORTHO_MODE };

class Camera {
public:
    Camera();
    ~Camera();
    void Orient(Point& eye, Point& focus, Vector& up);
    void Orient(Point& eye, Vector& look, Vector& up);
    void SetViewAngle (double viewAngle);
    void SetNearPlane (double nearPlane);
    void SetFarPlane (double farPlane);
    void SetScreenSize (int screenWidth, int screenHeight);

    Matrix GetProjectionMatrix() const;
    Matrix GetModelViewMatrix() const;

    void RotateV(double angle);
    void RotateU(double angle);
    void RotateW(double angle);
    void Rotate(Point p, Vector axis, double degree);
    void Translate(const Vector &v);

    Point GetEyePoint() const;
    Vector GetLookVector() const;
    Vector GetUpVector() const;
    double GetViewAngle() const;
    double GetNearPlane() const;
    double GetFarPlane() const;
    int GetScreenWidth() const;
    int GetScreenHeight() const;

    double GetFilmPlaneDepth() const;
    double GetScreenWidthRatio() const;

    CameraMode getMode() const;
    Camera &setMode(CameraMode m);
    
    Matrix getUvw2xyzMatrix() const;

    void Reset();
    
    Vector getU();
    Vector getV();
private:
    Matrix getXyz2uvwMatrix() const;
    
    Matrix GetScaleMatrix() const;
    Matrix GetUnhingeMatrix() const;
    Matrix GetTranslationMatrix() const;

    double Angle;
    Point Eye;
    Vector u;
    Vector v;
    Vector w;
    Vector up;
    double near;
    double far;
    int width;
    int height;

    CameraMode mode;
};
#endif
