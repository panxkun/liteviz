#ifndef __MESH_H__
#define __MESH_H__

#include <Eigen/Eigen>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <assimp/Importer.hpp>  
#include <assimp/scene.h>  
#include <assimp/postprocess.h>  
#include <liteviz/core/shader.h>
#include <liteviz/core/viewport.h>

#define COLOR_WHITE     Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f)
#define COLOR_RED       Eigen::Vector4f(1.0f, 0.0f, 0.0f, 1.0f)
#define COLOR_GREEN     Eigen::Vector4f(0.0f, 1.0f, 0.0f, 1.0f)
#define COLOR_BLUE      Eigen::Vector4f(0.0f, 0.0f, 1.0f, 1.0f)

#define COLOR_AXIS_X    Eigen::Vector4f(0.819, 0.219, 0.305, 0.5f) 
#define COLOR_AXIS_Y    Eigen::Vector4f(0.454, 0.674, 0.098, 0.5f) 
#define COLOR_AXIS_Z    Eigen::Vector4f(0.207, 0.403, 0.619, 0.5f)


class ImageTexture{
public:
    GLuint textureID = 0;
    Eigen::Vector2i textureSize = Eigen::Vector2i(0, 0); // w, h

    ImageTexture(){
        
        if (textureID != 0) {
            std::cerr << "Texture already initialized!" << std::endl;
            return;
        }

        glGenTextures(1, &textureID);
        if (textureID == 0) {
            std::cerr << "Failed to generate texture!" << std::endl;
            return;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        unsigned char blackData[3] = {0, 0, 0};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blackData);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL Error during texture initialization: " << err << std::endl;
            return;
        }

    }

    ~ImageTexture() {
        releaseBuffers();
    }

    void setup(unsigned char* data, Eigen::Vector2i size){
        if (size.x() != textureSize.x() || size.y() != textureSize.y()) {
            textureSize = size;
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize.x(), textureSize.y(), 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize.x(), textureSize.y(), GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        glBindTexture(GL_TEXTURE_2D, textureID);

    }

    void releaseBuffers(){
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }
    }

    GLuint getTextureID() const {
        return textureID;
    }
};

class Mesh
{
protected:
    std::vector<Eigen::Vector3f> positions;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector4f> colors;
    std::vector<Eigen::Vector2f> texture_coords;
    std::vector<GLuint> indices;

    Eigen::Matrix4f model_matrix;

public:

    std::string name = "mesh";

    Mesh(){}
    void setup(){}

    void clean(){
        positions.clear();
        colors.clear();
        indices.clear();
    }
    std::vector<Eigen::Vector3f> getPositions() const {return positions;}
    std::vector<Eigen::Vector3f> getNormals() const {return normals;}
    std::vector<Eigen::Vector4f> getColors() const {return colors;}
    std::vector<Eigen::Vector2f> getTexCoords() const {return texture_coords;}
    std::vector<GLuint> getIndices() const {return indices;}
    size_t getIndicesSize() const {return indices.size();}

    float scale = 1.0;
    void setScale(float scale){
        this->scale = scale;
    }

    void setPos(Eigen::Vector3f pos){
        this->model_matrix.block<3, 1>(0, 3) = pos;
    }

    virtual void setColor(Eigen::Vector4f color) {
        colors.clear();
        for(size_t i = 0; i < positions.size(); ++i){
            colors.push_back(color);
        }
    };

    Eigen::Matrix4f getModelMatrix(const float scale=1.0){
        Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
        matrix.block<3, 3>(0, 0) = model_matrix.block<3, 3>(0, 0);
        matrix.block<3, 1>(0, 3) = model_matrix.block<3, 1>(0, 3) * scale; // TODO: pxk: wried for scale
        return matrix;
    }

    void transform(Eigen::Matrix4f model_matrix){
        const auto& R = model_matrix.block<3, 3>(0, 0);
        const auto& t = model_matrix.block<3, 1>(0, 3);
        for(size_t i = 0; i < positions.size(); ++i){
            positions[i] = R * positions[i] + t;
        }
    }

    virtual void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport) = 0;
};

class Grid : public Mesh
{
public:

