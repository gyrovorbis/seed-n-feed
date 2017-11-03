#ifndef UTILITIES_H
#define UTILITIES_H

#define VERSION_MAJOR  0
#define VERSION_MINOR  1
#define VERSION_PATCH  1

#define _STRINGIFY_MACRO(a) #a
#define STRINGIFY_MACRO(a) _STRINGIFY_MACRO(a)

#define VERSION_STRING STRINGIFY_MACRO(VERSION_MAJOR) "." STRINGIFY_MACRO(VERSION_MINOR) "." STRINGIFY_MACRO(VERSION_PATCH)

#ifndef GYRO_BUILD_NODE
#   define GYRO_LOCAL_BUILD
#   define GYRO_BUILD_NODE     LOCAL
#endif
#ifndef GYRO_BUILD_JOB
#   define GYRO_BUILD_JOB      UNKNOWN
#endif
#ifndef GYRO_BUILD_NUMBER
#   define GYRO_BUILD_NUMBER   UNKNOWN
#endif
#ifndef GYRO_BUILD_BRANCH
#   define GYRO_BUILD_BRANCH   UNKNOWN
#endif
#ifndef GYRO_BUILD_COMMIT
#   define GYRO_BUILD_COMMIT   UNKNOWN
#endif

#define Q_CSTR(a) (a.toStdString().c_str())

#define KG_PER_LB                       0.453592
#define DOUBLE_VALIDATOR_DECIMALS_MAX   10



#endif // UTILITIES_H