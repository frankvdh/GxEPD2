#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#if defined(RPI) || defined(WIN32)
#define Debug(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define Debug(p) Serial.print(p)
#endif
#else
#define Debug(fmt, ...)
#endif // DEBUG
#endif // DEBUG_H_
