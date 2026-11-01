/* Select UART hardware access at compile time from the device profile. */
#if MINI_UART_STYLE == 0
#include "modern.inc"
#elif MINI_UART_STYLE == 1
#include "legacy.inc"
#elif MINI_UART_STYLE == 2
#include "mega64.inc"
#elif MINI_UART_STYLE == 3
#include "tiny841.inc"
#elif MINI_UART_STYLE == 4
#include "tiny1634.inc"
#elif MINI_UART_STYLE == 5
#include "soft.inc"
#else
#error Unsupported USART hardware profile
#endif
