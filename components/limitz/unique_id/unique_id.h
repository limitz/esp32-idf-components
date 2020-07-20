#pragma once
#if (CONFIG_LMTZ_UNIQUE_ID_SRC_MANUAL)
inline const char* unique_id() { return CONFIG_LMTZ_UNIQUE_ID_MANUAL; }
#elif (CONFIG_LMTZ_UNIQUE_ID_SRC_MAC)
const char* unique_id();
#endif
