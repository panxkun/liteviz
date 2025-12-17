#ifndef __LITEVIZ_BASE_RENDERER_H__
#define __LITEVIZ_BASE_RENDERER_H__

#include <liteviz/core/common.h>
#include <liteviz/core/viewport.h>
#include <liteviz/core/shader.h>
#include <liteviz/core/mesh.h>

namespace liteviz {

class BaseRenderer {
public:
    virtual ~BaseRenderer() = default;
    virtual void render(const Viewport& viewport) = 0;
};

} // namespace liteviz

#endif // __LITEVIZ_BASE_RENDERER_H__