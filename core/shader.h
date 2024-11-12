#ifndef __VIEWER_SHADER_H__
#define __VIEWER_SHADER_H__

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <Eigen/Eigen>
#include <GL/glew.h>
#include <GL/gl.h>


template <typename E> inline GLenum is_type_integral() {
    return GL_FALSE;
}

template <> inline GLenum is_type_integral<GLbyte>() {
    return GL_TRUE;
}

template <> inline GLenum is_type_integral<GLshort>() {
    return GL_TRUE;
}

template <> inline GLenum is_type_integral<GLint>() {
    return GL_TRUE;
}

template <> inline GLenum is_type_integral<GLubyte>() {
    return GL_TRUE;
}

template <> inline GLenum is_type_integral<GLushort>() {
    return GL_TRUE;
}

template <> inline GLenum is_type_integral<GLuint>() {
    return GL_TRUE;
}

template <typename E> inline GLenum get_type_enum() {
    puts("Error getting type enum.");
    exit(0);
    return GL_NONE;
}

template <> inline GLenum get_type_enum<GLbyte>() {
    return GL_BYTE;
}

template <> inline GLenum get_type_enum<GLshort>() {
    return GL_SHORT;
}

template <> inline GLenum get_type_enum<GLint>() {
    return GL_INT;
}

template <> inline GLenum get_type_enum<GLubyte>() {
    return GL_UNSIGNED_BYTE;
}

template <> inline GLenum get_type_enum<GLushort>() {
    return GL_UNSIGNED_SHORT;
}

template <> inline GLenum get_type_enum<GLuint>() {
    return GL_UNSIGNED_INT;
}

template <> inline GLenum get_type_enum<GLfloat>() {
    return GL_FLOAT;
}


class Shader {
public:
    Shader(const char *vshader_path, const char *fshader_path, const char *gshader_path = nullptr) {
        GLint status;

        std::string vshader_source = readShaderSourceFromFile(vshader_path);
        std::string fshader_source = readShaderSourceFromFile(fshader_path);

        vshader = glCreateShader(GL_VERTEX_SHADER);
        const char* vshader_code = vshader_source.c_str();
        glShaderSource(vshader, 1, &vshader_code, nullptr);
        glCompileShader(vshader);
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            char infoLog[512];
            glGetShaderInfoLog(vshader, 512, nullptr, infoLog);
            std::cerr << "Error compiling vertex shader:\n" << infoLog << std::endl;
            exit(1);
        }

        fshader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fshader_code = fshader_source.c_str();
        glShaderSource(fshader, 1, &fshader_code, nullptr);
        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            char infoLog[512];
            glGetShaderInfoLog(fshader, 512, nullptr, infoLog);
            std::cerr << "Error compiling fragment shader:\n" << infoLog << std::endl;
            exit(1);
        }

        GLuint gshader = 0;
        if (gshader_path != nullptr) {
            std::string gshader_source = readShaderSourceFromFile(gshader_path);
            gshader = glCreateShader(GL_GEOMETRY_SHADER);
            const char* gshader_code = gshader_source.c_str();
            glShaderSource(gshader, 1, &gshader_code, nullptr);
            glCompileShader(gshader);
            glGetShaderiv(gshader, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                char infoLog[512];
                glGetShaderInfoLog(gshader, 512, nullptr, infoLog);
                std::cerr << "Error compiling geometry shader:\n" << infoLog << std::endl;
                exit(1);
            }
        }

        program = glCreateProgram();
        glAttachShader(program, vshader);
        glAttachShader(program, fshader);
        if (gshader_path != nullptr) {
            glAttachShader(program, gshader);
        }
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != (GLint)GL_TRUE) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Error linking shader program:\n" << infoLog << std::endl;
            exit(1);
        }

        glGenBuffers(1, &index_buffer);
        glGenVertexArrays(1, &vertex_array);
    }

    ~Shader() {
        for (auto [attrib, buffer] : attribute_buffers) {
            glDeleteBuffers(1, &buffer);
        }
        glDeleteBuffers(1, &index_buffer);
        glDeleteVertexArrays(1, &vertex_array);

        glDetachShader(program, fshader);
        glDetachShader(program, vshader);
        glDeleteProgram(program);
        glDeleteShader(fshader);
        glDeleteShader(vshader);
    }

    void bind() {
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    }

    void unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void set_uniform(const std::string &name, const float &value) {
        GLint uni = uniform(name);
        glUniform1f(uni, value);
    }

    void set_uniform(const std::string &name, const Eigen::Vector3f &vector) {
        GLint uni = uniform(name);
        glUniform3fv(uni, 1, vector.data());
    }

    void set_uniform(const std::string &name, const Eigen::Vector4f &vector) {
        GLint uni = uniform(name);
        glUniform4fv(uni, 1, vector.data());
    }

    void set_uniform(const std::string &name, const Eigen::Matrix4f &matrix) {
        GLint uni = uniform(name);
        glUniformMatrix4fv(uni, 1, GL_FALSE, matrix.data());
    }

    // texture
    void set_uniform(const std::string &name) {
        GLint uni = uniform(name);
        glUniform1i(uni, 0);
    }

    std::string readShaderSourceFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open shader file: " << filePath << std::endl;
            exit(1);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    template <typename E, int N>
    void set_attribute(const std::string &name,
                    const std::vector<Eigen::Matrix<E, N, 1>> &data) {
        GLint attrib = attribute(name);
        if (attribute_buffers.count(attrib) == 0) {
            GLuint buffer;
            glGenBuffers(1, &buffer);
            attribute_buffers[attrib] = buffer;
        }
        GLuint buffer = attribute_buffers.at(attrib);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(E) * N * data.size(), &data[0], GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(attrib);
        glVertexAttribPointer(attrib, N, get_type_enum<E>(), is_type_integral<E>(), 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void set_indices(const std::vector<unsigned int> &indices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_DYNAMIC_DRAW);
    }

    void draw(GLenum mode, GLuint start, GLuint count) {
        glDrawArrays(mode, start, count);
    }

    void draw_indexed(GLenum mode, GLuint start, GLuint count) {
        glDrawElements(mode, count, GL_UNSIGNED_INT,
                    (const void *)(start * sizeof(GLuint)));
    }

    private:
    GLint uniform(const std::string &name) {
        if (uniforms.count(name) == 0) {
            GLint location =
                glGetUniformLocation(program, name.c_str());
            if (location == -1) {
                puts("Error getting uniform location.");
                exit(0);
            }
            uniforms[name] = location;
        }
        return uniforms.at(name);
    }

    GLint attribute(const std::string &name) {
        if (attributes.count(name) == 0) {
            GLint location = glGetAttribLocation(program, name.c_str());
            if (location == -1) {
                puts("Error getting attribute location.");
                exit(0);
            }
            attributes[name] = location;
        }
        return attributes.at(name);
    }

    GLuint program;
    GLuint vshader;
    GLuint fshader;
    std::map<std::string, GLint> uniforms;
    std::map<std::string, GLint> attributes;
    std::map<GLint, GLuint> attribute_buffers;
    GLuint index_buffer;
    GLuint vertex_array;

};


#endif // LIGHTVIS_SHADER_H
