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
- nRF51 DK or nRF52 DK
- Keil ARM MKD v5.22 (strictly speaking you can use any IDE you want)
- nRF Commandline Tools

## Tutorial Steps
<!---
```c    
    uint8_t data
    // put C code here
```

[link to webpage](www.google.com)
--->
### Step 1 - Creating a Custom Base UUID 

The first thing we need to do is to create a new .c file, lets call it ble_custom_service.c, and its accompaning .h file ble_custom_service.h.  At the top of the header file we'll need to include the following .h files

```c
#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
```

Next, we're going to need a 128-bit UUID for our custom service since we're not going to implement our service with one of the  16-bit Bluetooth SIG UUIDs that are reserved for standardized profiles. There are several ways to generate a 128-bit UUID, but we'll use [this](https://www.uuidgenerator.net/version4) Online UUID generator. The webpage will generate a random 128-bit UUID, which in my case was

```
f364adc9-b000-4042-ba50-05ca45bf8abc
```
The UUID is given as the sixteen octets of a UUID are represented as 32 hexadecimal (base 16) digits, displayed in five groups separated by hyphens, in the form 8-4-4-4-12. The 16 octets are given in big-endian, while we use the small-endian representation in our SDK. Thus, we must reverse the byte-ordering when we define our UUID base, as shown below.

```c
#define CUSTOM_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}
```
Now that we have defined our Base UUID, we need to define a 16-bit UUID for the Custom Service and a 16-bit UUID for a Custom Value Characteristic.  

```c
#define CUSTOM_SERVICE_UUID               0x1400
#define CUSTOM_VALUE_CHAR_UUID            0x1401
```
The values for the 16-bit UUIDs can pretty much be choosen by random 

- [ ] Look up if the 16-bit UUIDs can be chosen randomly or if any rules apply. 


### Step 2 - Implementing the Custom Service 

First things first, we need to include the ble_custom_service.h header file we just created as well as some common SDK header files in ble_custom_service.c.

```c
#include "sdk_common.h"
#include "ble_srv_common.h"
#include "ble_custom_service.h"
#include <string.h>
```

Ok, so far so good. Now we need to create two structures in ble_custom_service.h, one Custom Service init structure, ble_cus_init_t struct to hold all the options and data needed to initialize our custom service.

```c
/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;
```

The second struct that we need to create is the Custom Service structure, ble_cus_s, which holds the status information of the service. 

```c
/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_s
{
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;           /**< Handles related to the Custom Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};
```

The next step is to add a forward declaration of the ble_cus_t type
```c
// Forward declaration of the ble_cus_t type.
typedef struct ble_cus_s ble_cus_t;
```


The first function we're going to implement is ble_cus_init function, which we're going to initialize our service with. First, we need to do is to add its function decleration in the ble_custom_service.h file. 

```c
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

The first thing we should do upon entering ble_cus_init is to check that none of the pointers that we passed as arguments are NULL and declare the two variables err_code and ble_uuid.

```c
uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;
}
```
After verifying that the pointers are valid we can initialize the Custom Service structure

```c
// Initialize service structure
p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;
```

This consists of setting the connection handle to invalid( should only be valid when we're in a connection). Next, we're going to add our custom (also referred to as vendor specific) base UUID to the BLE stack's table.

```c
// Add Custom Service UUID
ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
VERIFY_SUCCESS(err_code);

