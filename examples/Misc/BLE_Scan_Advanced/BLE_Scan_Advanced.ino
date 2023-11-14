#include <Adafruit_TinyUSB.h> // for Serial
#include <bluefruit.h>

#define BEACON_DATA_LEN     0x15
#define BEACON_DATA_TYPE    0x02
#define COMPANY_IDENTIFIER  0x004C

void setup() 
{
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Bluefruit52 Central ADV Scan ibeacon");
  Serial.println("------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  /* Set the device name */
  Bluefruit.setName("Bluefruit52");

  /* Set the LED interval for blinky pattern on BLUE LED */
  // Bluefruit.setConnLedInterval(250);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter out packet with a min rssi
   * - Interval = 100 ms, window = 50 ms
   * - Use active scan (used to retrieve the optional scan response adv packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-80);
  //Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE); // only invoke callback if detect bleuart service
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  Serial.println("Scanning ...");
}

void loop() 
{
  delay(10000);
  Bluefruit.Scanner.stop();
  delay(10000);
  Bluefruit.Scanner.start(0);
  // nothing to do
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  PRINT_LOCATION();
  uint8_t len = 0;
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  /* Check for Manufacturer Specific Data */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));
  if (len)
  {
    uint8_t beacon_data_len = 0;
    uint8_t beacon_data_type = 0;
    uint16_t company_identifier = 0;
    beacon_data_len = buffer[3];
    beacon_data_type = buffer[2];
    memcpy((uint8_t*)(&company_identifier), buffer, 2);
    if (beacon_data_type == BEACON_DATA_TYPE)
    {
      if (company_identifier == COMPANY_IDENTIFIER)
      {
        if (beacon_data_len == BEACON_DATA_LEN)
        {
          Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
          Serial.print("\n");              
          Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);
          Serial.printf("iBeacon: ");
          for (uint8_t i = 0; i < len; i++) Serial.printf("%02x ", buffer[i]);
          Serial.printf("\r\n");
        }
      }
    }

    memset(buffer, 0, sizeof(buffer));
  }

  Serial.println();

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}
