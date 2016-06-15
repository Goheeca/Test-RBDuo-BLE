/* 
 * BLE peripheral preferred connection parameters:
 *     - Minimum connection interval = MIN_CONN_INTERVAL * 1.25 ms, where MIN_CONN_INTERVAL ranges from 0x0006 to 0x0C80
 *     - Maximum connection interval = MAX_CONN_INTERVAL * 1.25 ms,  where MAX_CONN_INTERVAL ranges from 0x0006 to 0x0C80
 *     - The SLAVE_LATENCY ranges from 0x0000 to 0x03E8
 *     - Connection supervision timeout = CONN_SUPERVISION_TIMEOUT * 10 ms, where CONN_SUPERVISION_TIMEOUT ranges from 0x000A to 0x0C80
 */
#define MIN_CONN_INTERVAL          0x0028 // 50ms.
#define MAX_CONN_INTERVAL          0x0190 // 500ms.
#define SLAVE_LATENCY              0x0000 // No slave latency.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s.

// https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_GENERIC_REMOTE_CONTROL

#define BLE_DEVICE_NAME            "GoheecusBLE"

// Length of characteristic value.
#define RGB_CHARACTERISTIC_MAX_LEN    3

/******************************************************
 *               Variable Definitions
 ******************************************************/
// Services: https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx
// Generic Access: https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.generic_access.xml
// Device Information: https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.device_information.xml
// https://www.uuidgenerator.net/version1
// Primary service 128-bits UUID // 5248ccfc-3290-11e6-ac61-9e71128cae77
static uint8_t rgb_service_uuid[16] = { 0x52,0x48,0xcc,0xfc,0x32,0x90,0x11,0xe6,0xac,0x61,0x9e,0x71,0x12,0x8c,0xae,0x77 };
// Characteristics 128-bits UUID // ecc0e918-3290-11e6-ac61-9e71128cae77
static uint8_t rgb_char_uuid[16]    = { 0xec,0xc0,0xe9,0x18,0x32,0x90,0x11,0xe6,0xac,0x61,0x9e,0x71,0x12,0x8c,0xae,0x77 };


// GAP and GATT characteristics value
static uint8_t  appearance[2] = { 
  LOW_BYTE(BLE_PERIPHERAL_APPEARANCE), 
  HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE) 
};

static uint8_t  change[4] = {
  0x00, 0x00, 0xFF, 0xFF
};

static uint8_t  conn_param[8] = {
  LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL), 
  LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL), 
  LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY), 
  LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

/* 
 * BLE peripheral advertising parameters:
 *     - advertising_interval_min: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
 *     - advertising_interval_max: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
 *     - advertising_type: 
 *           BLE_GAP_ADV_TYPE_ADV_IND 
 *           BLE_GAP_ADV_TYPE_ADV_DIRECT_IND 
 *           BLE_GAP_ADV_TYPE_ADV_SCAN_IND 
 *           BLE_GAP_ADV_TYPE_ADV_NONCONN_IND
 *     - own_address_type: 
 *           BLE_GAP_ADDR_TYPE_PUBLIC 
 *           BLE_GAP_ADDR_TYPE_RANDOM
 *     - advertising_channel_map: 
 *           BLE_GAP_ADV_CHANNEL_MAP_37 
 *           BLE_GAP_ADV_CHANNEL_MAP_38 
 *           BLE_GAP_ADV_CHANNEL_MAP_39 
 *           BLE_GAP_ADV_CHANNEL_MAP_ALL
 *     - filter policies: 
 *           BLE_GAP_ADV_FP_ANY 
 *           BLE_GAP_ADV_FP_FILTER_SCANREQ 
 *           BLE_GAP_ADV_FP_FILTER_CONNREQ 
 *           BLE_GAP_ADV_FP_FILTER_BOTH
 *     
 * Note:  If the advertising_type is set to BLE_GAP_ADV_TYPE_ADV_SCAN_IND or BLE_GAP_ADV_TYPE_ADV_NONCONN_IND, 
 *        the advertising_interval_min and advertising_interval_max should not be set to less than 0x00A0.
 */
