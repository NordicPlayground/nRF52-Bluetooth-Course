Custom Service Tutorial
-------

This tutorial will show you how to create a custom service with a custom value characteristic in the ble_app_template project found in the Nordic nRF5 SDK v13.0.0. This tutorial can be seen as the combined version of the BLE [Advertising](https://devzone.nordicsemi.com/tutorials/5) / [Services](https://devzone.nordicsemi.com/tutorials/8) / [Characteristics](https://devzone.nordicsemi.com/tutorials/17) , A Beginner's Tutorial series, which I strongly recommend to take a look at as they go deeper into the matter than this tutorial.

The aim of this tutorial is simply to create one service with one characteristic without too much theory in between the steps. There are no .c or .h files that needs to be downloaded as we will be starting from scratch in the ble_app_template project. 

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
<!---
```C    
    uint8_t data
    // put C code here
```

[link to webpage](www.google.com)
--->
### Step 1 - 

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
The UUID is given as the sixteen octets of a UUID are represented as 32 hexadecimal (base 16) digits, displayed in five groups separated by hyphens, in the form 8-4-4-4-12. The 16 octets are given in big-endian, while we use the small-endian representation in our SDK. Thus, we must reverse the byte-ordering when we define our UUID base, as shown below.

```C    
#define CUSTOM_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}
````
Now that we have defined our Base UUID, we need to define a 16-bit UUID for the Custom Service and a 16-bit UUID for a Custom Value Characteristic.  

```C   
#define CUSTOM_SERVICE_UUID               0x1400
#define CUSTOM_VALUE_CHAR_UUID            0x1401
```
The values for the 16-bit UUIDs can pretty much be choosen by random 

- [ ] Look up if the 16-bit UUIDs can be chosen randomly or if any rules apply. 

Next we need to declare an event type specific to our service

```C    
typedef enum
{
    BLE_CUS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_CUS_EVT_NOTIFICATION_DISABLED,                             /**< Custom value notification disabled event. */
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED
} ble_cus_evt_type_t;
```

After declaring the event type we need to declare an event structure that holds a ble_cus_evt_type_t event, i.e. 

```C    
    /**@brief Custom Service event. */
    typedef struct
    {
        ble_cus_evt_type_t evt_type;                                  /**< Type of event. */
    } ble_cus_evt_t;
```

The next step is to add a forward declaration of the ble_cus_t type
```C 
    // Forward declaration of the ble_cus_t type.
    typedef struct ble_cus_s ble_cus_t;
```
The reason why we need to do this is because the ble_cus_t type will be referenced in the decleration of the Custom Service event handler type which we will define next

```C 
    /**@brief Custom Service event handler type. */
    typedef void (*ble_cus_evt_handler_t) (ble_cus_t * p_bas, ble_cus_evt_t * p_evt);
```

This event handler type can be used to declare an event handler function that we can invoke and pass ble_cus_evt_type_t events to. More on this latter. 

Ok, so far so good. Now we need to create two structures, one Custom Service init structure, ble_cus_init_t struct to hold all the options and data needed to initialize our custom service.

```C 
    /**@brief Battery Service init structure. This contains all options and data needed for
    *        initialization of the service.*/
    typedef struct
    {
        ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
        bool                          support_notification;           /**< TRUE if notification of Battery Level measurement is supported. */
        ble_srv_report_ref_t *        p_report_ref;                   /**< If not NULL, a Report Reference descriptor with the specified value will be added to the Battery Level characteristic */
        uint8_t                       initial_custom_value;           /**< Initial custom value */
        ble_srv_cccd_security_mode_t  battery_level_char_attr_md;     /**< Initial security level for battery characteristics attribute */
        ble_gap_conn_sec_mode_t       battery_level_report_read_perm; /**< Initial security level for battery report read attribute */
    } ble_cus_init_t;
```

The second struct that we need to create is the Custom Service structure, ble_cus_s, which holds the status information of the service. 

```C 
    /**@brief Battery Service structure. This contains various status information for the service. */
    struct ble_cus_s
    {
        ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
        uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
        ble_gatts_char_handles_t      custom_value_handles;          /**< Handles related to the Custom Value characteristic. */
        uint16_t                      report_ref_handle;              /**< Handle of the Report Reference descriptor. */
        uint8_t                       custom_value_last;             /**< Last Custom Value measurement passed to the Custom Service. */
        uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
        bool                          is_notification_supported;      /**< TRUE if notification of Custom Value is supported. */
        uint8_t                       uuid_type; 
    };
```


Great, the last thing we need to do in the ble_custom_service.h file is to add some function declerations. First, we're going to add the ble_cus_init function

```C    
    /**@brief Function for initializing the Custom Service.
    *
    * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
    *                          the application. It will be initialized by this function, and will later
    *                          be used to identify this particular service instance.
    * @param[in]   p_cus_init  Information needed to initialize the service.
    *
    * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
    */
    uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);
```
Second, we're going to add the ble_cus_on_ble_evt function

```C   
    /**@brief Function for handling the Application's BLE Stack events.
    *
    * @details Handles all events from the BLE stack of interest to the Custom Service.
    *
    * @note For the requirements in the BAS specification to be fulfilled,
    *       ble_bas_battery_level_update() must be called upon reconnection if the
    *       battery level has changed while the service has been disconnected from a bonded
    *       client.
    *
    * @param[in]   p_cus      Battery Service structure.
    * @param[in]   p_ble_evt  Event received from the BLE stack.
    */
    void ble_cus_on_ble_evt(ble_cus_t * p_bas, ble_evt_t * p_ble_evt);
```

and lastly we're going to add the ble_cus_custom_value_update function.

- [ ] Add brief to the ble_cus_custom_value_update function

```C  
    uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value);
```

### Step 2 -  ble_custom_service.c