    void gen_grid_level(
        const Eigen::Vector3f& location, 
        const float& scale,
        const int level, 
        std::vector<Eigen::Vector3f>& grid_lines_x, 
        std::vector<Eigen::Vector3f>& grid_lines_y) const {

        const float step = pow(10, -level);
        const float gap = step * scale;
        const float size = 100.5;

        float x0 = location.x() * scale;
        float y0 = location.y() * scale;
        float z0 = location.z() * scale;

        int x_lo = (int)ceil((-size + x0) / gap);
        int x_hi = (int)floor((size + x0) / gap);
        int y_lo = (int)ceil((-size + y0) / gap);
        int y_hi = (int)floor((size + y0) / gap);

        grid_lines_x.clear();
        grid_lines_y.clear();

        for (int x = x_lo; x <= x_hi; ++x) {
            grid_lines_x.emplace_back(x * gap - x0, -size, -z0);
            grid_lines_x.emplace_back(x * gap - x0,  size, -z0);
        }
        for (int y = y_lo; y <= y_hi; ++y) {
            grid_lines_y.emplace_back(-size, y * gap - y0, -z0);
            grid_lines_y.emplace_back( size, y * gap - y0, -z0);
        }
    }

    void setup(const Eigen::Vector3f location = Eigen::Vector3f(0.0f, 0.0f, 0.0f), const float& scale = 1.0f)
    {   
        clean();

        const float level = log10f(scale * 5);

        std::vector<Eigen::Vector3f> grid_lines_x;
        std::vector<Eigen::Vector3f> grid_lines_y;

        gen_grid_level(location, scale, floor(level) - 1, grid_lines_x, grid_lines_y);

        for(size_t i = 0; i < grid_lines_x.size(); ++i){
            positions.push_back(grid_lines_x[i]);
            bool pivot = int(grid_lines_x[i].x() + location.x() * scale) == 0;
            colors.push_back(pivot ? COLOR_AXIS_Y : Eigen::Vector4f(0.5, 0.5, 0.5, 0.25));
        }

        for(size_t i = 0; i < grid_lines_y.size(); ++i){
            positions.push_back(grid_lines_y[i]);
            bool pivot = int(grid_lines_y[i].y() + location.y() * scale) == 0;
            colors.push_back(pivot ? COLOR_AXIS_X : Eigen::Vector4f(0.5, 0.5, 0.5, 0.25));
        }

        gen_grid_level(location, scale, floor(level), grid_lines_x, grid_lines_y);

        for(size_t i = 0; i < grid_lines_x.size(); ++i){
            positions.push_back(grid_lines_x[i]);
            colors.push_back(Eigen::Vector4f{0.5, 0.5, 0.5, float(pow(level - floor(level), 0.9) * 0.25)});
        }

        for(size_t i = 0; i < grid_lines_y.size(); ++i){
            positions.push_back(grid_lines_y[i]);
            colors.push_back(Eigen::Vector4f{0.5, 0.5, 0.5, float(pow(level - floor(level), 0.9) * 0.25)});
        }

        for (size_t i = 0; i < positions.size(); ++i){
            indices.push_back(i);
        }
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_LINES, 0, getIndicesSize());
        shader->unbind();
    }
};