ble_uuid.type = p_cus->uuid_type;
ble_uuid.uuid = CUSTOM_SERVICE_UUID;
```

We're almost done, the last thing we have to do is to add the Custom Service decleration to the BLE Stack's GATT table. 

```c
// Add the Custom Service
err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
if (err_code != NRF_SUCCESS)
{
    return err_code;
}
```

If you have followed the steps correctly, then ble_cus_init should look like this.

```c
uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return err_code;
}
```

### Step 3 - Initializing the Service and advertising our 128-bit UUID.

Now it's time to initialize the service in main.c and put our 128-bit UUID in the advertisement packet so that other BLE devices can see that our device has the Custom Service. 

First, add the ble_cus.h file to the include list in main.c

```c
#include "ble_cus.h"
```

and then add the m_cus of the ble_cus_t type below the defines 

```c
static ble_cus_t    m_cus;
``` 

The next step is to find the empty services_init function in main.c, which should look like this

```c
/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    /* YOUR_JOB: Add code to initialize the services used by the application.
       ret_code_t                         err_code;
       ble_xxs_init_t                     xxs_init;
       ble_yys_init_t                     yys_init;

       // Initialize XXX Service.
       memset(&xxs_init, 0, sizeof(xxs_init));

       xxs_init.evt_handler                = NULL;
       xxs_init.is_xxx_notify_supported    = true;
       xxs_init.ble_xx_initial_value.level = 100;

       err_code = ble_bas_init(&m_xxs, &xxs_init);
       APP_ERROR_CHECK(err_code);

       // Initialize YYY Service.
       memset(&yys_init, 0, sizeof(yys_init));
       yys_init.evt_handler                  = on_yys_evt;
       yys_init.ble_yy_initial_value.counter = 0;

       err_code = ble_yy_service_init(&yys_init, &yy_init);
       APP_ERROR_CHECK(err_code);
     */
}
```

Ok, we're going to do as we're told, i.e. create a ble_cus_init_t struct and populate it with the necessary data and then pass it as an argument to our service init function ble_cus_init();

```c
/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    /* YOUR_JOB: Add code to initialize the services used by the application.*/
    ret_code_t                         err_code;
    ble_cus_init_t                     cus_init;

     // Initialize CUS Service init structure to zero.
    memset(&cus_init, 0, sizeof(cus_init));
    
    // Set the cus event handler
    cus_init.evt_handler                = NULL;
	
    err_code = ble_cus_init(&m_cus, &cus_init);
    APP_ERROR_CHECK(err_code);	
}
```
We're setting the event handler to NULL as we have not created a event handler function yet, but do not worry we will do that in one of the steps ahead.Now that we have initialized the service we have to add the 128bit UUID to the advertisement packet. If you navigate to the top of main.c you should find the m_adv_uuids array.

```c
// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */
```

We need to replace the BLE_UUID_DEVICE_INFORMATION_SERVICE with the CUSTOM_SERVICE_UUID we defined in ble_custom_service.h as well as replace  BLE_UUID_TYPE_BLE with BLE_UUID_TYPE_VENDOR_BEGIN since this is a 128-bit vendor specific UUID and not a 16-bit Bluetooth SIG UUDID. m_adv_uuids should now look like this

```c
// YOUR_JOB: Use UUIDs for service(s) used in your application.
 ble_uuid_t m_adv_uuids[] = {{CUSTOM_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN }}; /**< Universally unique service identifiers. */
```
After this step you can try to compile your project, but you'll quickly discover that 
We need to tell the BLE stack that we're using a vendor-specific 128-bit UUID and not a 16-bit bit UUID. This is done by changing the following line in ble_stack_init() 
```c
ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 0
```
to 

```c
ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 1
```
After this change the configuration section of ble_stack_init should look like the snippet below

```c
// Overwrite some of the default configurations for the BLE stack.
ble_cfg_t ble_cfg;

