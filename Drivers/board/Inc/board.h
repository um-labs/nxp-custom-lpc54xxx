#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"

#define BOARD_EXTCLKINRATE (0)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitSDRAM(void);
void BOARD_InitSPIFI(void);

void BOARD_InitI2C(void);
void BOARD_LockI2C(void);
void BOARD_UnlockI2C(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
