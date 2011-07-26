#ifndef __STM32F10x_WWDG_H
#define __STM32F10x_WWDG_H

#include "stm32f10x.h"

/*
 * WWDG_Prescaler 
 */ 
  
#define WWDG_Prescaler_1    ((uint32_t)0x00000000)
#define WWDG_Prescaler_2    ((uint32_t)0x00000080)
#define WWDG_Prescaler_4    ((uint32_t)0x00000100)
#define WWDG_Prescaler_8    ((uint32_t)0x00000180)

#define IS_WWDG_PRESCALER(PRESCALER)					\
  (((PRESCALER) == WWDG_Prescaler_1) ||					\
   ((PRESCALER) == WWDG_Prescaler_2) ||					\
   ((PRESCALER) == WWDG_Prescaler_4) ||					\
   ((PRESCALER) == WWDG_Prescaler_8))

#define IS_WWDG_WINDOW_VALUE(VALUE) ((VALUE) <= 0x7F)
#define IS_WWDG_COUNTER(COUNTER) (((COUNTER) >= 0x40) && ((COUNTER) <= 0x7F))

void WWDG_DeInit(void);
void WWDG_SetPrescaler(uint32_t WWDG_Prescaler);
void WWDG_SetWindowValue(uint8_t WindowValue);
void WWDG_EnableIT(void);
void WWDG_SetCounter(uint8_t Counter);
void WWDG_Enable(uint8_t Counter);
FlagStatus WWDG_GetFlagStatus(void);
void WWDG_ClearFlag(void);

#endif /* __STM32F10x_WWDG_H */
