# Partitions 

The current ESP32-S3 comes with 16 MB of flash memory. 
It's not yet defined if it's possible to include all audio assets within 10 MB. 

So we are likely to use the SD-Card to store all audio assets. 

Other pages related assets will comes with the firmware binary to make the system easier to manage.

## Internal Flash

| Parition | Description | Size |
|---|---|---|
| nvs | Configuration space | 128 KB |
| OTA1 | OTA1 | ~7.5 MB |
| OTA2 | OTA2 | ~7.5 MB |

## Micro-SD Card

The smallest micro-sd is big enough for the system to works.

- Audio assets are stored there. 
- System logs? 


