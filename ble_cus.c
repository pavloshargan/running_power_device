#include "sdk_common.h"
#include "ble_cus.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */

static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_CONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */

static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);

    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_DISCONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Custom Value Characteristic Written to.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
 
    }
    if (p_evt_write->handle == p_cus->package_idx_handles.value_handle)
    {
        
        ble_gatts_value_t rx_data;
        rx_data.len = sizeof(p_cus->package_idx);
        rx_data.offset = 0;
        rx_data.p_value = (uint8_t*)&(p_cus->package_idx);

        sd_ble_gatts_value_get(p_cus->conn_handle, p_cus->package_idx_handles.value_handle, &rx_data);
        
        for(int i=0;i<9;i++)
        {
            p_cus->package[i] = p_cus->accl_arr[p_cus->package_idx*9 + i];
        }
        
        p_cus->package[9] = p_cus->package_idx;

        ble_gatts_value_t tx_data;
        tx_data.len = sizeof(p_cus->package);
        tx_data.offset = 0;
        tx_data.p_value = (uint8_t*)&(p_cus->package);

        sd_ble_gatts_value_set(p_cus->conn_handle, p_cus->package_handles.value_handle, &tx_data); 
        
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

void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_cus_t * p_cus = (ble_cus_t *) p_context;
    
    NRF_LOG_INFO("BLE event received. Event type = %d\r\n", p_ble_evt->header.evt_id); 
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
/* Handling this event is not necessary
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            NRF_LOG_INFO("EXCHANGE_MTU_REQUEST event received.\r\n");
            break;
*/
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

static uint32_t power_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = POWER_CHAR_UUID;
    

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 2;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 2;
    attr_char_value.p_value     = (uint8_t*)&p_cus->power;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->power_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

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

static uint32_t custom_value2_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID2;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 2;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 2;
    attr_char_value.p_value     = (uint8_t*)&p_cus->acc_x;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles_2);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}
static uint32_t pakage_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = PACKAGE_CHAR_UUID;
    

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 20;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 20;
    attr_char_value.p_value     = (uint8_t*)&p_cus->package;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->package_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

static uint32_t pakage_idx_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = PACKAGE_IDX_CHAR_UUID;
    

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 2;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 2;
    attr_char_value.p_value     = (uint8_t*)&p_cus->package_idx;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->package_idx_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}
void ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->evt_handler               = p_cus_init->evt_handler;
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;

    // Add the Custom Service
    sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    
    
    custom_value2_char_add(p_cus, p_cus_init);
    custom_value_char_add(p_cus, p_cus_init);
    pakage_char_add(p_cus, p_cus_init);
    pakage_idx_char_add(p_cus, p_cus_init);
    power_char_add(p_cus, p_cus_init);
}

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
        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code); 
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n"); 
    }


    return err_code;
}

