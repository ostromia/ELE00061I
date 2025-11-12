#ifndef __GPIO_H
#define __GPIO_H

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} GPIO;

#endif
