#ifndef ASSIGN3_CONSTANTS_H_
#define ASSIGN3_CONSTANTS_H_

#define M_PI 3.14159265358979323846
#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)
#define kPi 3.14159265358979323846
#define k89DegreesToRadians 1.55334303

// Enum for game states
typedef enum {
  kStart = 0,
  kPause = 1,
  kResume = 2,
  kAutoDrive = 3,
  kCrashingFall = 4,
  kCrashingCliff = 5,
  kGameOver = 6,
} kGameState;

#endif
