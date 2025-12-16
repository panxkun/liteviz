#ifndef __LITEVIZ_BASE_DATA_H__
#define __LITEVIZ_BASE_DATA_H__

#include <liteviz/core/common.h>

namespace liteviz {

enum class DataType {
    POINT_CLOUD,
    MESH,
    SPLAT,
    SFM,
    TRAJECTORY,
    IMAGE,
    UNKNOWN
};

class BaseData {
public:
    virtual ~BaseData() = default;
    virtual DataType getType() const = 0;
};

} // namespace liteviz

#endif // __LITEVIZ_BASE_DATA_H__