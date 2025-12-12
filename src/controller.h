/* File: controller.h */
#ifndef INCLUDED_CONTROLLER_H
#define INCLUDED_CONTROLLER_H

/*
 * Initialize controller support
 */
void controller_init(void);

/*
 * Check for controller input and queue keypresses
 * Returns TRUE if input was handled
 */
int controller_check(void);

#endif /* INCLUDED_CONTROLLER_H */