class Cube : public Mesh{

public:
    void setup(const float scale=1.0f){

        clean();

        std::vector<Eigen::Vector3f> vs =  std::vector<Eigen::Vector3f>{
            Eigen::Vector3f(-0.5f, -0.5f, -0.5f) * scale,
            Eigen::Vector3f(-0.5f,  0.5f, -0.5f) * scale,
            Eigen::Vector3f( 0.5f,  0.5f, -0.5f) * scale,
            Eigen::Vector3f( 0.5f, -0.5f, -0.5f) * scale,
            Eigen::Vector3f(-0.5f, -0.5f,  0.5f) * scale,
            Eigen::Vector3f(-0.5f,  0.5f,  0.5f) * scale,
            Eigen::Vector3f( 0.5f,  0.5f,  0.5f) * scale,
            Eigen::Vector3f( 0.5f, -0.5f,  0.5f) * scale
        };

        Eigen::Vector3f pivot(0.0f, 0.0f, 0.0f);

        std::vector<Eigen::Vector3f> cube_vertices;
        for(size_t i = 0; i < vs.size(); ++i){
            cube_vertices.push_back((vs[i] + pivot));
        }

        indices = std::vector<GLuint>{
            0, 2, 1, 0, 2, 3, 0, 5, 1, 0, 5, 4, 0, 7, 3, 0, 7, 4,
            6, 4, 5, 6, 4, 7, 6, 3, 2, 6, 3, 7, 6, 1, 2, 6, 1, 5
        };

        std::vector<Eigen::Vector4f> cube_colors = std::vector<Eigen::Vector4f>{
            COLOR_RED, COLOR_BLUE, COLOR_GREEN,
            COLOR_RED, COLOR_BLUE, COLOR_GREEN
        };
        
        for (size_t i = 0; i < cube_colors.size(); ++i){
            for(size_t j = 0; j < 6; ++j){
                positions.push_back(cube_vertices[indices[6 * i + j]]);
                colors.push_back(cube_colors[i]);
            }
        }
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport) {
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->draw(GL_TRIANGLES, 0, getIndicesSize()); // TODO: use draw_indexed() will cause the cube error
        shader->unbind();
    }
};

class Frustum: public Mesh{

    std::vector<Eigen::Vector3f> triangle_positions;
    std::vector<Eigen::Vector4f> triangle_colors;
    std::vector<GLuint> triangle_indices;

    std::vector<Eigen::Vector3f> axis_positions;
    std::vector<Eigen::Vector4f> axis_colors;
    std::vector<GLuint> axis_indices;

public:
    void setup(const Eigen::Vector4f& intr, const float& scale=1.0f){
        clean();

        const float& fx = intr.x();
        const float& cx = intr.z();
        const float& cy = intr.w();

        positions =  std::vector<Eigen::Vector3f>{
            Eigen::Vector3f( 0.0f, 0.0f, 0.0f) * scale,
            Eigen::Vector3f( cx,  cy, fx) * scale,
            Eigen::Vector3f(-cx,  cy, fx) * scale,
            Eigen::Vector3f(-cx, -cy, fx) * scale,
            Eigen::Vector3f( cx, -cy, fx) * scale,
        };

        triangle_positions = std::vector<Eigen::Vector3f>{
            Eigen::Vector3f( cx/3, -cy*1.05, fx) * scale,
            Eigen::Vector3f(-cx/3, -cy*1.05, fx) * scale,
            Eigen::Vector3f( 0, -cy*1.25, fx) * scale,
        };

        indices = std::vector<GLuint>{
            0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3, 3, 4, 4, 1
        };

        triangle_indices = std::vector<GLuint>{0, 1, 2};

        for (size_t i = 0; i < positions.size(); ++i){
            colors.push_back(Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
        }

        for (size_t i = 0; i < triangle_positions.size(); i++)
        {
            triangle_colors.push_back(Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
        }

        axis_positions = std::vector<Eigen::Vector3f>{
            Eigen::Vector3f( 0.0f, 0.0f, 0.0f) * cx * scale,
            Eigen::Vector3f( 1.0f, 0.0f, 0.0f) * cx * scale,
            Eigen::Vector3f( 0.0f, 0.0f, 0.0f) * cx * scale,
            Eigen::Vector3f( 0.0f, 1.0f, 0.0f) * cx * scale,
            Eigen::Vector3f( 0.0f, 0.0f, 0.0f) * cx * scale,
            Eigen::Vector3f( 0.0f, 0.0f, 1.0f) * cx * scale,
        };

        axis_colors = std::vector<Eigen::Vector4f>{
            COLOR_AXIS_X, COLOR_AXIS_X, 
            COLOR_AXIS_Y, COLOR_AXIS_Y, 
            COLOR_AXIS_Z, COLOR_AXIS_Z
        };

        axis_indices = std::vector<GLuint>{0, 1, 2, 3, 4, 5};
    }

    void setColor(Eigen::Vector4f color) {
        colors.clear();
        for(size_t i = 0; i < positions.size(); ++i){
            colors.push_back(color);
        }
        triangle_colors.clear();
        for(size_t i = 0; i < triangle_positions.size(); ++i){
            triangle_colors.push_back(color);
        }
    };

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_LINES, 0, getIndicesSize());
        shader->unbind();

        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_attribute("Color", triangle_colors);
        shader->set_attribute("Position", triangle_positions);
        shader->set_indices(triangle_indices);
        shader->draw_indexed(GL_TRIANGLES, 0, triangle_indices.size());
        shader->unbind();

        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_attribute("Color", axis_colors);
        shader->set_attribute("Position", axis_positions);
        shader->set_indices(axis_indices);
        shader->draw_indexed(GL_LINES, 0, axis_indices.size());
        shader->unbind();
    }

