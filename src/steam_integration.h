/* File: steam_integration.h */
#ifndef INCLUDED_STEAM_INTEGRATION_H
#define INCLUDED_STEAM_INTEGRATION_H

/*
 * Initialize Steam API
 * Returns TRUE if successful
 */
int steam_init(void);

/*
 * Shutdown Steam API
 */
void steam_cleanup(void);

/*
 * Process Steam callbacks (call this every frame/turn)
 */
void steam_update(void);

#endif /* INCLUDED_STEAM_INTEGRATION_H */

