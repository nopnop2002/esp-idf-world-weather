# esp-idf-world-weather
Display the weather forecast on M5Stack.   
Get the weather forecast from https://www.metaweather.com/api/   
No registration is required.   
You can use this application immediately.   

---

# Hardware requirements
M5Stack

---

# Firmware configuration
You have to set this config value using menuconfig.   

- CONFIG_ESP_WIFI_SSID   
SSID of your wifi.

- CONFIG_ESP_WIFI_PASSWORD   
PASSWORD of your wifi.

- CONFIG_ESP_MAXIMUM_RETRY   
Maximum number of retries when connecting to wifi.

- CONFIG_ESP_WOEID   
WOEID(Where On Earth IDentifier) which you want to display the weather forecast.

- CONFIG_ESP_UPDATE_PERIOD   
Display update cycle (minutes)

- CONFIG_ESP_FONT   
The font to use.

```
git clone https://github.com/nopnop2002/esp-idf-ili9340
cd esp-idf-ili9340/
make menuconfig
make flash
```

![config-1](https://user-images.githubusercontent.com/6020549/73102236-c4c02f00-3f34-11ea-9cea-93832df67bb9.jpg)
![config-2](https://user-images.githubusercontent.com/6020549/73102240-c7228900-3f34-11ea-85d2-aaaae9636303.jpg)
![config-3](https://user-images.githubusercontent.com/6020549/73102246-c853b600-3f34-11ea-841b-a64bf23f21df.jpg)

---

# How to get WOEID
Request WOEID at https://www.metaweather.com/api/#locationsearch.   
Cities containing 'san':   
https://www.metaweather.com/api/location/search/?query=san

```
[
    {
        "woeid": 2487956,
        "latt_long": "37.777119, -122.41964",
        "location_type": "City",
        "title": "San Francisco"
    },
    {
        "woeid": 2487889,
        "latt_long": "32.715691,-117.161720",
        "location_type": "City",
        "title": "San Diego"
    },
    {
        "woeid": 2488042,
        "latt_long": "37.338581,-121.885567",
        "location_type": "City",
        "title": "San Jose"
    },
    {
        "woeid": 2487796,
        "latt_long": "29.424580,-98.494614",
        "location_type": "City",
        "title": "San Antonio"
    },
    {
        "woeid": 2488853,
        "latt_long": "36.974018,-122.030952",
        "location_type": "City",
        "title": "Santa Cruz"
    },
    {
        "woeid": 349859,
        "latt_long": "-33.463039,-70.647942",
        "location_type": "City",
        "title": "Santiago"
    },
    {
        "woeid": 56558361,
        "latt_long": "36.406651,25.456530",
        "location_type": "City",
        "title": "Santorini"
    },
    {
        "woeid": 773964,
        "latt_long": "43.461498,-3.810010",
        "location_type": "City",
        "title": "Santander"
    },
    {
        "woeid": 1132447,
        "latt_long": "35.170429,128.999481",
        "location_type": "City",
        "title": "Busan"
    },
    {
        "woeid": 773692,
        "latt_long": "28.46163,-16.267059",
        "location_type": "City",
        "title": "Santa Cruz de Tenerife"
    },
    {
        "woeid": 2488867,
        "latt_long": "35.666431,-105.972572",
        "location_type": "City",
        "title": "Santa Fe"
    }
]
```

Cities containing 'kyo':  
https://www.metaweather.com/api/location/search/?query=kyo

```
[
    {
        "woeid": 1118370,
        "latt_long": "35.670479,139.740921",
        "location_type": "City",
        "title": "Tokyo"
    },
    {
        "woeid": 15015372,
        "latt_long": "35.098129,135.718933",
        "location_type": "City",
        "title": "Kyoto"
    }
]
```

WOEID of TOKYO is 1118370.   
You can use test.py for confirmation.

# Operation

## View1
Initial screen.   
Press Left button briefly.   

## View2
Press Middle button briefly.   

## View3
Press Right button briefly.   

## View4
Press and hold Left button.   

## View5
Press and hold Middle button.   

## View6
Press and hold Right button.   

