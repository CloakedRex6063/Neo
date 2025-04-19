#pragma once

#define NEO_WARN_BEG() __pragma(warning(push))

#define NEO_WARN_END() __pragma(warning(pop))

#define NEO_WARN_WCONV() __pragma(warning(disable : 4244))