#ifndef __LITEVIZ_MESH_H__
#define __LITEVIZ_MESH_H__

#include <liteviz/core/common.h>
#include <liteviz/core/shader.h>
#include <liteviz/core/viewport.h>

namespace liteviz {

class ImageTexture{
public:

    ImageTexture(GLuint internalformat, GLuint format, GLuint type): textureID(0), textureSize(0, 0), internalformat(internalformat), format(format), type(type)
    {
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

        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, 1, 1, 0, format, type, nullptr);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL Error during texture initialization: " << err << std::endl;
            return;
        }
    }

    ~ImageTexture() {
        releaseBuffers();
    }

    void setup(void* data, vec2i size){
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        if (size.x() != textureSize.x() || size.y() != textureSize.y()) {
            textureSize = size;
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, textureSize.x(), textureSize.y(), 0, format, type, data);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize.x(), textureSize.y(), format, type, data);
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

public:
    GLuint textureID;
    GLuint internalformat;
    GLuint format;
    GLuint type;
    vec2i textureSize;

};


class Mesh{

protected:
    std::vector<vec3f> positions;
    std::vector<vec3f> normals;
    std::vector<vec4f> colors;
    std::vector<vec2f> texture_coords;
    std::vector<GLuint> indices;

    mat4f model_matrix;

public:

    Mesh(){}
    void setup(){}

    void clean(){
        positions.clear();
        colors.clear();
        indices.clear();
    }
    std::vector<vec3f> getPositions() const {return positions;}
    std::vector<vec3f> getNormals() const {return normals;}
    std::vector<vec4f> getColors() const {return colors;}
    std::vector<vec2f> getTexCoords() const {return texture_coords;}
    std::vector<GLuint> getIndices() const {return indices;}
    size_t getIndicesSize() const {return indices.size();}

    float scale = 1.0;
    void setScale(float scale){
        this->scale = scale;
    }

    void setPos(vec3f pos){
        this->model_matrix.block<3, 1>(0, 3) = pos;
    }

    virtual void setColor(vec4f color) {
        colors.clear();
        for(size_t i = 0; i < positions.size(); ++i){
            colors.push_back(color);
        }
    };

    mat4f getModelMatrix(const float scale=1.0){
        mat4f matrix = mat4f::Identity();
        matrix.block<3, 3>(0, 0) = model_matrix.block<3, 3>(0, 0);
        matrix.block<3, 1>(0, 3) = model_matrix.block<3, 1>(0, 3) * scale; // TODO: pxk: wried for scale
        return matrix;
    }

    void transform(mat4f model_matrix){
        const auto& R = model_matrix.block<3, 3>(0, 0);
        const auto& t = model_matrix.block<3, 1>(0, 3);
        for(size_t i = 0; i < positions.size(); ++i){
            positions[i] = R * positions[i] + t;
        }
    }

    bool empty(){
        return positions.empty();
    }

    virtual void draw(Shader* shader, const Viewport& viewport) = 0;
};

class Grid : public Mesh{

public:

