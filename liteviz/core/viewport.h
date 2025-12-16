#ifndef __LITEVIZ_VIEWPORT_H__
#define __LITEVIZ_VIEWPORT_H__

#include <liteviz/core/common.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif // __LITEVIZ_VIEWPORT_H__

namespace liteviz {

class Viewport {

class CameraMotion{

public:
    CameraMotion() = default;

    CameraMotion(Viewport* viewport): 
        viewport(viewport), 
        transform(mat4f::Identity()),
        deltaTransform(mat4f::Identity()),
        last_z(0.80f),
        intersection_center(vec3f::Zero()){}

    void rotate(const vec2f& pos){
        vec2f offset = (pos - prevPos);
        float rf = 0.005f;

        mat4f T1 = mat4f::Identity();
        mat3f Rx = Eigen::AngleAxisf(offset.y() * rf, vec3f::UnitX()).toRotationMatrix();
        mat3f Ry = Eigen::AngleAxisf(offset.x() * rf, vec3f::UnitY()).toRotationMatrix();
        T1.block<3, 3>(0, 0) = Rx * Ry;
        T1.block<3, 1>(0, 3) = T1.block<3, 3>(0, 0) * -intersection_center;

        mat4f T2 = mat4f::Identity();
        T2.block<3, 1>(0, 3) = intersection_center;

        deltaTransform = T2 * T1;

        transform = transform * deltaTransform.inverse();
        deltaTransform = mat4f::Identity();
        prevPos = pos;
    }

    void translate(const vec2f& pos){
        vec2f offset = (pos - prevPos);
        float tf = 0.1;
        if(last_z == 1){
            deltaTransform.block<3, 1>(0, 3) = vec3f(offset.x() * tf, -offset.y() * tf, 0);
        }else{
            vec3f Pw;
            vec3f Pc;
            viewport->pixelUnproject(pos, last_z, Pw, Pc);
            float deltaX = Pc.x() - intersection_center.x();
            float deltaY = Pc.y() - intersection_center.y();
            deltaTransform.block<3, 1>(0, 3) = vec3f(deltaX, deltaY, 0);
            intersection_center = Pc;
        }

        transform = transform * deltaTransform.inverse();
        deltaTransform = mat4f::Identity();
        prevPos = pos;
    }

    void zoom(float delta){

        deltaTransform(2, 3) = delta;

        transform = transform * deltaTransform.inverse();
        deltaTransform = mat4f::Identity();
    }

    void follow(const mat4f& target_transform){
        mat4f deltaTransform = target_transform * prev_target_transform.inverse();
        transform = deltaTransform * transform;
        prev_target_transform = target_transform;
    }

    void initScreenPos(const vec2f& pos){

        float zNDC;
        vec3f pc;
        vec3f pw;
        viewport->getPixelPosition(pos, zNDC, pw, pc, last_z);

        if(zNDC != 1){
            last_z = zNDC;
            intersection_center = pc;
        }
        prevPos = pos;
    }

    void initTargetTransform(const mat4f& target_transform){
        prev_target_transform = target_transform;
    }

    void initTransformation(const mat4f& transform){
        this->transform = transform;
    }

    mat3f getRotation() const{
        return transform.block<3, 3>(0, 0);
    }

    vec3f getPosition() const{
        return transform.block<3, 1>(0, 3);
    }

    mat4f getTransformation() const{
        return transform;
    }

private:
    float last_z;
    vec3f intersection_center;
    mat4f transform;      // camera to world
    mat4f deltaTransform;
    vec2f prevPos;
    Viewport* viewport = nullptr;

    mat4f prev_target_transform;
};

public:
    Eigen::Vector2i windowSize;
    Eigen::Vector2i frameBufferSize;
    mat4f openGLTransform;

    bool is_following = false;

    float zNear = 0.2f;
    float zFar = 100.0f;
    float fov = 60.0f;

    CameraMotion camera;

    Viewport(
        size_t width    = 1280,
        size_t height   = 720,
        vec3f eye       = vec3f(2, 2, 2), 
        vec3f center    = vec3f(0, 0, 0), 
        vec3f up        = vec3f(0, 0, 1)) // up usually be set to (0, 1, 0)
    {
        windowSize = Eigen::Vector2i(width, height);
        vec3f zAxis = (eye - center).normalized();
        vec3f xAxis = up.cross(zAxis).normalized();
        vec3f yAxis = zAxis.cross(xAxis).normalized();

        mat3f R = mat3f::Identity();
        R.col(0) = xAxis;
        R.col(1) = yAxis;
        R.col(2) = zAxis;
        
        mat4f transform = mat4f::Identity();
        transform.block<3, 3>(0, 0) = R;
        transform.block<3, 1>(0, 3) = eye;

        camera = CameraMotion(this);
        camera.initTransformation(transform);

        openGLTransform = mat4f::Identity();
        openGLTransform(1, 1) = -1;
        openGLTransform(2, 2) = -1;
    }