static advParams_t adv_params = {
  .adv_int_min   = 0x0030,
  .adv_int_max   = 0x0030,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0,0,0,0,0,0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};

// BLE peripheral advertising data
static uint8_t adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,   
  
  0x11,
  BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
  0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71 
};

// BLE peripheral scan respond data
static uint8_t scan_response[] = {
  0x09,
  BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
  'G', 'o',  'h', 'e', 'e', 'c', 'u', 's'
};

// Characteristic value handle
static uint16_t rgb_char_handle = 0x0000;
// Buffer of characterisitc value.
static uint8_t rgb_char_data[RGB_CHARACTERISTIC_MAX_LEN] = { 0x00, 0x00, 0x00 };

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
 * @brief Connect handle.
 *
 * @param[in]  status   BLE_STATUS_CONNECTION_ERROR or BLE_STATUS_OK.
 * @param[in]  handle   Connect handle.
 *
 * @retval None
 */
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default: break;
  }
}

/**
 * @brief Disconnect handle.
 *
 * @param[in]  handle   Connect handle.
 *
 * @retval None
 */
void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Disconnected.");
}

/**
 * @brief Callback for reading event.
 *
 * @note  If characteristic contains client characteristic configuration,then client characteristic configration handle is value_handle+1.
 *        Now can't add user_descriptor.
 *
 * @param[in]  value_handle    
 * @param[in]  buffer 
 * @param[in]  buffer_size    Ignore it.
 *
 * @retval  Length of current attribute value.
 */
uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {   
  uint8_t characteristic_len = 0;

  Serial.print("Read value handler: ");
  Serial.println(value_handle, HEX);

  if (rgb_char_handle == value_handle) {   // Characteristic value handle.
    Serial.println("RGB Char read:");
    memcpy(buffer, rgb_char_data, RGB_CHARACTERISTIC_MAX_LEN);
    characteristic_len = RGB_CHARACTERISTIC_MAX_LEN;
  }
  return characteristic_len;
}

/**
 * @brief Callback for writting event.
 *
 * @param[in]  value_handle  
 * @param[in]  *buffer       The buffer pointer of writting data.
 * @param[in]  size          The length of writting data.   
 *
 * @retval 
 */
int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  Serial.print("Write value handler: ");
  Serial.println(value_handle, HEX);

  if (rgb_char_handle == value_handle) {
    memcpy(rgb_char_data, buffer, size);
    Serial.print("RGB Char write value: ");
    for (uint8_t index = 0; index < size; index++) {
      Serial.print(rgb_char_data[index], HEX);
      Serial.print(" ");
    }
    RGB.color(rgb_char_data[0], rgb_char_data[1], rgb_char_data[2]);
    Serial.println(" ");
  }
  return 0;
}

/**
 * @brief Setup.
 */
void setup() {
  RGB.control(true);
  
  
  Serial.begin(115200);
  delay(5000);
  Serial.println("BLE Chrome demo.");
    
  // Open debugger, must befor init().
  //ble.debugLogger(true);
  //ble.debugError(true);
  //ble.enablePacketLogger();
    
  // Initialize ble_stack.
  ble.init();

  // Register BLE callback functions.
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onDataReadCallback(gattReadCallback);
  ble.onDataWriteCallback(gattWriteCallback);

  // Add GAP service and characteristics
  ble.addService(BLE_UUID_GAP);
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

  // Add GATT service and characteristics
  ble.addService(BLE_UUID_GATT);
  ble.addCharacteristic(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

  // Add primary service1.
  ble.addService(rgb_service_uuid);
  // Add characteristic to service1, return value handle of characteristic.
  rgb_char_handle = ble.addCharacteristicDynamic(rgb_char_uuid, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, rgb_char_data, RGB_CHARACTERISTIC_MAX_LEN);
  
  // Set BLE advertising parameters
  ble.setAdvertisementParams(&adv_params);

  // Set BLE advertising and scan respond data
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
  ble.setScanResponseData(sizeof(scan_response), scan_response);
  
  // Start advertising.
  ble.startAdvertising();
  Serial.println("BLE start advertising.");
}

/**
 * @brief Loop.
 */
void loop() {
    
}
