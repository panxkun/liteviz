#ifndef __LITEVIZ_BASE_CONFIG_H__
#define __LITEVIZ_BASE_CONFIG_H__

#include <liteviz/core/common.h>

namespace liteviz {

struct BaseConfig{
    virtual ~BaseConfig() = default;  // Make it polymorphic for dynamic_cast

    // Base Settings
    vec4f bgColor = vec4f(1.0f, 1.0f, 1.0f, 0.00f);
    float targetFrameRate = -1.0f;
    float fov = 60.0f;
    bool vsync = true;
    bool transparentConfigBG = true;
    float x_size = 300.0f;
    float y_size = 20.0f;
    
};

} // namespace liteviz

#endif // __LITEVIZ_BASE_CONFIG_H__