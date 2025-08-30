# ESP32 SIP Intercom

This project is an **ESP32-based SIP intercom** built with ESP-IDF.  
It uses:
- **Adafruit ESP32 Feather V2** as the main MCU.
- **MAX98357 I2S DAC/Amplifier** for audio output.
- Two GPIO buttons:
  - **Intercom button** → initiate SIP call to predefined extension.
  - **DND button** → toggle Do Not Disturb mode.
- Two LED indicators:
  - **Call LED** → indicates an active call.
  - **DND LED** → indicates Do Not Disturb mode.

## File Structure
```
esp32-sip-intercom/
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/
│   ├── main.c
│   ├── app_intercom.c
│   ├── app_intercom.h
│   ├── sip_wrapper.c
│   ├── sip_wrapper.h
│   ├── audio_i2s.c
│   ├── audio_i2s.h
│   ├── gpio_buttons.c
│   ├── gpio_buttons.h
│   ├── led_status.c
│   ├── led_status.h
│   ├── config.h
│   └── CMakeLists.txt
```

## Build & Flash
Make sure you have ESP-IDF installed and set up:

```bash
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. export.sh
```

Then build this project:

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with your ESP32 serial port.

## Notes
- The current SIP implementation is a **stub** using `sip_wrapper.c`.  
- Replace it with a real SIP/PJSIP integration as needed.  
- Audio is initialized via I²S (`audio_i2s.c`) but does not yet transmit/receive RTP packets.

---
