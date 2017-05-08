#ifndef BLE_CUS_H__
#define BLE_CUS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

// CUSTOM_SERVICE_UUID_BASE f364adc9-b000-4042-ba50-05ca45bf8abc

#define CUSTOM_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}

#define CUSTOM_SERVICE_UUID               0x1400
#define CUSTOM_VALUE_CHAR_UUID            0x1401
																					
/**@brief Custom Service event type. */
typedef enum
{
    BLE_CUS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_CUS_EVT_NOTIFICATION_DISABLED,                             /**< Custom value notification disabled event. */
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED
} ble_cus_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_cus_evt_type_t evt_type;                                  /**< Type of event. */
} ble_cus_evt_t;

// Forward declaration of the ble_cus_t type.
typedef struct ble_cus_s ble_cus_t;


/**@brief Custom Service event handler type. */
typedef void (*ble_cus_evt_handler_t) (ble_cus_t * p_bas, ble_cus_evt_t * p_evt);

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

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
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


uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value);

#endif // BLE_CUS_H__