void power_update(ble_cus_t * p_cus)
{
    // double pow_avg = 0;
    // int x,y,z;
    // int buffer_counter = p_cus->buff_counter;
    // int x_idx, y_idx, z_idx;
    // x_idx = (149*3)%(450-buffer_counter);
    // y_idx = (149*3+1)%(450-buffer_counter);
    // z_idx = (149*3+2)%(450-buffer_counter);
    // x=p_cus->buff[x_idx];
    // y=p_cus->buff[y_idx];
    // z=p_cus->buff[z_idx];
    // pow_avg = pow_avg + sqrt(x*x+y*y+z*z)/150;
    // p_cus->power = (uint16_t)(pow_avg);


    // int acc_full[150]; 
    // int diff_z_and_full[150];
    // int x,y,z;
    // int signal_max = 0;
    // int buffer_counter = p_cus->buff_counter;
    // int x_idx, y_idx, z_idx;
    // int temp_buff[450];
    // for(int j = 0; j<450; j++)
    //     temp_buff[j] = p_cus->buff[j];
    // for(int i=0; i<150; i++)
    // {
    //    x_idx = (i*3)%(450-buffer_counter);
    //    y_idx = (i*3+1)%(450-buffer_counter);
    //    z_idx = (i*3+2)%(450-buffer_counter);
    //    x=temp_buff[x_idx];
    //    y=temp_buff[y_idx];
    //    z=temp_buff[z_idx];
    //    acc_full[i]=sqrt(x*x+y*y+z*z);
    //    diff_z_and_full[i]=abs(z-acc_full[i]);
    //    if(diff_z_and_full[i]>signal_max)
    //         signal_max = diff_z_and_full[i];
    // }
    // for(int j=0;j<149;j++)
    //     diff_z_and_full[j] = (int)((diff_z_and_full[j+1]+diff_z_and_full[j])/2);

    // int peak_coords[15];
    // int peak_coords_size = 0;
    // double peaks_coords_dispersion = 10000;
    // for(int y = signal_max; y>=0; y-=50)
    // {
    //     int intersection_idxs[15];
    //     int intersection_idxs_size = 0;
    //     int count = 0;
    //     for(int i=0; i<149;i++)
    //     {
    //         if((diff_z_and_full[i+1]>y) != (diff_z_and_full[i]>y))
    //         {
    //             if(count%2==0)
    //             {
    //                 intersection_idxs[intersection_idxs_size] = i;
    //                 intersection_idxs_size++;
    //             }
    //             count++;
    //         }
    //     }
    //     if(intersection_idxs_size < 4)
    //         continue;
    //     int diffs[intersection_idxs_size-1];
    //     double diffs_mean = 0;
    //     for(int i=0;i<intersection_idxs_size-1;i++)
    //     {
    //         diffs[i] = intersection_idxs[i+1] - intersection_idxs[i];
    //         diffs_mean+=diffs[i];
    //     }
    //     diffs_mean = diffs_mean/(intersection_idxs_size-1);
    //     double dispersion = 0;
    //     double mean_deviation;
    //     for(int i=0;i<intersection_idxs_size-1;i++)
    //     {
    //         mean_deviation = (diffs[i]-diffs_mean);
    //         dispersion = dispersion + mean_deviation*mean_deviation;
    //     }
    //     dispersion = sqrt(dispersion/(intersection_idxs_size-1));
    //     double stride_3_power = 0;
    //     if(dispersion>peaks_coords_dispersion)
    //     {
    //         for(int i=0;i<3;i++)
    //         {
    //             double one_stride_power = 0;
    //             for(int j=peak_coords[peak_coords_size-i-2];j<peak_coords[peak_coords_size-i-1];j++)
    //             {
    //                 one_stride_power = one_stride_power+acc_full[j];
    //             }
    //             one_stride_power = one_stride_power/(peak_coords[peak_coords_size-i-1]-peak_coords[peak_coords_size-i-2]);
    //             stride_3_power=stride_3_power+one_stride_power;
    //         }
    //         p_cus->power = (uint16_t)(stride_3_power/3);
    //         break;
    //     }
    //     else
    //     {
    //         for(int j=0;j<15;j++)
    //             peak_coords[j]=intersection_idxs[j];
    //         peak_coords_size = intersection_idxs_size;
    //         peaks_coords_dispersion = dispersion;
    //     }
    //     p_cus->power = (uint16_t)(stride_3_power/3);

    // }

    double last_50_avg_pow=0;
    for(int i=0;i<50;i++)
        last_50_avg_pow = last_50_avg_pow + p_cus->pow_buf[(p_cus->pow_buf_counter+i)%50];
    last_50_avg_pow = last_50_avg_pow/50;
    p_cus->power = (uint16_t)(last_50_avg_pow);

    ble_gatts_hvx_params_t hvx_params;
    uint16_t len = sizeof(p_cus->power);
    hvx_params.handle		= p_cus->power_handles.value_handle;
    hvx_params.type	    	= BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset		= 0;
    hvx_params.p_len		= &len;
    hvx_params.p_data		= (uint8_t*)&(p_cus->power);
    sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
}
void package_update(ble_cus_t * p_cus)
{
    ble_gatts_hvx_params_t hvx_params;
    uint16_t len = sizeof(p_cus->package);
    hvx_params.handle		= p_cus->package_handles.value_handle;
    hvx_params.type	    	= BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset		= 0;
    hvx_params.p_len		= &len;
    hvx_params.p_data		= (uint8_t*)&(p_cus->package);
    sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);

}
