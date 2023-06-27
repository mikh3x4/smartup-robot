
#pragma once


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define ASSERT(condition) ASSERTM(condition, "")

#define ASSERTM(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s\n%s in %s:%d %s\n\n", \
                    #condition, message, __FILENAME__, __LINE__, __func__); \
            panic(); \
        } \
    } while (0)

#define ASSERT_RETURN(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s returning\nin %s:%d %s\n\n", \
                    #condition, __FILENAME__, __LINE__, __func__); \
            return false; \
        } \
    } while (0)

void panic(){
    while (1){
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
    }
}

