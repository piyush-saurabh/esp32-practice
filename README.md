# esp32-practice
This repository contains code for ESP32 developed using IDF

Below are the different code examples used in this repo

## ble-gatt-server
Here ESP32 acts as GATT server whereas the mobile device acts as client

Use mobile apps like [BLE Scanner](https://play.google.com/store/apps/details?id=com.macdom.ble.blescanner&hl=en_IN&gl=US) or [NRF Connect for Mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&hl=en_IN&gl=US) to view the BLE advertisement and receive notifications from GATT server (ESP32).

**Note**: Parining of ESP32 with mobile device or Linux system is failing. Use the above mobile apps view the advertisement

## ble-gatt-client
Here mobile device acts as GATT server and ESP32 acts as client.