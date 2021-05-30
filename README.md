# esp-idf-world-weather
Display the weather forecast on M5Stack.   
Get the weather forecast from https://www.metaweather.com/api/   
No registration is required.   
You can use this application immediately.   

---

# Hardware requirements
M5Stack

---

# Install

```
git clone https://github.com/nopnop2002/esp-idf-world-weather
cd esp-idf-world-weather
chmod 777 getpem.sh
./getpem.sh
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```

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

![config-1](https://user-images.githubusercontent.com/6020549/73102236-c4c02f00-3f34-11ea-9cea-93832df67bb9.jpg)
![config-2](https://user-images.githubusercontent.com/6020549/73102240-c7228900-3f34-11ea-85d2-aaaae9636303.jpg)
![config-3](https://user-images.githubusercontent.com/6020549/73102246-c853b600-3f34-11ea-841b-a64bf23f21df.jpg)

---

# How to get WOEID
How to use location search:   
https://www.metaweather.com/api/#locationsearch.   

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
You can know the weather forecast of TOKYO.   
https://www.metaweather.com/api/location/1118370/   

You can use test.py for confirmation.

---

# Operation

## View1
Initial screen.   
Press Left button briefly.   
Left:Gothic font.   
Right:Mincyo font.   
![view1](https://user-images.githubusercontent.com/6020549/73107772-0b685600-3f42-11ea-88a9-54d043848aa4.JPG)

## View2
Press Middle button briefly.   
![view2](https://user-images.githubusercontent.com/6020549/73107778-0e634680-3f42-11ea-95d3-be31b6e818b9.JPG)

## View3
Press Right button briefly.   
![view3](https://user-images.githubusercontent.com/6020549/73107781-102d0a00-3f42-11ea-804b-7f678eea60bc.JPG)

## View4
Press and hold Left button.   
![view4](https://user-images.githubusercontent.com/6020549/73107786-11f6cd80-3f42-11ea-8954-151482557060.JPG)

I downloaded the image file from here.   

```
wget https://www.metaweather.com/static/img/weather/png/sn.png
wget https://www.metaweather.com/static/img/weather/png/sl.png
wget https://www.metaweather.com/static/img/weather/png/h.png
wget https://www.metaweather.com/static/img/weather/png/t.png
wget https://www.metaweather.com/static/img/weather/png/hr.png
wget https://www.metaweather.com/static/img/weather/png/lr.png
wget https://www.metaweather.com/static/img/weather/png/s.png
wget https://www.metaweather.com/static/img/weather/png/hc.png
wget https://www.metaweather.com/static/img/weather/png/lc.png
wget https://www.metaweather.com/static/img/weather/png/c.png
```

## View5
Press and hold Middle button.   
![view5](https://user-images.githubusercontent.com/6020549/73107791-13c09100-3f42-11ea-902d-990fdf212dbf.JPG)

## View6
Press and hold Right button.   
![view6](https://user-images.githubusercontent.com/6020549/73107792-158a5480-3f42-11ea-8980-64d71d868d79.JPG)

---

# Font File   
You can add your original fonts.   
The format of the font file is the FONTX format.   
Your font file is put in font directory.   
Your font file is uploaded to SPIFFS partition using meke flash.   

Please refer [this](http://elm-chan.org/docs/dosv/fontx_e.html) page about FONTX format.   

```
FontxFile yourFont[2];
InitFontx(yourFont,"/spiffs/your_font_file_name","");
uint8_t ascii[10];
strcpy((char *)ascii, "MyFont");
uint16_t color = RED;
lcdDrawString(&dev, yourFont, x, y, ascii, color);
```
---

# Font File Editor(FONTX Editor)   
[There](http://elm-chan.org/fsw/fontxedit.zip) is a font file editor.   
This can be done on Windows 10.   
Developer page is [here](http://elm-chan.org/fsw_e.html).   

![FontxEditor](https://user-images.githubusercontent.com/6020549/78731275-3b889800-797a-11ea-81ba-096dbf07c4b8.png)


This library uses the following as default fonts:   
- font/ILGH24XB.FNT // 12x24Dot Gothic
- font/ILMH24XB.FNT // 12x24Dot Mincyo

Changing this file will change the font.


