#include "sdk_common.h"
#include "ble_cus.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_DISCONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_bas       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_cus_t * p_cus, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Custom Value Characteristic Written
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        if(*p_evt_write->data == 0x01)
        {
            nrf_gpio_cfg_output(20);
            nrf_gpio_pin_clear(20); 
        }
        else if(*p_evt_write->data == 0x02)
        {
            nrf_gpio_pin_set(20); 
        }
        else
        {
          //Do nothing
        }
    }


    if (p_cus->is_notification_supported)
    {
        if (
            (p_evt_write->handle == p_cus->custom_value_handles.cccd_handle)
            &&
            (p_evt_write->len == 2)
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

                p_cus->evt_handler(p_cus, &evt);
            }
        }
    }
}

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
            on_write(p_cus, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_bas        Battery Service structure.
 * @param[in]   p_bas_init   Information needed to initialize the service.
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
    uint8_t             initial_custom_value;
    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
    uint8_t             init_len;

    // Add Custom Value characteristic
    if (p_cus->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        // According to BAS_SPEC_V10, the read operation on cccd should be possible without
        // authentication.
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
        cccd_md.write_perm = p_cus_init->battery_level_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = (p_cus->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_cus->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;
		
    //BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BATTERY_LEVEL_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->battery_level_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->battery_level_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_custom_value = p_cus_init->initial_custom_value;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = &initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
/*
    if (p_cus_init->p_report_ref != NULL)
    {
        // Add Report Reference descriptor
        BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_REPORT_REF_DESCR);

        memset(&attr_md, 0, sizeof(attr_md));

        attr_md.read_perm = p_cus_init->battery_level_report_read_perm;
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

        attr_md.vloc    = BLE_GATTS_VLOC_STACK;
        attr_md.rd_auth = 0;
        attr_md.wr_auth = 0;
        attr_md.vlen    = 0;

        init_len = ble_srv_report_ref_encode(encoded_report_ref, p_cus_init->p_report_ref);

        memset(&attr_char_value, 0, sizeof(attr_char_value));

        attr_char_value.p_uuid    = &ble_uuid;
        attr_char_value.p_attr_md = &attr_md;
        attr_char_value.init_len  = init_len;
        attr_char_value.init_offs = 0;
        attr_char_value.max_len   = attr_char_value.init_len;
        attr_char_value.p_value   = encoded_report_ref;

        err_code = sd_ble_gatts_descriptor_add(p_cus->custom_value_handles.value_handle,
                                               &attr_char_value,
                                               &p_cus->report_ref_handle);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        p_cus->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }
		*/
    return NRF_SUCCESS;
}

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
    p_cus->is_notification_supported = p_cus_init->support_notification;
    p_cus->custom_value_last         = 0;

    // Add service
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;
    //BLE_UUID_BLE_ASSIGN(ble_uuid, CUSTOM_SERVICE_UUID_BASE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom level characteristic
    return custom_value_char_add(p_cus, p_cus_init);
}

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value)
{
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    if (custom_value != p_cus->custom_value_last)
    {
        // Initialize value struct.
        memset(&gatts_value, 0, sizeof(gatts_value));

        gatts_value.len     = sizeof(uint8_t);
        gatts_value.offset  = 0;
        gatts_value.p_value = &custom_value;

        // Update database.
        err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                          p_cus->custom_value_handles.value_handle,
                                          &gatts_value);
        if (err_code == NRF_SUCCESS)
        {
            // Save new battery value.
            p_cus->custom_value_last = custom_value;
        }
        else
        {
            return err_code;
        }

        // Send value if connected and notifying.
        if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID) && p_cus->is_notification_supported)
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
    }

    return err_code;
}