    void gen_grid_level(
        const vec3f& location, 
        const float& scale,
        const int level, 
        std::vector<vec3f>& grid_lines_x, 
        std::vector<vec3f>& grid_lines_y) const {

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

    void setup()
    {   
        clean();

        const vec3f location = vec3f(0.0f, 0.0f, 0.0f);
        const float scale = 1.0f;
        const float level = log10f(scale * 5);

        std::vector<vec3f> grid_lines_x;
        std::vector<vec3f> grid_lines_y;

        gen_grid_level(location, scale, floor(level) - 1, grid_lines_x, grid_lines_y);

        for(size_t i = 0; i < grid_lines_x.size(); ++i){
            positions.push_back(grid_lines_x[i]);
            bool pivot = int(grid_lines_x[i].x() + location.x() * scale) == 0;
            colors.push_back(pivot ? COLOR_AXIS_Y : vec4f(0.5, 0.5, 0.5, 0.25));
        }

        for(size_t i = 0; i < grid_lines_y.size(); ++i){
            positions.push_back(grid_lines_y[i]);
            bool pivot = int(grid_lines_y[i].y() + location.y() * scale) == 0;
            colors.push_back(pivot ? COLOR_AXIS_X : vec4f(0.5, 0.5, 0.5, 0.25));
        }

        gen_grid_level(location, scale, floor(level), grid_lines_x, grid_lines_y);

        for(size_t i = 0; i < grid_lines_x.size(); ++i){
            positions.push_back(grid_lines_x[i]);
            colors.push_back(vec4f{0.5, 0.5, 0.5, float(pow(level - floor(level), 0.9) * 0.25)});
        }

        for(size_t i = 0; i < grid_lines_y.size(); ++i){
            positions.push_back(grid_lines_y[i]);
            colors.push_back(vec4f{0.5, 0.5, 0.5, float(pow(level - floor(level), 0.9) * 0.25)});
        }

        for (size_t i = 0; i < positions.size(); ++i){
            indices.push_back(i);
        }
    }

    void draw(Shader* shader, const Viewport& viewport){
        
        mat4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();
        
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
    // create a centered cube with given edge length
    void setup(float size = 1.0f){
        clean();
        const float h = 0.5f * size;

        positions = std::vector<vec3f>{
            vec3f(-h, -h, -h), // 0
            vec3f( h, -h, -h), // 1
            vec3f( h,  h, -h), // 2
            vec3f(-h,  h, -h), // 3
            vec3f(-h, -h,  h), // 4
            vec3f( h, -h,  h), // 5
            vec3f( h,  h,  h), // 6
            vec3f(-h,  h,  h)  // 7
        };

        // per-vertex color: use classic RGB mapping (R=x>0, G=y>0, B=z>0)
        colors.clear();
        for (const auto &p : positions){
            vec4f c;
            c.x() = p.x() > 0.0f ? 1.0f : 0.0f; // R
            c.y() = p.y() > 0.0f ? 1.0f : 0.0f; // G
            c.z() = p.z() > 0.0f ? 1.0f : 0.0f; // B
            c.w() = 1.0f;
            colors.push_back(c);
        }

        // triangles (two per face)
        indices = std::vector<GLuint>{
            // Front (+Z)
            4, 5, 6,  6, 7, 4,
            // Back (-Z)
            0, 1, 2,  2, 3, 0,
            // Right (+X)
            1, 5, 6,  6, 2, 1,
            // Left (-X)
            0, 4, 7,  7, 3, 0,
            // Top (+Y)
            3, 2, 6,  6, 7, 3,
            // Bottom (-Y)
            0, 1, 5,  5, 4, 0
        };
    }

    void draw(Shader* shader, const Viewport& viewport) override {
        if (!shader) return;

        mat4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();

        // Caller is expected to set necessary uniforms (uMVP/uModelView or ProjMat)
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

class Frustum: public Mesh{

    std::vector<vec3f> plane_positions;
    std::vector<vec4f> plane_colors;
    std::vector<GLuint> plane_indices;

    std::vector<vec3f> triangle_positions;
    std::vector<vec4f> triangle_colors;
    std::vector<GLuint> triangle_indices;

    std::vector<vec3f> axis_positions;
    std::vector<vec4f> axis_colors;
    std::vector<GLuint> axis_indices;

public:
    void setup(const vec4f& intr, const float& scale=1.0f){
        clean();

        const float& fx = intr.x();
        const float& cx = intr.z();
        const float& cy = intr.w();

        positions =  std::vector<vec3f>{
            vec3f( 0.0f, 0.0f, 0.0f) * scale,
            vec3f( cx,  cy, fx) * scale,
            vec3f(-cx,  cy, fx) * scale,
            vec3f(-cx, -cy, fx) * scale,
            vec3f( cx, -cy, fx) * scale,
        };

        plane_positions = std::vector<vec3f>{
            vec3f( cx,  cy, fx) * scale,
            vec3f(-cx,  cy, fx) * scale,
            vec3f(-cx, -cy, fx) * scale,
            vec3f( cx, -cy, fx) * scale,
        };

        triangle_positions = std::vector<vec3f>{
            vec3f( cx/3, -cy*1.05, fx) * scale,
            vec3f(-cx/3, -cy*1.05, fx) * scale,
            vec3f( 0, -cy*1.25, fx) * scale,
        };

        indices = std::vector<GLuint>{
            0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3, 3, 4, 4, 1
        };

        plane_indices = std::vector<GLuint>{
            0, 2, 1, 0, 2, 3
        };

        triangle_indices = std::vector<GLuint>{0, 1, 2};

        for (size_t i = 0; i < positions.size(); ++i){
            colors.push_back(vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        }

        for (size_t i = 0; i < plane_positions.size(); ++i){
            plane_colors.push_back(vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        }

        for (size_t i = 0; i < triangle_positions.size(); i++){
            triangle_colors.push_back(vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        }

        axis_positions = std::vector<vec3f>{
            vec3f( 0.0f, 0.0f, 0.0f) * cx * scale,
            vec3f( 1.0f, 0.0f, 0.0f) * cx * scale,
            vec3f( 0.0f, 0.0f, 0.0f) * cx * scale,
            vec3f( 0.0f, 1.0f, 0.0f) * cx * scale,
            vec3f( 0.0f, 0.0f, 0.0f) * cx * scale,
            vec3f( 0.0f, 0.0f, 1.0f) * cx * scale,
        };

        axis_colors = std::vector<vec4f>{
            COLOR_AXIS_X, COLOR_AXIS_X, 
            COLOR_AXIS_Y, COLOR_AXIS_Y, 
            COLOR_AXIS_Z, COLOR_AXIS_Z
        };

        axis_indices = std::vector<GLuint>{0, 1, 2, 3, 4, 5};
    }

    void setColor(vec4f color) {
        colors.clear();
        for(size_t i = 0; i < positions.size(); ++i){
            colors.push_back(color);
        }
        triangle_colors.clear();
        for(size_t i = 0; i < triangle_positions.size(); ++i){
            triangle_colors.push_back(color);
        }
        plane_colors.clear();
        for(size_t i = 0; i < plane_positions.size(); ++i){
            plane_colors.push_back(color);
        }
    };

    void draw(Shader* shader, const Viewport& viewport) {

        mat4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();

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

    void transform(mat4f model_matrix){
        const auto& R = model_matrix.block<3, 3>(0, 0);
        const auto& t = model_matrix.block<3, 1>(0, 3);
        for(size_t i = 0; i < positions.size(); ++i){
            positions[i] = R * positions[i] + t;
        }

        for(size_t i = 0; i < plane_positions.size(); ++i){
            plane_positions[i] = R * plane_positions[i] + t;
        }

        for(size_t i = 0; i < triangle_positions.size(); ++i){
            triangle_positions[i] = R * triangle_positions[i] + t;
        }

        for(size_t i = 0; i < axis_positions.size(); ++i){
            axis_positions[i] = R * axis_positions[i] + t;
        }
    }

};


class PointCloud: public Mesh{
public:
    void setup(const int& num=10000)
    {
        clean();
        srand(static_cast<unsigned int>(time(NULL)));

        for (int i = 0; i < num; ++i) {
            vec3f point;
            point.x() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            point.y() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            point.z() = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            positions.push_back(point);

            vec4f color;
            color.x() = static_cast<float>(rand()) / RAND_MAX;
            color.y() = static_cast<float>(rand()) / RAND_MAX;
            color.z() = static_cast<float>(rand()) / RAND_MAX;
            color.w() = 1;
            colors.push_back(color);

            indices.push_back(i);
        }
    }

    void setup(const std::vector<vec3f>& pc, const vec4f color){
        clean();
        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i]);
            colors.push_back(color);
            indices.push_back(positions.size() - 1);
        }
    }

    void setup(const std::vector<vec3f>& pc, const std::vector<vec4f> color){
        clean();
        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i]);
            colors.push_back(color[i]);
            indices.push_back(positions.size() - 1);
        }
    }

    void setPointSize(const int size){
        point_size = size;
    }

    void draw(Shader* shader, const Viewport& viewport){

        mat4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();

        shader->bind();
        shader->set_uniform("ProjMat", transform);
        shader->set_uniform("Alpha", 1.0f);
        shader->set_uniform("PointSize", static_cast<float>(point_size));
        shader->set_attribute("Color", getColors());
        shader->set_attribute("Position", getPositions());
        shader->set_indices(getIndices());
        shader->draw_indexed(GL_POINTS, 0, getIndicesSize());
        shader->unbind();
    }

private:
    int point_size = 1;
};

class Line: public Mesh{
public:
    void setup(std::vector<vec3f>& pc, std::vector<vec4f>& color)
    {
        clean();
        if(pc.size() <= 1)
            return;

        positions = pc;
        colors = color;

        for(size_t i = 0; i < positions.size(); ++i){
            indices.push_back(i);
        }
    }

    void setup(const std::vector<vec3f>& pc, const vec4f color){
        clean();
        if(pc.size() <= 1)
            return;

        for (size_t i = 0; i < pc.size(); ++i) {
            positions.push_back(pc[i]);
            colors.push_back(color);
            indices.push_back(positions.size() - 1);
        }
    }

    void draw(Shader* shader, const Viewport& viewport){

        mat4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();

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

} // namespace liteviz

#endif // __LITEVIZ_MESH_H__
