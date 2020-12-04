#define CMD_VIEW1       100
#define CMD_VIEW2       200
#define CMD_VIEW3       300
#define CMD_VIEW4       400
#define CMD_VIEW5       500
#define CMD_VIEW6       600
#define CMD_UPDATE      700

typedef struct {
    uint16_t command;
    TaskHandle_t taskHandle;
} CMD_t;


typedef struct {
    double  wind_speed;                 // "wind_speed": 6.245802910999761
    char    applicable_date[16];        // "applicable_date": "2020-01-21"
    int     predictability;             // "predictability": 68
    char    weather_state_abbr[16];     // "weather_state_abbr": "lr"
    char    weather_state_name[32];     // "weather_state_name": "Light Rain"
    char    created[32];                // "created": "2020-01-16T03:21:02.526991Z"
    double  wind_direction;             // "wind_direction": 357.5
    double  air_pressure;               // "air_pressure": 1020.0
    int     humidity;                   // "humidity": 42
    double  visibility;                 // "visibility": 14.596009305654974
    double  the_temp;                   // "the_temp": 9.615
    double  min_temp;                   // "min_temp": 5.404999999999999
    double  max_temp;                   // "max_temp": 10.425
    int     id;                         // "id": 4970689951956992
    char    wind_direction_compass[6];  // "wind_direction_compass": "NNW"
} DAILY_t;


typedef struct {
    char    title[32];                  // "title": "Tokyo"
    int     woeid;                      // "woeid": 1118370
    char    sun_set[64];                // "sun_set": "2020-01-16T16:51:05.516037+09:00"
    char    latt_long[64];              // "latt_long": "35.670479,139.740921"
    char    time[64];                   // "time": "2020-01-16T13:46:06.039385+09:00"
    char    timezone_name[32];          // "timezone_name": "JST"
    char    timezone[32];               // "timezone": "Asia/Tokyo"
    char    sun_rise[64];               // "sun_rise": "2020-01-16T06:49:50.916354+09:00"
    char    location_type[32];          // "location_type": "City"
    DAILY_t daily[6];                   // See above
} WEATHER_t;