    void follow(const mat4f& target_transform, bool is_follow){

        if(is_follow){
            if(!is_following){
                camera.initTargetTransform(target_transform);
            }
            is_following = true;
            camera.follow(target_transform);
        }else{
            is_following = false;
        }
    }

    float getFocal() const {
        return windowSize.y() / (tan((fov / 180.0f * M_PI) / 2.0f) * 2.0f);
    }

    vec2f getTanXY() const {
        float tanHalfFov = tan((fov / 180.0 * M_PI) / 2.0f);
        float aspect = static_cast<float>(frameBufferSize.x()) / frameBufferSize.y();
        return vec2f(aspect * tanHalfFov, tanHalfFov);
    }

    vec3f getCameraPosition() const {
        return camera.getPosition();
    }

    mat3f getCameraRotation() const {
        return camera.getRotation();
    }

    Eigen::Vector2i getFrameBufferSize() const {
        return frameBufferSize;
    }

    void getPixelPosition(const vec2f& pos, float& zNDC, vec3f& Pw, vec3f& Pc, float default_z){
        Eigen::Vector2i pick_pos = {int(pos.x()), int(windowSize.y() - pos.y())};
        glReadPixels(pick_pos.x(), pick_pos.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zNDC);

        if(zNDC == 1){
            zNDC = default_z;
        }

        pixelUnproject(pos, zNDC, Pw, Pc);
    }

    void pixelUnproject(const vec2f& pos, const float& zNDC, vec3f& Pw, vec3f& Pc){
        vec3d Pc_;
        mat4d Identity = mat4d::Identity();
        mat4d Projmatrix = getProjectionMatrix().cast<double>();
        Eigen::Vector4i Viewport = Eigen::Vector4i(0, 0, windowSize.x(), windowSize.y());
        Eigen::Vector2i pick_pos = {int(pos.x()), int(windowSize.y() - pos.y())};

        // Replace deprecated gluUnProject with local implementation using Eigen
        double objx = 0.0, objy = 0.0, objz = 0.0;
        bool ok = unProject(static_cast<double>(pick_pos.x()), static_cast<double>(pick_pos.y()), static_cast<double>(zNDC), Identity.data(), Projmatrix.data(), Viewport.data(), &objx, &objy, &objz);
        if (ok) {
            Pc_ = vec3d(objx, objy, objz);
        } else {
            // fallback if inversion fails
            Pc_ = vec3d::Zero();
        }

        Pc = Pc_.cast<float>();
        Pw = camera.getRotation() * Pc + camera.getPosition();
    }

    // Replacement implementation of gluUnProject using Eigen.
    // Returns true on success and writes object coordinates to objx/objy/objz.
    static bool unProject(double winx, double winy, double winz,
                          const double model[16], const double proj[16], const int viewport[4],
                          double* objx, double* objy, double* objz) {
        Eigen::Map<const mat4d> M(model);
        Eigen::Map<const mat4d> P(proj);

        mat4d A = P * M;

        // quick singular check
        if (std::abs(A.determinant()) < 1e-12) return false;
        mat4d inv = A.inverse();

        // map window coordinates to NDC
        double x = (winx - viewport[0]) / static_cast<double>(viewport[2]) * 2.0 - 1.0;
        double y = (winy - viewport[1]) / static_cast<double>(viewport[3]) * 2.0 - 1.0;
        double z = 2.0 * winz - 1.0;

        vec4d in(x, y, z, 1.0);
        vec4d out = inv * in;
        if (std::abs(out(3)) < 1e-12) return false;
        out /= out(3);

        *objx = out(0);
        *objy = out(1);
        *objz = out(2);
        return true;
    }

    mat4f getViewMatrix() const{
        return camera.getTransformation().inverse();
    }

    mat4f getProjectionMatrix() const {

        float tanHalfFov = tan((fov / 180.0 * M_PI) / 2.0f);
        float aspect = static_cast<float>(frameBufferSize.x()) / frameBufferSize.y();

        mat4f projmatrix = mat4f::Zero();
        projmatrix(0, 0) = 1.0f / (aspect * tanHalfFov);
        projmatrix(1, 1) = 1.0f / tanHalfFov;
        projmatrix(2, 2) = -(zFar + zNear) / (zFar - zNear);
        projmatrix(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
        projmatrix(3, 2) = -1.0f; 

        return projmatrix;
    }

    void setFoV(float fov){
        this->fov = fov;
    }

    void setViewMatrix(mat4f viewmat){
        camera.initTransformation(viewmat * openGLTransform);
    }

    void setProjectionMatrix(float zNear, float zFar, float fov){
        this->zNear = zNear;
        this->zFar = zFar;
        this->fov = fov;
    }
};

} // namespace liteviz

#endif