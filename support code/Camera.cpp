#include "Camera.h"
#include <iostream>
#include <cstdio>
#include <stdio.h>

using namespace std;

Camera::Camera() {
    mode = CAMERA_PERSPECTIVE_MODE;
}

Camera::~Camera() { }

void Camera::Orient(Point& eye, Point& focus, Vector& up) {
    Vector look = focus - eye;
    Orient(eye, look, up);
}

void Camera::Orient(Point& eye, Vector& look, Vector& up) {
    this->Eye = eye;
    this->w = -normalize(look);
    u = cross(up, this->w);
    this->u = normalize(u);
    v = cross(this->w, this->u);
    this->v = normalize(v);
    this->up = up;
}

Matrix Camera::GetProjectionMatrix() const {
    switch(mode) {
    case CAMERA_ORTHO_MODE: {
        Matrix S = Matrix(1.,0,0,0,
                          0,1.,0,0,
                          0,0,2./(far - near),0,
                          0,0,0,1.);
        Matrix T = Matrix(1.,0,0,0,
                          0,1.,0,0,
                          0,0,-1.,-(far+near)/2,
                          0,0,0,1);
        return S*T;
    }
    case CAMERA_PERSPECTIVE_MODE:
    default:
        return GetUnhingeMatrix() * GetScaleMatrix();
    }
}

Matrix Camera::GetScaleMatrix() const {
    double farplane_width = tan((DEG_TO_RAD(Angle / 2.))) * far;
    return Matrix(1. / farplane_width, 0, 0, 0,
                  0, width / (farplane_width * height), 0, 0,
                  0, 0, 1. / far, 0,
                  0, 0, 0, 1);
}
Matrix Camera::GetUnhingeMatrix() const {
    double c = -near / far;
    return Matrix(1., 0, 0, 0,
                  0, 1., 0, 0,
                  0, 0, -1. / (c + 1.), c / (c + 1.),
                  0, 0, -1., 0);
}


void Camera::SetViewAngle (double viewAngle) {
    Angle = viewAngle;
}

void Camera::SetNearPlane (double nearPlane) {
    near = nearPlane;
}

void Camera::SetFarPlane (double farPlane) {
    far = farPlane;
}

void Camera::SetScreenSize (int screenWidth, int screenHeight) {
    width = screenWidth;
    height = screenHeight;
}

Matrix Camera::GetModelViewMatrix() const {
    return getXyz2uvwMatrix() * GetTranslationMatrix();
}
Matrix Camera::GetTranslationMatrix() const {
    return Matrix(1, 0, 0, -Eye[0],
                  0, 1, 0, -Eye[1],
                  0, 0, 1, -Eye[2],
                  0, 0, 0, 1);
}

void Camera::RotateV(double angle) {
    Rotate(Eye, v, angle);

    Vector w_prime = getUvw2xyzMatrix() * rotY_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * w;
    Vector u_prime = getUvw2xyzMatrix() * rotY_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * u;
    Vector up_prime = getUvw2xyzMatrix() * rotY_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * up;
    up = up_prime;
    w = w_prime;
    u = u_prime;
}

void Camera::RotateU(double angle) {
    Rotate(Eye, u, angle);

    Vector w_prime = getUvw2xyzMatrix() * rotX_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * w;
    Vector v_prime = getUvw2xyzMatrix() * rotX_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * v;
    Vector up_prime = getUvw2xyzMatrix() * rotX_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * up;
    up = up_prime;
    w = w_prime;
    v = v_prime;

}

void Camera::RotateW(double angle) {
    Rotate(Eye, w, angle);

    Vector u_prime = getUvw2xyzMatrix() * rotZ_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * u;
    Vector v_prime = getUvw2xyzMatrix() * rotZ_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * v;
    Vector up_prime = getUvw2xyzMatrix() * rotZ_mat(DEG_TO_RAD(angle))
                                                * getXyz2uvwMatrix() * up;
    up = up_prime;
    u = u_prime;
    v = v_prime;
}

void Camera::Translate(const Vector &v) {
    Matrix translate = trans_mat(-v);
    Eye = getUvw2xyzMatrix() * translate * getXyz2uvwMatrix() * Eye;
}

Matrix Camera::getXyz2uvwMatrix() const {
    return Matrix(u[0], u[1], u[2], 0,
                  v[0], v[1], v[2], 0,
                  w[0], w[1], w[2], 0,
                  0,    0,    0,    1);
}

Matrix Camera::getUvw2xyzMatrix() const {
    return Matrix(u[0], v[0], w[0], 0,
                  u[1], v[1], w[1], 0,
                  u[2], v[2], w[2], 0,
                  0,    0,    0,    1);
}


void Camera::Rotate(Point p, Vector axis, double degrees) {
    Matrix M1 = rotY_mat(atan(axis[2] / axis[0]));
    Vector axis_prime = M1 * axis;
    Matrix M2 = rotZ_mat(atan(axis_prime[1]/axis_prime[0]));
    Matrix M3 = rotX_mat(DEG_TO_RAD(degrees));
    Matrix M1_inv = rotY_mat(-atan(axis[2]/axis[0]));
    Matrix M2_inv = rotZ_mat(-atan(axis_prime[2]/axis_prime[0]));
    Point p_prime = M1_inv * M2_inv * M3 * M2 * M1 * p;
    p[0] = p_prime[0];
    p[1] = p_prime[1];
    p[2] = p_prime[2];
}


Point Camera::GetEyePoint() const {
    return Eye;
}

Vector Camera::GetLookVector() const {
    return -w;
}

Vector Camera::GetUpVector() const {
    return up;
}

double Camera::GetViewAngle() const {
    return Angle;
}

double Camera::GetNearPlane() const {
    return near;
}

double Camera::GetFarPlane() const {
    return far;
}

int Camera::GetScreenWidth() const {
    return width;
}

int Camera::GetScreenHeight() const {
    return height;
}

double Camera::GetFilmPlaneDepth() const {
    return 0;
}

double Camera::GetScreenWidthRatio() const {
    return width / height;
}

void Camera::Reset() {
    //Point eye = Point(2,2,2);
    //Vector look = Vector(-2,-2,-2);
    Point eye = Point(0.0, 0.0, 5.0);
    Vector look = Vector(0, 0, -1);
    Vector up = Vector(0,1,0);
    Orient(eye, look, up);

    SetNearPlane(0.001);
    SetFarPlane(30);
    SetViewAngle(45);
    SetScreenSize(500,500);
}

CameraMode Camera::getMode() const {
    return mode;
}
Camera &Camera::setMode(CameraMode m) {
    mode = m;
    return *this;
}

Vector Camera::getU() {
    return u;
}

Vector Camera::getV() {
    return v;
}
