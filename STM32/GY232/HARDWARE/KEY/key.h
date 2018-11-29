#ifndef __KEY_H
#define __KEY_H

typedef enum {
    Button_KEY0 = 0,
    Button_KEY1 = 1,
    Button_WAKEUP = 2
} Button_TypeDef;

typedef struct {
    GPIO_TypeDef *BUTTON_PORT;
    uint16_t BUTTON_PIN;
    GPIOMode_TypeDef BUTTON_MODE;
} Button_PinModeDef;

#define KEY0 PAin(13)
#define KEY1 PAin(15)
#define KEY2 PAin(0)

void KEY_Init ( void );
u8 KEY_Scan ( void );
#endif