/**
 * DxWifi includes and definitions
 */


#ifndef DXWIFI_H
#define DXWIFI_H

#define DXWIFI_VERSION_MAJOR    0
#define DXWIFI_VERSION_MINOR    1
#define DXWIFI_VERSION_PATCH    0
#define DXWIFI_VERSION_RELEASE  "alpha"

#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

#define DXWIFI_VERSION      STRINGIFY(DXWIFI_VERSION_MAJOR) "." \
                            STRINGIFY(DXWIFI_VERSION_MINOR) "." \
                            STRINGIFY(DXWIFI_VERSION_PATCH) "-" \
                            DXWIFI_VERSION_RELEASE

#endif // DXWIFI_H