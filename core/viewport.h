#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include <Eigen/Eigen>
#include <Eigen/Dense>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>


class DualQuaternion {
public:
    DualQuaternion() : real(Eigen::Quaternionf::Identity()), dual(Eigen::Quaternionf(0, 0, 0, 0)) {}

    DualQuaternion(const Eigen::Quaternionf& rotation, const Eigen::Vector3f& translation) {
        real = rotation;
        dual = Eigen::Quaternionf(
            0.5f * translation.x(), 
            0.5f * translation.y(), 
            0.5f * translation.z(),
            0.0f) * real;
    }

    DualQuaternion(const Eigen::Quaternionf& rotation) {
        real = rotation;
        dual = Eigen::Quaternionf(0, 0, 0, 0);
    }

    static DualQuaternion translation(const Eigen::Vector3f& translation) {
        return DualQuaternion(Eigen::Quaternionf::Identity(), translation);
    }

    DualQuaternion operator+(const DualQuaternion& other) const {
        Eigen::Quaternionf real_sum(
            real.x() + other.real.x(),
            real.y() + other.real.y(),
            real.z() + other.real.z(),
            real.w() + other.real.w()
        );

        Eigen::Quaternionf dual_sum(
            dual.x() + other.dual.x(),
            dual.y() + other.dual.y(),
            dual.z() + other.dual.z(),
            dual.w() + other.dual.w()
        );

        return DualQuaternion(real_sum, dual_sum);
    }

    DualQuaternion operator*(const DualQuaternion& other) const {
        Eigen::Quaternionf real_part = real * other.real;
        Eigen::Quaternionf dual_part = real * other.dual;
        dual_part.coeffs() += (dual * other.real).coeffs();

        dual_part = Eigen::Quaternionf(
            real_part.x() + dual_part.x(),
            real_part.y() + dual_part.y(),
            real_part.z() + dual_part.z(),
            real_part.w() + dual_part.w()
        );

        return DualQuaternion(real_part, dual_part);
    }

    DualQuaternion operator*(float scalar) const {
        Eigen::Quaternionf real_scaled(
            real.x() * scalar,
            real.y() * scalar,
            real.z() * scalar,
            real.w() * scalar
        );

        Eigen::Quaternionf dual_scaled(
            dual.x() * scalar,
            dual.y() * scalar,
            dual.z() * scalar,
            dual.w() * scalar
        );

        return DualQuaternion(real_scaled, dual_scaled);
    }

    static DualQuaternion Identity() {
        return DualQuaternion();
    }

    void normalize() {
        float norm = real.norm();
        real.normalize();
        dual = Eigen::Quaternionf(
            dual.x() / norm,
            dual.y() / norm,
            dual.z() / norm,
            dual.w() / norm
        );
    }

    DualQuaternion conjugate() const {
        return DualQuaternion(real.conjugate(), dual.conjugate());
    }

    DualQuaternion inverted() const {
        DualQuaternion conjugate = this->conjugate();
        return conjugate * (1.0f / (real.w() * real.w() + real.x() * real.x() + real.y() * real.y() + real.z() * real.z()));
    }

    Eigen::Matrix3f getRotation() const {
        return real.toRotationMatrix();
    }

    Eigen::Vector3f getPosition() const {
        Eigen::Quaternionf realConjugate = real.conjugate();
        Eigen::Quaternionf transQuat = Eigen::Quaternionf(
            dual.x() * 2.0f,
            dual.y() * 2.0f,
            dual.z() * 2.0f,
            dual.w() * 2.0f
        ) * realConjugate;
        return Eigen::Vector3f(transQuat.x(), transQuat.y(), transQuat.z());
    }

private:
    Eigen::Quaternionf real;
    Eigen::Quaternionf dual;

    DualQuaternion(const Eigen::Quaternionf& r, const Eigen::Quaternionf& d) : real(r), dual(d) {}
};

class Viewport {

class CameraMotion{

public:
    CameraMotion() = default;

    CameraMotion(Viewport* viewport): 
        viewport(viewport), 
        transform(Eigen::Matrix4f::Identity()),
        deltaTransform(Eigen::Matrix4f::Identity()),
        last_z(1.0f),
        intersection_center(Eigen::Vector3f::Zero()){}

    void rotate(const Eigen::Vector2f& pos){
        Eigen::Vector2f currPosNdc = screenCoordToNdc(pos);
        Eigen::Vector2f offset = (currPosNdc - prevPosNdc);

        prevPosNdc = currPosNdc;
    }

    void translate(const Eigen::Vector2f& pos){
        Eigen::Vector2f currPosNdc = screenCoordToNdc(pos);
        Eigen::Vector2f offset = (currPosNdc - prevPosNdc);


        if(last_z == 1){
            deltaTransform.block<3, 1>(0, 3) = Eigen::Vector3f(offset.x(), offset.y(), 0);
        }else{
            Eigen::Vector3f Pw;
            Eigen::Vector3f Pc;
            viewport->pixelUnproject(pos, last_z, Pw, Pc);
            float deltaX = Pc.x() - intersection_center.x();
            float deltaY = Pc.y() - intersection_center.y();
            deltaTransform.block<3, 1>(0, 3) = Eigen::Vector3f(deltaX * 5, deltaY * 5, 0);
            intersection_center = Pc;
        }

        // float zNDC;
        // Eigen::Vector3f pc;
        // Eigen::Vector3f pw;
        // viewport->getPixelPosition(pos, zNDC, pw, pc);
        // std::cout << "pw: " << pw.transpose() << " pc: " << pc.transpose() << std::endl;

        prevPosNdc = currPosNdc;
    }

    void zoom(float delta){
        float scale = std::exp(-delta * 0.1f);
        deltaTransform.block<3, 1>(0, 3).z() = scale - 1.0f;
    }