    void transform(Eigen::Matrix4f model_matrix){
        const auto& R = model_matrix.block<3, 3>(0, 0);
        const auto& t = model_matrix.block<3, 1>(0, 3);
        for(size_t i = 0; i < positions.size(); ++i){
            positions[i] = R * positions[i] + t;
        }

        for(size_t i = 0; i < triangle_positions.size(); ++i){
            triangle_positions[i] = R * triangle_positions[i] + t;
        }

        for(size_t i = 0; i < axis_positions.size(); ++i){
            axis_positions[i] = R * axis_positions[i] + t;
        }
    }

};

class Plane: public Mesh{
public:
    void setup(const Eigen::Vector4f& K, const float& scale=1.0f){
        
        clean();

        const float& fx = K.x();
        const float& cx = K.z();
        const float& cy = K.w();

        positions = std::vector<Eigen::Vector3f>{
            Eigen::Vector3f( cx,  cy, fx) * scale,
            Eigen::Vector3f(-cx,  cy, fx) * scale,
            Eigen::Vector3f(-cx, -cy, fx) * scale,
            Eigen::Vector3f( cx, -cy, fx) * scale,
            Eigen::Vector3f( cx/3, cy*1.05, fx) * scale,
            Eigen::Vector3f(-cx/3, cy*1.05, fx) * scale,
            Eigen::Vector3f( 0, cy*1.25, fx) * scale,
        };

        indices = std::vector<GLuint>{
            0, 2, 1, 0, 2, 3, 4, 5, 6
        };

        for (size_t i = 0; i < positions.size(); ++i){
            colors.push_back(Eigen::Vector4f(1.0f, 0.0f, 0.0f, 0.2f));
        }
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_TRIANGLES, 0, getIndicesSize());
        shader->unbind();
    }
};

class FovPoly: public Mesh{
public:
    void setup(const Eigen::Vector4f& K, const float& scale=1.0f){
        
        clean();

        const float& fx = K.x();
        const float& cx = K.z();
        const float& cy = K.w();

        float far = 1000;

        positions =  std::vector<Eigen::Vector3f>{
            Eigen::Vector3f( cx,  0, fx) * scale,
            Eigen::Vector3f(-cx,  0, fx) * scale,
            Eigen::Vector3f( cx,  0, fx) * scale * far,
            Eigen::Vector3f(-cx,  0, fx) * scale * far,
        };

        indices = std::vector<GLuint>{
            0, 1, 2, 1, 2, 3
        };

        for (size_t i = 0; i < positions.size(); ++i){
            colors.push_back(Eigen::Vector4f(1.0f, 0.0f, 0.0f, 0.2f));
        }

    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 0.2f);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_TRIANGLES, 0, getIndicesSize());
        shader->unbind();
    }
};

class Line: public Mesh{
public:
    void setup(std::vector<Eigen::Vector3f>& pos, std::vector<Eigen::Vector4f>& c){
        clean();
        if(pos.size() <= 1)
            return;

        positions = pos;
        colors = c;
        for(size_t i = 0; i < positions.size(); ++i){
            indices.push_back(i);
        }
    }

    void setup(std::vector<Eigen::Vector3f>& pos, Eigen::Vector4f& c){
        clean();
        if(pos.size() <= 1)
            return;
        positions = pos;
        for(size_t i = 0; i < positions.size(); ++i){
            colors.push_back(c);
            indices.push_back(i);
        }
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_LINES, 0, getIndicesSize());
        shader->unbind();
    }
};

// class ImageTex: public Mesh{

