#include "../romulusni/RomulusNI.hpp"


namespace romulusni {

// Global with the 'main' size. Used by pload()
uint64_t g_main_size = 0;
// Global with the 'main' addr. Used by pload()
uint8_t* g_main_addr = 0;

// Counter of nested write transactions
thread_local int64_t tl_nested_write_trans = 0;
// Counter of nested read-only transactions
thread_local int64_t tl_nested_read_trans = 0;
// Global instance
RomulusNI gRomNI {};



//
// Private methods
//






} // End of romulusni namespace
