/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.h"
#include "common.h"
#include "led.h"
#include "scoreboard.h"
#include "string.h"

/* Private function declarations */
static int scbd_rate_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg);
static int scbd_write_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg);

/* Private variables */


//static uint8_t scbd_chr_val[2] = {0};
static uint16_t scbd_chr_val_handle;


static uint16_t scbd_chr_conn_handle = 0;
static bool scbd_chr_conn_handle_inited = false;
static bool scbd_ind_status = false;

/* Automation IO service */
//static const ble_uuid16_t auto_io_svc_uuid = BLE_UUID16_INIT(0x1815);
static uint16_t led_chr_val_handle;
/*static const ble_uuid128_t led_chr_uuid =
    BLE_UUID128_INIT(0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef,
                     0x12, 0x12, 0x25, 0x15, 0x00, 0x00);*/
static const ble_uuid128_t scoreboard_svc_base_uuid =
    BLE_UUID128_INIT(0x57, 0x45, 0xc8, 0x16, 0xe0, 0xe1, 0x42, 0x1d,
                     0xaa, 0x25, 0x4a, 0x61, 0xad, 0x22, 0x29, 0x0a);

static const ble_uuid128_t scoreboard_svc_read_indicate_char_uuid = 
    BLE_UUID128_INIT(0x57, 0x45, 0xc8, 0x16, 0xe0, 0xe1, 0x42, 0x1d,
                     0xaa, 0x25, 0x4a, 0x61, 0xad, 0x22, 0x8B, 0x2F);

static const ble_uuid128_t scoreboard_svc_write_char_uuid = 
    BLE_UUID128_INIT(0x57, 0x45, 0xc8, 0x16, 0xe0, 0xe1, 0x42, 0x1d,
                     0xaa, 0x25, 0x4a, 0x61, 0xad, 0x22, 0xD6, 0x4E);

/* GATT services table */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /* Scoreboard service */
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &scoreboard_svc_base_uuid.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {/* Scoreboard characteristic */
              .uuid = &scoreboard_svc_read_indicate_char_uuid.u,
              .access_cb = scbd_rate_chr_access,
              .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
              .val_handle = &scbd_chr_val_handle},
              {.uuid = &scoreboard_svc_write_char_uuid.u,
                .access_cb = scbd_write_chr_access,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &led_chr_val_handle},
             {
                 0, /* No more characteristics in this service. */
             }}},

    /* Automation IO service 
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &auto_io_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){ LED characteristic 
                                        {.uuid = &led_chr_uuid.u,
                                         .access_cb = led_chr_access,
                                         .flags = BLE_GATT_CHR_F_WRITE,
                                         .val_handle = &led_chr_val_handle},
                                        {0}},
    },*/

    {
        0, /* No more services. */
    },
};

/* Private functions */
static int scbd_rate_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg) {
    /* Local variables */
    int rc;
    char* greetings = get_score_str();

    /* Handle access events */
    /* Note: Scoreboard characteristic is read only */
    switch (ctxt->op) {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == scbd_chr_val_handle) {
            /* Update access buffer value */
            ESP_LOGI(TAG, "%s \n", greetings);
            rc = os_mbuf_append(ctxt->om, greetings,
                                strlen(greetings));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(
        TAG,
        "unexpected access operation scoreboard characteristic, opcode: %d",
        ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

static int scbd_write_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
    /* Local variables */
    //int rc;
    printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);

    char * data = (char *)ctxt->om->om_data;

    char local_buf[64];  // pick a size larger than your maximum expected command
    uint16_t len = ctxt->om->om_len;

    // Copy up to local_buf size - 1, then null terminate
    size_t copy_len = (len < sizeof(local_buf) - 1) ? len : (sizeof(local_buf) - 1);
    memcpy(local_buf, ctxt->om->om_data, copy_len);
    local_buf[copy_len] = '\0';

    if (strcmp(local_buf, (char *)"INC CHALLENGER\0")==0){
        increment_score(challenger);
        ESP_LOGI(TAG, "INCREMENTED CHALLENGER");
    }else if(strcmp(local_buf, (char *)"INC DEFENDER\0") == 0){
        increment_score(defender);
        ESP_LOGI(TAG, "INCREMENTED DEFENDER");
    }else if(strcmp(local_buf, (char *)"RESET\0") == 0){
        initalize_scoreboard();
        ESP_LOGI(TAG, "SCOREBOARD RESET");
    }else if(strcmp(local_buf, (char *)"RESET SCORE\0") == 0){
        reset_score();
        ESP_LOGI(TAG, "SCORE RESET");
    }else if(strcmp(local_buf, (char *)"SET DOUBLES\0") == 0){
        set_match_type(doubles);
        ESP_LOGI(TAG, "MATCH TYPE SET TO DOUBLES");
    }else if(strcmp(local_buf, (char *)"SET SINGLES\0") == 0){
        set_match_type(singles);
        ESP_LOGI(TAG, "MATCH TYPE SET TO SINGLES");
    }else if(strcmp(local_buf, (char *)"SET BO3\0") == 0){
        set_number_of_sets_to_play(BO3);
        ESP_LOGI(TAG, "MATCH SET TO BO3");
    }else if(strcmp(local_buf, (char *)"SET BO5\0") == 0){
        set_number_of_sets_to_play(BO5);
        ESP_LOGI(TAG, "MATCH SET TO BO5");
    }else if(strcmp(local_buf, (char *)"SET BO7\0") == 0){
        set_number_of_sets_to_play(BO7);
        ESP_LOGI(TAG, "MATCH SET TO BO7");
    }else if(strcmp(local_buf, (char *)"SET OFFICIAL\0") == 0){
        set_official_status(official);
        ESP_LOGI(TAG, "THE MATCH IS NOW OFFICIAL");
    }else if(strcmp(local_buf, (char *)"SET UNOFFICIAL\0") == 0){
        set_official_status(not_official);
        ESP_LOGI(TAG, "THE MATCH IS NOW UNOFFICIAL");
    }

    print_current_score();
    send_scoreboard_indication();
    return 0;
}

/* Public functions */
void send_scoreboard_indication(void) {
    if (scbd_ind_status && scbd_chr_conn_handle_inited) {
        ble_gatts_indicate(scbd_chr_conn_handle,
                           scbd_chr_val_handle);
        ESP_LOGI(TAG, "scoreboard indication sent!");
    }
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}

/*
 *  GATT server subscribe event callback
 *      1. Update scoreboard subscription status
 */

void gatt_svr_subscribe_cb(struct ble_gap_event *event) {
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
        ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
                 event->subscribe.attr_handle);
    }

    /* Check attribute handle */
    if (event->subscribe.attr_handle == scbd_chr_val_handle) {
        /* Update scoreboard subscription status */
        scbd_chr_conn_handle = event->subscribe.conn_handle;
        scbd_chr_conn_handle_inited = true;
        scbd_ind_status = event->subscribe.cur_indicate;
    }
}

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void) {
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