// public: 
//     std::unique_ptr<ImageTexture> texture;

//     ImageTex(){
//         texture = std::make_unique<ImageTexture>();
//     }

//     void releaseBuffers(){
//         texture->releaseBuffers();
//     }

//     GLuint getTextureID() const {
//         return texture->getTextureID();
//     }

//     void setup(const cv::Mat& image, const Eigen::Vector4f& K, const float& scale=1.0f){

//         clean();

//         texture->setup(image);

//         const float& fx = K.x();
//         const float& cx = K.z();
//         const float& cy = K.w();

//         positions =  std::vector<Eigen::Vector3f>{
//             Eigen::Vector3f( cx,  cy, fx) * scale,
//             Eigen::Vector3f(-cx,  cy, fx) * scale,
//             Eigen::Vector3f(-cx, -cy, fx) * scale,
//             Eigen::Vector3f( cx, -cy, fx) * scale,
//         };

//         texture_coords = std::vector<Eigen::Vector2f>{
//             Eigen::Vector2f( 1.0f, 1.0f), 
//             Eigen::Vector2f( 1.0f, 0.0f), 
//             Eigen::Vector2f( 0.0f, 0.0f), 
//             Eigen::Vector2f( 0.0f, 1.0f), 
//         };

//         indices = std::vector<GLuint>{
//             0, 2, 1, 0, 2, 3
//         };
//     }

//     void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
//         shader->bind();
//         shader->set_uniform("ProjMat", transform);
//         glActiveTexture(GL_TEXTURE0);
//         glBindTexture(GL_TEXTURE_2D, getTextureID());
//         shader->set_uniform("Texture");
//         shader->set_attribute("TexCoords", getTexCoords());
//         shader->set_attribute("Position", getPositions());
//         shader->set_indices(getIndices());
//         shader->draw_indexed(GL_TRIANGLES, 0, getIndicesSize());
//         shader->unbind();
//         releaseBuffers();
//     }

// };

class CoordinateFrame: public Mesh{
public:
    void setup(
        const float& scale=1.0f,
        const Eigen::Vector3f x = Eigen::Vector3f(1.0f, 0.0f, 0.0f),
        const Eigen::Vector3f y = Eigen::Vector3f(0.0f, 1.0f, 0.0f),
        const Eigen::Vector3f z = Eigen::Vector3f(0.0f, 0.0f, 1.0f)
        )
    {
        Eigen::Vector3f o = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
        positions =  std::vector<Eigen::Vector3f>{
            o * scale, x * scale,
            o * scale, y * scale,
            o * scale, z * scale,
        };

        colors = std::vector<Eigen::Vector4f>{
            COLOR_AXIS_X, COLOR_AXIS_X,
            COLOR_AXIS_Y, COLOR_AXIS_Y,
            COLOR_AXIS_Z, COLOR_AXIS_Z
        };

        indices = std::vector<GLuint>{0, 1, 2, 3, 4, 5};
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_LINES, 0, getIndicesSize());
        shader->unbind();
    }
};

class ObjectMesh: public Mesh{
public:

    void setup(const std::string path, Eigen::Matrix4f model_matrix=Eigen::Matrix4f::Identity(), const float scale=1.f){

        clean();

        Assimp::Importer importer;  

        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);  
        
        if (!scene) {  
            std::cerr << "failed to load .obj file!" << std::endl;
            exit(-1);  
        }

        // TODO: now default only one mesh node 
        aiMesh* mesh = scene->mMeshes[0];  
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {  
            aiVector3D vertex = mesh->mVertices[i];  
            positions.emplace_back(vertex.x * scale, vertex.y * scale, vertex.z * scale);
            colors.push_back(Eigen::Vector4f(0.788, 0.329, 0.200, 1.00));
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {  
            aiFace face = mesh->mFaces[j];  
            for (unsigned int k = 0; k < face.mNumIndices; k++) {  
                unsigned int vertexIndex = face.mIndices[k];
                indices.push_back(vertexIndex);
            }  
        }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {  
            aiVector3D normal = mesh->mNormals[i];  
            normals.emplace_back(normal.x, normal.y ,normal.z);
        }  

        this->set_model_matrix(model_matrix);
    }

