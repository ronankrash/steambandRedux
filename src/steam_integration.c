/* File: steam_integration.c */
#include "steam_integration.h"
#include "logging.h"

/*
 * NOTE: To enable Steam integration, you must:
 * 1. Download the Steamworks SDK from the Steam Partner site.
 * 2. Copy the 'public' folder from the SDK to 'steamworks_sdk/public' in your project root.
 * 3. Copy 'redist/steam_api64.dll' (or steam_api.dll for 32-bit) to your build output directory.
 * 4. Define USE_STEAM in CMakeLists.txt or your build configuration.
 */

#ifdef USE_STEAM
#include "steam/steam_api.h"
#pragma comment(lib, "steam_api64.lib") /* Adjust for 32-bit if needed */
#endif

int steam_init(void) {
#ifdef USE_STEAM
    if (!SteamAPI_Init()) {
        LOG_E("SteamAPI_Init() failed. Ensure Steam is running and steam_appid.txt is present.");
        return 0;
    }
    LOG_I("Steam API initialized successfully.");
    return 1;
#else
    LOG_I("Steam integration disabled (USE_STEAM not defined).");
    return 1; /* Pretend success */
#endif
}

void steam_cleanup(void) {
#ifdef USE_STEAM
    SteamAPI_Shutdown();
#endif
}

void steam_update(void) {
#ifdef USE_STEAM
    SteamAPI_RunCallbacks();
#endif
}