    Eigen::Vector2f screenCoordToNdc(const Eigen::Vector2f& pos) const{
        const float w = viewport->window_size.x();
        const float h = viewport->window_size.y();
        return Eigen::Vector2f(2.0f * pos.x() / w - 1.0f, 1.0f - 2.0f * pos.y() / h);
    }

    void initScreenPos(const Eigen::Vector2f& pos){
        prevPosNdc = screenCoordToNdc(pos);

        float zNDC;
        Eigen::Vector3f pc;
        Eigen::Vector3f pw;
        viewport->getPixelPosition(pos, zNDC, pw, pc);

        if(zNDC != 1){
            last_z = zNDC;
            intersection_center = pc;
        }
    }

    void initTransformation(const Eigen::Matrix4f& transform){
        this->transform = transform;
    }

    void updateTransformation(){
        transform = transform * deltaTransform.inverse();
        deltaTransform = Eigen::Matrix4f::Identity();
    }

    Eigen::Matrix3f getRotation() const{
        return transform.block<3, 3>(0, 0);
    }

    Eigen::Vector3f getPosition() const{
        return transform.block<3, 1>(0, 3);
    }

    Eigen::Matrix4f getTransformation() const{
        return transform;
    }

private:
    float last_z;
    Eigen::Vector3f intersection_center;
    Eigen::Matrix4f transform;      // camera to world
    Eigen::Matrix4f deltaTransform;
    Eigen::Vector2f prevPosNdc;
    Viewport* viewport = nullptr;
};

public:
    Eigen::Vector2i window_size;
    Eigen::Vector2i framebuffer_size;

    bool follow_camera = false;
    Eigen::Matrix4f followed_projmatrix = Eigen::Matrix4f::Identity();

    float zNear = 1.0e-1;
    float zFar = 1.0e2;
    float fov = 45.0f;

    CameraMotion camera;

    Viewport(
        size_t width            = 1280,
        size_t height           = 720,
        Eigen::Vector3f eye     = Eigen::Vector3f(1, 1, 1), 
        Eigen::Vector3f center  = Eigen::Vector3f(0, 0, 0), 
        Eigen::Vector3f up      = Eigen::Vector3f(0, 0, 1)) // up usually be set to (0, 1, 0)
    {
        window_size = Eigen::Vector2i(width, height);
        Eigen::Vector3f zAxis = (center - eye).normalized();
        Eigen::Vector3f xAxis = up.cross(zAxis).normalized();
        Eigen::Vector3f yAxis = zAxis.cross(xAxis).normalized();

        Eigen::Matrix3f R = Eigen::Matrix3f::Identity();
        R.col(0) = xAxis;
        R.col(1) = yAxis;
        R.col(2) = -zAxis;
        
        Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
        transform.block<3, 3>(0, 0) = R;
        transform.block<3, 1>(0, 3) = (eye - center) * 3.0f + center;

        camera = CameraMotion(this);
        camera.initTransformation(transform);
    }

    void getPixelPosition(const Eigen::Vector2f& pos, float& zNDC, Eigen::Vector3f& Pw, Eigen::Vector3f& Pc){
        Eigen::Vector2i pick_pos = {int(pos.x()), int(window_size.y() - pos.y())};
        glReadPixels(pick_pos.x(), pick_pos.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zNDC);
        pixelUnproject(pos, zNDC, Pw, Pc);
    }

    void pixelUnproject(const Eigen::Vector2f& pos, const float& zNDC, Eigen::Vector3f& Pw, Eigen::Vector3f& Pc){
        Eigen::Vector3d Pc_;
        Eigen::Matrix4d Identity = Eigen::Matrix4d::Identity();
        Eigen::Matrix4d Projmatrix = getProjectionMatrix().cast<double>();
        Eigen::Vector4i Viewport = Eigen::Vector4i(0, 0, window_size.x(), window_size.y());
        Eigen::Vector2i pick_pos = {int(pos.x()), int(window_size.y() - pos.y())};
        gluUnProject(pick_pos.x(), pick_pos.y(), zNDC, Identity.data(), Projmatrix.data(), Viewport.data(), &Pc_.x(), &Pc_.y(), &Pc_.z());
        Pc = Pc_.cast<float>();
        Pw = camera.getRotation() * Pc + camera.getPosition();
    }

    Eigen::Matrix4f getViewMatrix(){

        if(follow_camera) 
            return followed_projmatrix;

        camera.updateTransformation();

        return camera.getTransformation().inverse();
    }

    Eigen::Matrix4f getProjectionMatrix() const {

        float tanHalfFov = tan((fov / 180.0 * M_PI) / 2.0f);
        float aspect = static_cast<float>(framebuffer_size.x()) / framebuffer_size.y();

        Eigen::Matrix4f projmatrix = Eigen::Matrix4f::Zero();
        projmatrix(0, 0) = 1.0f / (aspect * tanHalfFov);
        projmatrix(1, 1) = 1.0f / tanHalfFov;
        projmatrix(2, 2) = -(zFar + zNear) / (zFar - zNear);
        projmatrix(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
        projmatrix(3, 2) = -1.0f; 

        return projmatrix;
    }

    void setViewMatrix(Eigen::Matrix4f transform, float dist=5){
        follow_camera = true;
        auto R = transform.block<3, 3>(0, 0);
        auto t = transform.block<3, 1>(0, 3);
        t = (t - dist * R.col(2));

        auto Rinv = R.transpose();
        auto tinv = -Rinv * t;

        followed_projmatrix.block<3, 3>(0, 0) = Rinv;
        followed_projmatrix.block<3, 1>(0, 3) = tinv;
    }

    void reset(){
        follow_camera = false;
    }
};

#endif