Custom Service Tutorial
-------

This tutorial will show you how to create a custom service with a custom value characteristic in the ble_app_template project found in the Nordic nRF5 SDK. 
<!---
## TODO

- [ ] Add register definition file (.svd) and retarget of printf to the ble_app_uart Segger Embedded Project.
--->

## Requirements

- nRF5 SDK v13.0.0
- nRF52 DK
- Keil ARM MKD v5.22
- nRF Commandline Tools

## Tutorial Steps

```C    
    uint8_t data
    // put C code here
```

[link to webpage](www.google.com)

### Step 1

The first thing we need to do is to create a new .c file, lets call it ble_custom_service.c, and its accompaning .h file ble_custom_service.h. We'll start by declaring types and functions in ble_custom_service.h before we move to the actual implementation of our custom service. At the top we'll need to include the following .h files

```C    
    #include <stdint.h>
    #include <stdbool.h>
    #include "ble.h"
    #include "ble_srv_common.h"
```


Next, we're going to need a 128-bit UUID for our custom service since its not Bluetooth SIG . There are several ways to generate a 128-bit UUID, but we'll use [this](https://www.uuidgenerator.net/version4) Online UUID generator. The webpage will generate a random 128-bit UUID, which in my case was

```C 
    f364adc9-b000-4042-ba50-05ca45bf8abc
```
Now, we're going to define our UUID base

```C    
#define CUSTOM_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}
````
The UUID is written in 

```C   
#define CUSTOM_SERVICE_UUID               0x1400
#define CUSTOM_VALUE_CHAR_UUID            0x1401
```