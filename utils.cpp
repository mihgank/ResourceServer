#include "utils.h"

namespace utils{
quint64 GetCurrentTimestamp() {
    return QDateTime::currentSecsSinceEpoch();
}
//TODO: BytesDecmpile function (for resources)
}