memset(&ble_cfg, 0, sizeof(ble_cfg));
ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 1;
err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
APP_ERROR_CHECK(err_code);
```
Now, adding a vendor-specific UUID to the BLE stack results in the RAM requirement of the SoftDevice increasing, which we need to take into account. Click "Options for Target" in Keil and modify the Read/Write Memory Areas so that IRAM1 has the start address 0x20001FD0 and size 0xE030, as shown in the screenshot below

- [ ] Optional: Add section where the function of app_ram_base since its useful for debugging.
- [ ] Add screenshot of Memory Settings here.

The final step we have to do is to change the calling order in  main() so that services_init() is called before advertising_init(). This is because we need to add the CUSTOM_SERVICE_UUID_BASE to the BLE stack's table using sd_ble_uuid_vs_add() in ble_cus_init() before we call advertising_init(). Doing it the otherway around will cause advertising_init() to return an error code. 

That should be it, compile the ble_app_template project, flash the S132 v4.0.2 SoftDevice and then flash the ble_app_template application. LED1 on your nRF52 DK should now start blinking, indicating that its advertising.  Use nRF Connect for Android/iOS to view the content of the advertisment package. 

### Step 4 - Adding a Custom Value Characteristic to the Custom Service.

A service is nothing with out a characteristic, so lets add one of those by creating the custom_value_char_add function. As this is a function that we want to be public, i.e. available to the application, we need to declare it in the ble_cus.h 


```c
/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);
```
Now, back in ble_cus.c, the first thing we have to do is to add several metadata variables that we will later populate. 


```c
/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
}
```
Now starts the rather tedious part of populating all these variables, we'll start with char_md, which sets the properties that will be displayed to the central during service discovery.

```c
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL; 
    char_md.p_sccd_md         = NULL;
}
```
So we want to be able to both write and read to our Custom Value characteristic, but we do not want to enable the notify property until later. Next we're going to populate the attr_md, which actually sets the properties( i.e. accessability of the attribute).

```c
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
}
```  

The permissions set in the attr_md struct should correspond with the properties set in the characteristic metadata struct char_md. We're going to provide the permissions in the Custom Service init structure that we pass to ble_cus_init in services_init(). The .vloc option is set to BLE_GATTS_VLOC_STACK as we want the characteristic to be stored in the SoftDevice RAM section and not in the Application RAM section. 

The next variable that we have to populate is ble_uuid, which is going to hold our CUSTOM_VALUE_CHAR_UUID and is of the same type as the CUSTOM_SERVICE_UUID_BASE, i.e. vendor specific, which we specified in the .uuid_type field of Custom Service strucure when we added the CUSTOM_SERVICE_UUID_BASE to the BLE stack's table.

```c
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;
```

Next, we're going to populate the attr_char_value struct, which sets the UUID, points to the attribute metadata and sets the size of the characteristic, in our case a single byte(uint8_t). 

```c
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
```

Finally, we're done populating structs and we can add our characteristic by calling sd_ble_gatts_characteristic_add() with the structs as arguments.

```c
err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
```

After all that hard work (copy-pasting) your custom_value_char_add() function should look like this. 

```c
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL; 
    char_md.p_sccd_md         = NULL;
		
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}
```
 The final step is to call custom_value_char_add() at the end of ble_cus_init the service has been added, i.e. 

```c
uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->evt_handler               = p_cus_init->evt_handler;
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    return custom_value_char_add(p_cus, p_cus_init);
}
```


### Step 5 - Handling events from the SoftDevice.
Great, we now have a Custom Service and a Custom Value Characteristic, but we want to be able to write to the characteristic and perform a specific task based on the value that was written to the characteristic, e.g. turn on a LED. However, before we can do that we need to do some event handling in ble_custom_service.h and ble_custom_service.c. 


Lastly, we're going to add the ble_cus_on_ble_evt function decleration, which will handle the events of the ble_cus_evt_type_t from our service.

```c
/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_cus      Custom Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_cus_on_ble_evt(ble_cus_t * p_cus, ble_evt_t * p_ble_evt);
```

We're now going to implement the ble_cus_on_ble_evt event handler in ble_custom_service.c.Upon entry its considered good practice to check that none of the pointers that we provided as arguments are NULL. 

```c
void ble_cus_on_ble_evt(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
}
```

After the NULL check we're going to add a switch-case to check which event that has been propagated to the application by the SoftDevice. 

```c
void ble_cus_on_ble_evt(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            break;

        default:
            // No implementation needed.
            break;
    }
}
```

For now we only need to care about the BLE_GAP_EVT_CONNECTED and BLE_GAP_EVT_DISCONNECTED events. So let's create the two functions on_connect() and on_disconnect(), starting with on_connect(). The only thing we need to do when we get the Connect event is to assign the connection handle in the Custom Service structure to the connection handle that is passed with the event. 

```c
/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}
```

Similarly, when we get the Disconnect event, the only thing we need to do is invalidate the connection handle in the Custom Service structure since the connection is now dead.

```c
/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
}
```

Now that we have one function for each event we only need to call the function in ble_cus_on_ble_evt, i.e.

```c
void ble_cus_on_ble_evt(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_cus, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
```

The last thing we have to do is make sure that the application dispatches the SoftDevice events to our ble_cus_on_ble_evt event handler function. This is done by adding the function call to the ble_evt_dispatch() function in main.c

```c
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    /** The Connection state module has to be fed BLE events in order to function correctly
     * Remember to call ble_conn_state_on_ble_evt before calling any ble_conns_state_* functions. */
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);
   
    //YOUR_JOB add calls to _on_ble_evt functions from each service your application is using
    ble_cus_on_ble_evt(&m_cus, p_ble_evt);
    
}
```

### Step 6 - Handling the Write event from the SoftDevice.

 Ok, now we really want to be able to write to the characteristic and perform a specific task based on the value that was written to the characteristic, e.g. turn on a LED. How are we going to that? You guessed it! More event handling!

 Whenever a characteristic is written to, a BLE_GATTS_EVT_WRITE event will be propagated to the application and dispatched to the functions in ble_evt_dispatch(). So this means that we need to add another case to our ble_cus_on_ble_evt switch-case statement, namely BLE_GATTS_EVT_WRITE


 ```c
