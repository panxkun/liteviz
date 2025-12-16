#ifndef __LITEVIZ_CUDABUFFER_H__
#define __LITEVIZ_CUDABUFFER_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>
#include <torch/torch.h>

namespace liteviz {

class CUDABuffer {
private:
    GLuint vbo; 
    GLuint fbo;
    cudaGraphicsResource* cudaResource;  
    GLenum bufferType;
    size_t bufferSize;

public:

    CUDABuffer(GLenum bufferType):
        bufferType(bufferType), bufferSize(0), cudaResource(nullptr), vbo(0) {
    }

    ~CUDABuffer() {
        if (cudaResource) {
            cudaGraphicsUnregisterResource(cudaResource);
        }
        if (vbo) {
            glDeleteBuffers(1, &vbo);
        }
    }

    void initialize(size_t initialSize) {
        bufferSize = initialSize;
        glGenTransformFeedbacks(1, &fbo);
        glGenBuffers(1, &vbo);
        glBindBuffer(bufferType, vbo);
        glBufferData(bufferType, bufferSize, nullptr, GL_STREAM_DRAW);
        cudaGraphicsGLRegisterBuffer(&cudaResource, vbo, cudaGraphicsMapFlagsWriteDiscard);
        glBindBuffer(bufferType, 0);
        cudaGraphicsMapResources(1, &cudaResource, 0);
    }

    void resizeBuffer(size_t newSize) {
        if (newSize > bufferSize) {
            if (cudaResource) {
                cudaGraphicsUnregisterResource(cudaResource);
            }
            initialize(newSize);
        }
    }

    void loadData(torch::Tensor tensor) {
        size_t dataSize = tensor.nbytes();
        resizeBuffer(dataSize);

        cudaGraphicsMapResources(1, &cudaResource, 0);

        void* devPtr;
        size_t size;
        cudaGraphicsResourceGetMappedPointer(&devPtr, &size, cudaResource);
        cudaMemcpy(devPtr, tensor.data_ptr(), dataSize, cudaMemcpyDeviceToDevice);
        cudaGraphicsUnmapResources(1, &cudaResource, 0);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL Error in uploadData(): 0x" << std::hex << error << std::dec << std::endl;
        }
    }

    GLuint getVBO() const {
        return vbo;
    }

    GLuint getFBO() const {
        return fbo;
    }

    int getSize() const {
        return bufferSize;
    }

    void bind(size_t idx, GLint size, GLenum type) const {
        glBindBuffer(bufferType, vbo);
        glVertexAttribPointer(idx, size, type, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(idx);
    }

    void unbind(size_t idx) const {
        glDisableVertexAttribArray(idx);
        glBindBuffer(bufferType, 0);
}

};

} // namespace liteviz

#endif // __LITEVIZ_CUDABUFFER_H__