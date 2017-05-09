Custom Service Tutorial
-------

This tutorial will show you how to create a custom service with a custom value characteristic in the ble_app_template project found in the Nordic nRF5 SDK. 
<!---
## TODO

- [ ] Add register definition file (.svd) and retarget of printf to the ble_app_uart Segger Embedded Project.
--->

## Requirements

nRF5 SDK v13.0.0
nRF52 DK
Keil ARM MKD v5.22
nRF Commandline Tools

## Tutorial Steps

```C    
    typedef enum {
        COMMAND_1,
        COMMAND_2,
        COMMAND_3,
        NO_COMMAND
    } uart_command_t;
```

[memcpy](https://www.tutorialspoint.com/c_standard_library/c_function_memcpy.htm)