void ble_cus_on_ble_evt(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_cus, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            break;
        default:
            // No implementation needed.
            break;
    }
}
```

Just like we did for the BLE_GAP_EVT_CONNECTED and BLE_GAP_EVT_DISCONNECTED we're going to create a on_write function that should be called when we get the Write event. 

```c
/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{

}
```

Now, once we get the Write event we have to get hold of the Write event parameters that are passed with the event and we have to verify that the the handle that is written to matches the Custom Value Characteristic handle, i.e.

```c
static void on_write(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Check if the handle passed with the event matches the Custom Value Characteristic handle.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        // Put specific task here. 
    }
```

So lets say that our specifc task is to toggle a LED on the nRF5x DK every time the Custom Value Characteristic is written to. We can do this by calling nrf_gpio_pin_toggle on one of the pins connected to the nRF5x DK LEDs, e.g. LED4. In order to do this we'll have to include boards.h and nrf_gpio.h in ble_cus.h as well as call nrf_gpio_pin_toggle in the on_write function


```c
static void on_write(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Check if the handle passed with the event matches the Custom Value Characteristic handle.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        nrf_gpio_pin_toggle(LED_4); 
    }
```

We'we now implemented the necessary event handling, but we have to allow the peer to actually write and/or read from the characteristic value. This is done by adding two lines to services_init() in main.c before we call ble_cus_init(). 

```c
BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_init.custom_value_char_attr_md.read_perm);
BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_init.custom_value_char_attr_md.write_perm);
```

These two lines sets the write and read permissions to the characteristic value attribute to open, i.e. the peer is allowed to write/read the value without encrypting the link first. Now, try writting to the characteristic using nRF Connect for Desktop or Android/iOS. Every time a value is written to the characteristic, LED4 on the nRF5x Dk should toogle. 

- [ ] Add screenshot from nRF Connect. 

Challenge 1: p_evt_write also has a data field. Use the data to decide if the LED is to be turned on or off. 

### Step 7 - Propagating Custom Service events to the application

Until now we've only handled the events that are propagated by the SoftDevice, but in some cases it makes sense to propagate events to the application. 

In order to do this we need to add an event handler to our Custom Service Init structure and Custom Service structure

```c
/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_s
{
    ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;           /**< Handles related to the Custom Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};
```

After adding the event handler to the structures we must make sure that we initialize our service correctly in ble_cus_init()

```c
// Initialize service structure
p_cus->evt_handler               = p_cus_init->evt_handler;
p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;
```

Next, we need to declare an event type specific to our service

```c
typedef enum
{
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED
} ble_cus_evt_type_t;
```
For now we're only going to add the BLE_CUS_EVT_CONNECTED and BLE_CUS_EVT_DISCONNECTED events, but we'll add some additional events later in the tutorial. 

After declaring the event type we need to declare an event structure that holds a ble_cus_evt_type_t event, i.e. 

```c
/**@brief Custom Service event. */
typedef struct
{
    ble_cus_evt_type_t evt_type;                                  /**< Type of event. */
} ble_cus_evt_t;
```

 Next, we need declare the Custom Service event handler type 

```c
/**@brief Custom Service event handler type. */
typedef void (*ble_cus_evt_handler_t) (ble_cus_t * p_cus, ble_cus_evt_t * p_evt);
```

Now, back in main we're going to create the event handler function on_cus_evt, which takes the same parameters as the ble_cus_evt_handler_t type.

```c
/**@brief Function for handling the Custom Service Service events.
 *
 * @details This function will be called for all Custom Service events which are passed to
 *          the application.
 *
 * @param[in]   p_cus_service  Custom Service structure.
 * @param[in]   p_evt          Event received from the Custom Service.
 *
 */
static void on_cus_evt(ble_cus_t     * p_cus_service,
                       ble_cus_evt_t * p_evt)
{
    switch(p_evt->evt_type)
    {
        case BLE_CUS_EVT_CONNECTED:
            break;

        case BLE_CUS_EVT_DISCONNECTED:
              break;

        default:
              // No implementation needed.
              break;
    }
}
```

Now, in order to propagate events from our service we need to assign the on_cus_evt function as the event handler function of our service when we initialize the Custom Service. This is done by setting the .evt_handler field of the cus_init struct equal to on_cus_evt, i.e.

```c
    // Set the cus event handler
    cus_init.evt_handler                = on_cus_evt;
```

We can now invoke this event handler from ble_custom_service.c by calling p_cus->evt_handler(p_cus, &evt) and as an example we'll invoke the event handler when we get the BLE_GAP_EVT_CONNECTED event, i.e. in the on_connect() function. It's fairly straight forward,  we simply declare a ble_cus_evt_t variable and set its .evt_type field to the BLE_CUS_EVT_CONNECTED and then invoke the event handler with said event. 

```c
static void on_connect(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_CONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}
```

### Step 8 - Notifying the Custom Value Characteristic

Next, we're going to add the ble_cus_custom_value_update function decleration to the ble_custom_service.h file, which we're going to use to update our Custom Value Characteristic.

```c
/**@brief Function for updating the custom value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value);
```
Back in ble_custom_service.c, we're going to continue the good practice of checking that the pointer we passed as an argument is'nt NULL

```c
uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value){
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }
}
```

Next, we're going to update the value in the GATT table with our custom_value that we passed as an argument to the ble_cus_custom_value_update() function. 

```c
uint32_t err_code = NRF_SUCCESS;
ble_gatts_value_t gatts_value;

// Initialize value struct.
memset(&gatts_value, 0, sizeof(gatts_value));

gatts_value.len     = sizeof(uint8_t);
gatts_value.offset  = 0;
gatts_value.p_value = &custom_value;

// Update database.
err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                    p_cus->custom_value_handles.value_handle,
                                    &gatts_value);
if (err_code != NRF_SUCCESS)
{
    return err_code;
}
```

After updating the value in the GATT table we're ready to notify our peer that the value of our Custom Value Characteristic has changed. 

```c
// Send value if connected and notifying.
if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
{
    ble_gatts_hvx_params_t hvx_params;

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_cus->custom_value_handles.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = gatts_value.offset;
    hvx_params.p_len  = &gatts_value.len;
    hvx_params.p_data = gatts_value.p_value;

    err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
}
else
{
    err_code = NRF_ERROR_INVALID_STATE;
}

return err_code;

```
The first thing we should do is to verify that we have a valid connection handle, i.e. that we're actually connected to a peer, if not we should return an error indicating that we're in an invalid state. Next, we set the ble_gatts_hvx_params_t, which contains the handle of custom value attribute, the type(i.e. if its a Notification or Indication), the value offsett( only used if its larger than 20bytes), the length and the data. After setting the hvx_params, we notify the peer by calling sd_ble_gatts_hvx(). Lastly, we should return the error code. 

After adding the code snippets above ble_cus_custom_value_update() should look like below

```c
uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value)
{
    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n"); 
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->custom_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->custom_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }


    return err_code;
}
```

Like the characteristic metadata we need to set the metadata of the CCCD in custom_value_char_add(), which is done by adding the following snippet before the characteristic metadata is set. 

```c
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
```
We're setting the read and write permissions to the CCCD to open, i.e. no encryption is needed to write or read from the CCCD. 

The next step is to add the Notify property to the Custom Value Characteristic and add a pointer to  the CCCD metadata in the characteristic metadata. This is done by modifying  the characteristic metadata properties in custom_value_char_add() function , i.e. setting .notify to 1 and .p_cccd_md to point to the CCCD metadata struct. 

```c
    char_md.char_props.notify = 1;  
    char_md.p_cccd_md         = &cccd_md; 
```

This will add a Client Characteristic Configuration Descriptor or CCCD to the Custom Value Characteristic which allows us to enable or disable notifications by writing to the CCCD. Notification is by default disabled and in order to enable it we have to write 0x0001 to the CCCD. Remember that everytime we write to a characteristic or one of its descriptors, we will get a Write event, thus we need to handle the case where a peer writes to the CCCD in the on_write() function. 

However, before modifying the on_write() function we need do is to add an additional event, BLE_CUS_EVT_NOTIFICATION_ENABLED, to the ble_cus_evt_type_t enumeration in ble_custom_service.h

```c
/**@brief Custom Service event type. */
typedef enum
{
    BLE_CUS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED
} ble_cus_evt_type_t;
```

Back to the on_write() function, the first thing we should do is to add a if-statement that checks if the handle that is written to matches the handle of the CCCD and that the value has the correct length. If we pass this check we need to check wether notifications has been enabled or not. 

```c
    // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    if ((p_evt_write->handle == p_cus->custom_value_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {

        // CCCD written, call application event handler
        if (p_cus->evt_handler != NULL)
        {
            ble_cus_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_CUS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_CUS_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_cus->evt_handler(p_cus, &evt);
        }

    }
