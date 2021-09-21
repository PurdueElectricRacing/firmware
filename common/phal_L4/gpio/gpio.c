#include "common/phal_L4/gpio/gpio.h"

#define GPIO_MODE_INPUT  (0b00)
#define GPIO_MODE_OUTPUT (0b01)
#define GPIO_MODE_AF     (0b10)
#define GPIO_MODE_ANALOG (0b11)

/**
 * @brief Initilize an array of GPIO pins with a specified output mode. Will also enable the specified clocks for the GPIO Pins
 * 
 * @return true Sucessful config of GPIO
 * @return false Unkonwn config of GPIO
 */
bool PHAL_initGPIO(GPIOConfig_t config[], uint8_t config_len)
{
    for (int i = 0; i < config_len; i++)
    {
        // Enable clock
        switch ((uint32_t) config[i].bank)
        {
            case (uint32_t) GPIOA:
                RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
                break;

            case (uint32_t) GPIOB:
                RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
                break;

            case (uint32_t) GPIOC:
                RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
                break;

            case (uint32_t) GPIOH:
                RCC->AHB2ENR |= RCC_AHB2ENR_GPIOHEN;
                break;
            
            default:
                return false;
                break;
        }

        // Setup GPIO output type
        switch(config[i].type)
        {
            case OUTPUT:
                config[i].bank->MODER &= ~(GPIO_MODER_MODE0_Msk << (GPIO_MODER_MODE1_Pos * config[i].pin));
                config[i].bank->MODER |= GPIO_MODE_OUTPUT << (GPIO_MODER_MODE1_Pos * config[i].pin);

                config[i].bank->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0_Msk << (GPIO_OSPEEDR_OSPEED1_Pos * config[i].pin));
                config[i].bank->OSPEEDR |= config[i].config.ospeed << (GPIO_OSPEEDR_OSPEED1_Pos * config[i].pin);
                break;
                
            case INPUT:
                config[i].bank->MODER &= ~(GPIO_MODER_MODE0_Msk << (GPIO_MODER_MODE1_Pos * config[i].pin));
                config[i].bank->MODER |= GPIO_MODE_INPUT << (GPIO_MODER_MODE1_Pos * config[i].pin);

                config[i].bank->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk << (GPIO_PUPDR_PUPD1_Pos * config[i].pin));
                config[i].bank->PUPDR |= config[i].config.push_pull << (GPIO_PUPDR_PUPD1_Pos * config[i].pin);
                break;

            case ALT_FUNC:
                config[i].bank->MODER &= ~(GPIO_MODER_MODE0_Msk << (GPIO_MODER_MODE1_Pos * config[i].pin));
                config[i].bank->MODER |= GPIO_MODE_AF << (GPIO_MODER_MODE1_Pos * config[i].pin);

                // Configure AF number
                uint8_t afr_i = config[i].pin > 7 ? 1 : 0;
                config[i].bank->AFR[afr_i] &= ~(GPIO_AFRL_AFSEL0_Msk << (GPIO_AFRL_AFSEL1_Pos * (config[i].pin % 8)));
                config[i].bank->AFR[afr_i] |= config[i].config.af_num << (GPIO_AFRL_AFSEL1_Pos * (config[i].pin % 8));
                break;

            default:
                return false;
        }
    }
    
}