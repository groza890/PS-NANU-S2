#pragma once
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>

// Dispozitivul tău
static const char* connectionString = "HostName=PS2.azure-devices.net;DeviceId=myESP32;SharedAccessKey=haOoaXnTPgQBsnANUUZnHQpPn9YPx8VZFHnvYhqrkSg=";

namespace AzureIoT {
  static IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient;

  inline void begin() {
    IoTHub_Init();
    deviceClient = IoTHubDeviceClient_LL_CreateFromConnectionString(
      connectionString, MQTT_Protocol);
    if (!deviceClient) {
      Serial.println("❌ IoT Hub client init failed");
      return;
    }
    // setează callback pentru C2D (comenzi cloud->device)
    IoTHubDeviceClient_LL_SetMessageCallback(deviceClient,
      [](IOTHUB_MESSAGE_HANDLE msg, void*) {
        const unsigned char* buff; size_t sz;
        IoTHubMessage_GetByteArray(msg, &buff, &sz);
        String s((char*)buff, sz);
        Serial.println("⬅️ Cmd from cloud: " + s);
        // ex: dacă s conține “LED:ON”, aprinde LED‐ul
        if (s.startsWith("LED:")) {
          bool on = s.substring(4) == "ON";
          digitalWrite(PIN_LED, on?HIGH:LOW);
        }
        IoTHubDeviceClient_LL_SendEventAsync(
          deviceClient,
          IoTHubMessage_CreateFromString("{\"ack\":\"OK\"}"),
          nullptr, nullptr);
        return IOTHUBMESSAGE_ACCEPTED;
      }, nullptr);

    Serial.println("✅ IoT Hub client initialized");
  }

  inline void sendTelemetry(const String &payload) {
    auto msg = IoTHubMessage_CreateFromString(payload.c_str());
    IoTHubDeviceClient_LL_SendEventAsync(deviceClient, msg, nullptr, nullptr);
    IoTHubMessage_Destroy(msg);
    IoTHubDeviceClient_LL_DoWork(deviceClient);
  }

  inline void loop() {
    IoTHubDeviceClient_LL_DoWork(deviceClient);
  }
}