```

We do not actually need to enable notifications as this is done automatically by the SoftDevice. However, the SoftDevice will propagate the write event and based on this event we can decide if its ok to start notifying characteristic values or not. 

Adding the code-snippet above to the on_write() function should result in the following function. 

```c
static void on_write(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Custom Value Characteristic Written to.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        nrf_gpio_pin_toggle(LED_4);
    }

    // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    if ((p_evt_write->handle == p_cus->custom_value_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        // CCCD written, call application event handler
        if (p_cus->evt_handler != NULL)
        {
            ble_cus_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_CUS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_CUS_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_cus->evt_handler(p_cus, &evt);
        }
    }
}
```

Now the last thing we have to do is to add the BLE_CUS_EVT_NOTIFICATION_ENABLED to the on_cus_evt() event handler in main.c and call ble_cus_custom_value_update(), i.e. 

```c
static void on_cus_evt(ble_cus_t     * p_cus_service,
                       ble_cus_evt_t * p_evt)
{
    ret_code_t err_code;
    uint8_t custom_value = 8;

    switch(p_evt->evt_type)
    {
        case BLE_CUS_EVT_NOTIFICATION_ENABLED:
            err_code = ble_cus_custom_value_update(p_cus_service, custom_value);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_CUS_EVT_CONNECTED :
            break;

        case BLE_CUS_EVT_DISCONNECTED:
            break;

        default:
              // No implementation needed.
              break;
    }
}
```
