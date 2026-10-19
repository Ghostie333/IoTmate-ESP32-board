# IoTmate - firmware listwy zasilającej ESP32

Firmware listwy zasilającej dla XIAO ESP32-C3. Steruje 4 przekaźnikami, konfiguruje Wi-Fi przez portal, rejestruje się w backendzie IoTmate i wymienia stan oraz komendy przez MQTT.

## Wymagania

- XIAO ESP32-C3
- 4-kanałowy moduł przekaźników aktywny w stanie niskim (GPIO1-4)
- Backend IoTmate oraz broker MQTT

## Portal konfiguracyjny

- SSID: `IoTmate-Board`
- Hasło: `iotmate-` + ostatnie 6 znaków adresu MAC (drukowane na porcie szeregowym)
- Przycisk resetu Wi-Fi: GPIO0 do GND

## Budowanie

Dev (TLS bez weryfikacji certyfikatu, HTTP backend 7070):

```sh
pio run
pio run --target upload
pio device monitor
```

Prod (weryfikacja CA, HTTPS backend 443):

```sh
pio run -e prod --target upload
```

## Uwagi sprzętowe

- GPIO2 jest pinem strapping. Stany modułu przekaźników w chwili bootu mogą blokować start ESP32.
