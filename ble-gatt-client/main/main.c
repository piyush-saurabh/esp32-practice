#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

// Make ESP32 connect to the BLE device with this name
#define DEVICE_NAME "CHIP2CLOUD"

uint8_t ble_addr_type;
void ble_app_scan(void);

// callback after getting attributes
int get_attr(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg)
{
    if(error->status ==0)
    {
        printf("got char value %.*s\n", attr->om->om_len, attr->om->om_data);
    }
    return error->status;
}

// callback after getting characteristics
int get_all_chars(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg)
{
    static uint16_t attr_handller;
    char buffer[50];
    switch (error->status)
    {
    case BLE_HS_EDONE:
        // read the characteristics
        ble_gattc_read(conn_handle, attr_handller, get_attr, NULL);
        break;
    case 0:
        memset(buffer, 0, sizeof(buffer));
        ble_uuid_to_str(&chr->uuid.u, buffer);
        ESP_LOGI("CHAR", "CHAR %s", buffer);
        if (strcmp(buffer, "0x2222") == 0)
        {
            attr_handller = chr->val_handle;
        }
        break;
    default:
        break;
    }

    return error->status;
}

// Callback function for discovering service
int service_disc(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_svc *service, void *arg)
{
    static uint16_t start, end;

    // service uuid which is discovered
    char buffer[50];

    switch (error->status)
    {   
    case BLE_HS_EDONE:
        // after finishing the emumeration of th services 

        // Get all the characterstics
        ble_gattc_disc_all_chrs(conn_handle, start, end, get_all_chars, NULL);

        break;
    case 0:
        // fill the buffer used for service discovery uuid
        memset(buffer, 0, sizeof(buffer));

        // fill the buffer with service uuid
        ble_uuid_to_str(&service->uuid.u, buffer);

        ESP_LOGI("SERVICE", "Service %s", buffer);

        // 
        if (strcmp(buffer, "0x1111") == 0)
        {
            start = service->start_handle;
            end = service->end_handle;
        }
        break;
    default:
        break;
    }

    return error->status;
}

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    // used to get the information advertised by other devices
    struct ble_hs_adv_fields fields;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
        // parse the fields advertised by other devices
        ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);

        // Print the name of the device being advertised
        printf("discovered device %.*s\n", fields.name_len, fields.name);

        // Connect to the device with the above device name
        if (fields.name_len == strlen(DEVICE_NAME) && memcmp(fields.name, DEVICE_NAME, strlen(DEVICE_NAME)) == 0)
        {
            printf("device found\n");

            // After connecting to the device, cancel the process of scanning
            ble_gap_disc_cancel();
            
            ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr, 1000, NULL, ble_gap_event, NULL); // callback function after connecting
        }
        break;
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_CONNECT %s", event->connect.status == 0 ? "OK" : "Failed");
        if (event->connect.status == 0)
        {
            // After connecting to the GATT server, discover all the available characteristics (gattc)
            // provide the callback function
            ble_gattc_disc_all_svcs(event->connect.conn_handle, service_disc, NULL);
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_DISCONNECT");
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_ADV_COMPLETE");
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_SUBSCRIBE");
        break;
    default:
        break;
    }
    return 0;
}

// Scan for BLE
void ble_app_scan(void)
{
    // Information about how we want to scan
    // BLE GAP discovery parameter
    // More details https://mynewt.apache.org/v1_5_0/mynewt_faq/bluetooth_faq.html#documentation-for-ble-gap-disc-params
    struct ble_gap_disc_params ble_gap_disc_params;

    ble_gap_disc_params.filter_duplicates = 1; // don't pick same device multipe times
    ble_gap_disc_params.passive = 1;
    ble_gap_disc_params.itvl = 0;
    ble_gap_disc_params.window = 0;
    ble_gap_disc_params.filter_policy = 0;
    ble_gap_disc_params.limited = 0;

    // Start the scanning/discovery process
    ble_gap_disc(ble_addr_type, BLE_HS_FOREVER, &ble_gap_disc_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_scan();
}

void host_task(void *param)
{
    nimble_port_run();
}

void app_main(void)
{
    nvs_flash_init();

    esp_nimble_hci_and_controller_init();
    nimble_port_init();

    ble_svc_gap_device_name_set(DEVICE_NAME);
    ble_svc_gap_init();

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}