    void set_model_matrix(Eigen::Matrix4f model_matrix){
        Eigen::Matrix3f R = Eigen::AngleAxisf(90 / 180.0f * M_PI, Eigen::Vector3f::UnitX()).matrix();
        R = Eigen::AngleAxisf(0 / 180.0f * M_PI, Eigen::Vector3f::UnitY()).matrix() * R;
        R = Eigen::AngleAxisf(-90.0 / 180.0f * M_PI, Eigen::Vector3f::UnitZ()).matrix() * R;
        Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
        m.block<3, 3>(0, 0) = R;
        this->model_matrix = model_matrix * m;
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport) {

        std::vector<Eigen::Vector3f> _positions;
        for(auto p: this->positions){
            _positions.push_back(p * this->scale);
        }

        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", _positions);
        shader->set_attribute("Normal", getNormals());
        shader->draw(GL_TRIANGLES, 0, getIndicesSize()); // TODO: use draw_indexed() will cause the cube error
        shader->unbind();
    }
};

class PointCloud: public Mesh{
public:
    void setup(const int& num=10000)
    {
        clean();
        srand(static_cast<unsigned int>(time(NULL)));

        for (int i = 0; i < num; ++i) {
            Eigen::Vector3f point;
            point.x() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            point.y() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            point.z() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            positions.push_back(point);

            Eigen::Vector4f color;
            color.x() = static_cast<float>(rand()) / RAND_MAX;
            color.y() = static_cast<float>(rand()) / RAND_MAX;
            color.z() = static_cast<float>(rand()) / RAND_MAX;
            color.w() = 1;
            colors.push_back(color);

            indices.push_back(i);
        }
    }

    void setup(const std::vector<Eigen::Vector3f>& pc, const Eigen::Vector4f color){
        clean();
        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i]);
            colors.push_back(color);
            indices.push_back(positions.size() - 1);
        }
    }
    
    void setup(const std::vector<Eigen::Vector3d>& pc, const Eigen::Vector4f color){
        clean();
        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i].cast<float>());
            colors.push_back(color);
            indices.push_back(positions.size() - 1);
        }
    }

    void add_points(const std::vector<Eigen::Vector3f>& pc, const Eigen::Vector4f color){

        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i]);
            colors.push_back(color);
            indices.push_back(positions.size() - 1);
        }
    }

    void add_points(const std::vector<Eigen::Vector3f>& pc, const std::vector<Eigen::Vector4f>& cs){

        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i]);
            colors.push_back(cs[i]);
            indices.push_back(positions.size() - 1);
        }
    }

    void setPointSize(const int size){
        point_size = size;
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("PointSize", point_size);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_POINTS, 0, getIndicesSize());
        shader->unbind();
    }

private:
    int point_size = 1;
};

class Surfel: public Mesh{
public:
    void setup(const int& num=10000)
    {
        clean();
        srand(static_cast<unsigned int>(time(NULL)));

        for (int i = 0; i < num; ++i) {
            Eigen::Vector3f point;
            point.x() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            point.y() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            point.z() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            positions.push_back(point);

            Eigen::Vector4f color;
            color.x() = static_cast<float>(rand()) / RAND_MAX;
            color.y() = static_cast<float>(rand()) / RAND_MAX;
            color.z() = static_cast<float>(rand()) / RAND_MAX;
            color.w() = 1;
            colors.push_back(color);

            Eigen::Vector3f normal;
            normal.x() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            normal.y() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            normal.z() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            normal = normal.normalized();
            normals.push_back(normal);

            Eigen::Vector3f scale;
            scale.x() = static_cast<float>(rand()) / RAND_MAX;
            scale.y() = static_cast<float>(rand()) / RAND_MAX;
            scale.z() = 0;
            scales.push_back(scale);

            indices.push_back(i);
        }
    }

    void draw(const std::shared_ptr<Shader>& shader, const Eigen::Matrix4f& transform, const Viewport& viewport){
        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_attribute("Normal", normals);
        shader->set_attribute("Scale", scales);
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_POINTS, 0, getIndicesSize());
        shader->unbind();
    }

    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector3f> scales;
};

#endif
