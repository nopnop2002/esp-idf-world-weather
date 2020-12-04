/* World Weather

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

#include "driver/gpio.h"

#include "esp_http_client.h" 
#include "esp_tls.h" 
#include "cJSON.h"

#include "ili9340.h"
#include "fontx.h"
#include "bmpfile.h"
#include "cmd.h"


// for M5Stack
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240
#define CS_GPIO			14
#define DC_GPIO			27
#define RESET_GPIO		33
#define BL_GPIO			32
#define DISPLAY_LENGTH	26
#define GPIO_INPUT_A	GPIO_NUM_39
#define GPIO_INPUT_B	GPIO_NUM_38
#define GPIO_INPUT_C	GPIO_NUM_37

extern QueueHandle_t xQueueCmd;

static const char *TAG = "M5STACK";

/* Root cert for metaweather.com, taken from metaweather_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.metaweather.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const char metaweather_com_root_cert_pem_start[] asm("_binary_metaweather_com_root_cert_pem_start");
extern const char metaweather_com_root_cert_pem_end[]	asm("_binary_metaweather_com_root_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	static char *output_buffer;  // Buffer to store response of http request from event handler
	static int output_len;		 // Stores number of bytes read
	switch(evt->event_id) {
		case HTTP_EVENT_ERROR:
			ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
			break;
		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
			break;
		case HTTP_EVENT_HEADER_SENT:
			ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
			break;
		case HTTP_EVENT_ON_HEADER:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
			break;
		case HTTP_EVENT_ON_DATA:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
			ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, content_length=%d", esp_http_client_get_content_length(evt->client));
			ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, output_len=%d", output_len);
			// If user_data buffer is configured, copy the response into the buffer
			if (evt->user_data) {
				memcpy(evt->user_data + output_len, evt->data, evt->data_len);
			} else {
				if (output_buffer == NULL && esp_http_client_get_content_length(evt->client) > 0) {
					output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
					output_len = 0;
					if (output_buffer == NULL) {
						ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
						return ESP_FAIL;
					}
				}
				memcpy(output_buffer + output_len, evt->data, evt->data_len);
			}
			output_len += evt->data_len;
			break;
		case HTTP_EVENT_ON_FINISH:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
			if (output_buffer != NULL) {
				// Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
				// ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
				free(output_buffer);
				output_buffer = NULL;
			}
			output_len = 0;
			break;
		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
			int mbedtls_err = 0;
			esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
			if (err != 0) {
				if (output_buffer != NULL) {
					free(output_buffer);
					output_buffer = NULL;
				}
				output_len = 0;
				ESP_LOGE(TAG, "Last esp error code: 0x%x", err);
				ESP_LOGE(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
			}
			break;
	}
	return ESP_OK;
}



// Left Button Monitoring
void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetTaskName(0), "Start");
	CMD_t cmdBuf;
	//cmdBuf.command = CMD_VIEW1;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_pad_select_gpio(GPIO_INPUT_A);
	gpio_set_direction(GPIO_INPUT_A, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_A);
		if (level == 0) {
			ESP_LOGI(pcTaskGetTaskName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_A);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			cmdBuf.command = CMD_VIEW1;
			if (diffTick > 200) cmdBuf.command = CMD_VIEW4;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}

// Middle Button Monitoring
void buttonB(void *pvParameters)
{
	ESP_LOGI(pcTaskGetTaskName(0), "Start");
	CMD_t cmdBuf;
	//cmdBuf.command = CMD_VIEW2;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_pad_select_gpio(GPIO_INPUT_B);
	gpio_set_direction(GPIO_INPUT_B, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_B);
		if (level == 0) {
			ESP_LOGI(pcTaskGetTaskName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_B);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			cmdBuf.command = CMD_VIEW2;
			if (diffTick > 200) cmdBuf.command = CMD_VIEW5;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}

// Right Button Monitoring
void buttonC(void *pvParameters)
{
	ESP_LOGI(pcTaskGetTaskName(0), "Start");
	CMD_t cmdBuf;
	//cmdBuf.command = CMD_VIEW3;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_pad_select_gpio(GPIO_INPUT_C);
	gpio_set_direction(GPIO_INPUT_C, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_C);
		if (level == 0) {
			ESP_LOGI(pcTaskGetTaskName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_C);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			cmdBuf.command = CMD_VIEW3;
			if (diffTick > 200) cmdBuf.command = CMD_VIEW6;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}

char *JSON_Types(int type) {
	if (type == cJSON_Invalid) return ("cJSON_Invalid");
	if (type == cJSON_False) return ("cJSON_False");
	if (type == cJSON_True) return ("cJSON_True");
	if (type == cJSON_NULL) return ("cJSON_NULL");
	if (type == cJSON_Number) return ("cJSON_Number");
	if (type == cJSON_String) return ("cJSON_String");
	if (type == cJSON_Array) return ("cJSON_Array");
	if (type == cJSON_Object) return ("cJSON_Object");
	if (type == cJSON_Raw) return ("cJSON_Raw");
	return NULL;
}

void JSONtoStruct(const cJSON * const root, WEATHER_t * weather) {
	const char* valuestring;
	int valueint;
	double valuedouble;

	valuestring = cJSON_GetObjectItem(root,"title")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->title, valuestring);

	valueint = cJSON_GetObjectItem(root,"woeid")->valueint;
	ESP_LOGD(TAG, "valueint=%d",valueint);
	weather->woeid = valueint;

	valuestring = cJSON_GetObjectItem(root,"sun_set")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->sun_set, valuestring);

	valuestring = cJSON_GetObjectItem(root,"latt_long")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->latt_long, valuestring);

	valuestring = cJSON_GetObjectItem(root,"time")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->time, valuestring);

	valuestring = cJSON_GetObjectItem(root,"timezone_name")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->timezone_name, valuestring);

	valuestring = cJSON_GetObjectItem(root,"timezone")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->timezone, valuestring);

	valuestring = cJSON_GetObjectItem(root,"sun_rise")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->sun_rise, valuestring);

	valuestring = cJSON_GetObjectItem(root,"location_type")->valuestring;
	ESP_LOGD(TAG, "valuestring=%s",valuestring);
	strcpy(weather->location_type, valuestring);

	cJSON *array = cJSON_GetObjectItem(root,"consolidated_weather");
	ESP_LOGD(TAG, "type=%s", JSON_Types(array->type));
	int array_size = cJSON_GetArraySize(array); 
	ESP_LOGD(TAG, "array_size=%d", array_size);
	for (int i=0;i<array_size;i++) {
		cJSON *array_element = cJSON_GetArrayItem(array,i);
		ESP_LOGD(TAG, "type=%s", JSON_Types(array_element->type));

		valuedouble = cJSON_GetObjectItem(array_element,"wind_speed")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].wind_speed = valuedouble;

		valuestring = cJSON_GetObjectItem(array_element,"applicable_date")->valuestring;
		ESP_LOGD(TAG, "valuestring=%s",valuestring);
		strcpy(weather->daily[i].applicable_date, valuestring);

		valueint = cJSON_GetObjectItem(array_element,"predictability")->valueint;
		ESP_LOGD(TAG, "valueint=%d",valueint);
		weather->daily[i].predictability = valueint;

		valuestring = cJSON_GetObjectItem(array_element,"weather_state_abbr")->valuestring;
		ESP_LOGD(TAG, "valuestring=%s",valuestring);
		strcpy(weather->daily[i].weather_state_abbr, valuestring);

		valuestring = cJSON_GetObjectItem(array_element,"weather_state_name")->valuestring;
		ESP_LOGD(TAG, "valuestring=%s",valuestring);
		strcpy(weather->daily[i].weather_state_name, valuestring);

		valuestring = cJSON_GetObjectItem(array_element,"created")->valuestring;
		ESP_LOGD(TAG, "valuestring=%s",valuestring);
		strcpy(weather->daily[i].created, valuestring);

		valuedouble = cJSON_GetObjectItem(array_element,"wind_direction")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].wind_direction = valuedouble;

		valuedouble = cJSON_GetObjectItem(array_element,"air_pressure")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].air_pressure = valuedouble;

		valueint = cJSON_GetObjectItem(array_element,"humidity")->valueint;
		ESP_LOGD(TAG, "valueint=%d",valueint);
		weather->daily[i].humidity = valueint;

		valuedouble = cJSON_GetObjectItem(array_element,"visibility")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].visibility = valuedouble;

		valuedouble = cJSON_GetObjectItem(array_element,"the_temp")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].the_temp = valuedouble;

		valuedouble = cJSON_GetObjectItem(array_element,"min_temp")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].min_temp = valuedouble;

		valuedouble = cJSON_GetObjectItem(array_element,"max_temp")->valuedouble;
		ESP_LOGD(TAG, "valuedouble=%f",valuedouble);
		weather->daily[i].max_temp = valuedouble;

		valueint = cJSON_GetObjectItem(array_element,"id")->valueint;
		ESP_LOGD(TAG, "valueint=%d",valueint);
		weather->daily[i].id = valueint;

		valuestring = cJSON_GetObjectItem(array_element,"wind_direction_compass")->valuestring;
		ESP_LOGD(TAG, "valuestring=%s",valuestring);
		strcpy(weather->daily[i].wind_direction_compass, valuestring);
	}
}

void StructSort(WEATHER_t * weather) {
	//DAILY_t work;
	for(int i=0;i<6;i++) {
		ESP_LOGI(TAG, "applicable_date=%s", weather->daily[i].applicable_date);
	}
}

void JSON_Dump(const cJSON * const root) {
	//ESP_LOGI(TAG, "root->type=%s", JSON_Types(root->type));
	cJSON *current_element = NULL;
	//ESP_LOGI(TAG, "roo->child=%p", root->child);
	//ESP_LOGI(TAG, "roo->next =%p", root->next);
	cJSON_ArrayForEach(current_element, root) {
		//ESP_LOGI(TAG, "type=%s", JSON_Types(current_element->type));
		//ESP_LOGI(TAG, "current_element->string=%p", current_element->string);
		if (current_element->string) {
			const char* string = current_element->string;
			ESP_LOGI(TAG, "[%s]", string);
		}
		if (cJSON_IsInvalid(current_element)) {
			ESP_LOGI(TAG, "Invalid");
		} else if (cJSON_IsFalse(current_element)) {
			ESP_LOGI(TAG, "False");
		} else if (cJSON_IsTrue(current_element)) {
			ESP_LOGI(TAG, "True");
		} else if (cJSON_IsNull(current_element)) {
			ESP_LOGI(TAG, "Null");
		} else if (cJSON_IsNumber(current_element)) {
			int valueint = current_element->valueint;
			double valuedouble = current_element->valuedouble;
			ESP_LOGI(TAG, "int=%d double=%f", valueint, valuedouble);
		} else if (cJSON_IsString(current_element)) {
			const char* valuestring = current_element->valuestring;
			ESP_LOGI(TAG, "%s", valuestring);
		} else if (cJSON_IsArray(current_element)) {
			//ESP_LOGI(TAG, "Array");
			JSON_Dump(current_element);
		} else if (cJSON_IsObject(current_element)) {
			//ESP_LOGI(TAG, "Object");
			JSON_Dump(current_element);
		} else if (cJSON_IsRaw(current_element)) {
			ESP_LOGI(TAG, "Raw(Not support)");
		}
	}
}

// for test
CJSON_PUBLIC(cJSON *) http_client_get_test(char * url)
{
	cJSON *root = NULL;
	ESP_LOGI(TAG, "Reading file");
	FILE* f = fopen("/fonts/test.json", "r");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for reading");
		return root;
	}
	char *buffer = malloc(4096 + 1); 
	fgets(buffer, 4096, f);
	fclose(f);
	ESP_LOGI(TAG, "buffer=\n%s", buffer);
	root = cJSON_Parse(buffer);
	free(buffer);
	return root;
}

size_t http_client_content_length(char * url)
{
	ESP_LOGI(TAG, "http_client_content_length url=%s",url);
	size_t content_length;
	
	esp_http_client_config_t config = {
		.url = url,
		.event_handler = _http_event_handler,
		//.user_data = local_response_buffer,		   // Pass address of local buffer to get response
		.cert_pem = metaweather_com_root_cert_pem_start,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	// GET
	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK) {
		ESP_LOGD(TAG, "HTTP GET Status = %d, content_length = %d",
				esp_http_client_get_status_code(client),
				esp_http_client_get_content_length(client));
		content_length = esp_http_client_get_content_length(client);

	} else {
		ESP_LOGW(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
		content_length = 0;
	}
	esp_http_client_cleanup(client);
	return content_length;
}



esp_err_t http_client_content_get(char * url, char * response_buffer)
{
	ESP_LOGI(TAG, "http_client_content_get url=%s",url);

	esp_http_client_config_t config = {
		.url = url,
		.event_handler = _http_event_handler,
		.user_data = response_buffer,		   // Pass address of local buffer to get response
		.cert_pem = metaweather_com_root_cert_pem_start,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	// GET
	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
				esp_http_client_get_status_code(client),
				esp_http_client_get_content_length(client));
		ESP_LOGD(TAG, "\n%s", response_buffer);
	} else {
		ESP_LOGW(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
	}
	esp_http_client_cleanup(client);
	return err;
}

CJSON_PUBLIC(cJSON *) http_client_get(char * url)
{
	// Get content length from event handler
	size_t content_length;
	while (1) {
		content_length = http_client_content_length(url);
		ESP_LOGI(TAG, "content_length=%d", content_length);
		if (content_length > 0) break;
		vTaskDelay(100);
	}

	// Allocate buffer to store response of http request from event handler
	char *response_buffer;
	response_buffer = (char *) malloc(content_length+1);
	if (response_buffer == NULL) {
		ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
		while(1) {
			vTaskDelay(1);
		}
	}
	bzero(response_buffer, content_length+1);

	// Get content from event handler
	while(1) {
		esp_err_t err = http_client_content_get(url, response_buffer);
		if (err == ESP_OK) break;
		vTaskDelay(100);
	}
	ESP_LOGI(TAG, "content_length=%d", content_length);
	ESP_LOGI(TAG, "\n[%s]", response_buffer);

	// Deserialize JSON
	cJSON *root = NULL;
	root = cJSON_Parse(response_buffer);
	free(response_buffer);
	return root;
}


void show_datetime(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	uint16_t ypos = (fontHeight*2)-1;
	char work[64];
	char *tp;
	char *tp2;
	uint8_t ascii[44];

	strcpy(work, weather.time);
	ESP_LOGD(TAG, "weather.time=%s", weather.time);
	tp = strtok(work, "T");
	ESP_LOGD(TAG, "tp=%s", tp);
	tp2 = strtok(NULL, ".");
	ESP_LOGD(TAG, "tp2=%s", tp2);
	sprintf((char *)ascii, "%s %s", tp, tp2);
	ESP_LOGD(TAG, "ascii=%s", (char *)ascii);
	// center align
	uint16_t title_len = strlen((char *)ascii) * fontWidth;
	uint16_t xpos_title = 0;
	if (SCREEN_WIDTH > title_len) xpos_title = (SCREEN_WIDTH - title_len) / 2;
	lcdDrawString(dev, fx, xpos_title, ypos, ascii, YELLOW);
}


void view1(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	uint8_t ascii[44];
	char work[64];
	char *tp;

	lcdDrawFillRect(dev, 0, (fontHeight*1), SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	show_datetime(dev, weather, fx, fontWidth, fontHeight);

	uint16_t xpos = (fontWidth*4)-1;
	uint16_t ypos = (fontHeight*4)-1;

#if 0
	strcpy(work, weather.latt_long);
	tp = strtok(work, ",");
	ESP_LOGD(TAG, "view1 tp=%s", tp);
	sprintf((char *)ascii, "latt	:%s", tp);
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

	tp = strtok(NULL, ",");
	ESP_LOGD(TAG, "view1 tp=%s", tp);
	sprintf((char *)ascii, "long	:%s", tp);
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

	sprintf((char *)ascii, "timezone:%s", weather.timezone);
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;
#endif

	sprintf((char *)ascii, "weather  :%s", weather.daily[0].weather_state_name);
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

	strcpy(work, weather.sun_rise);
	tp = strtok(work, "T");
	tp = strtok(NULL, ".");
	ESP_LOGD(TAG, "view1 tp=%s", tp);
	sprintf((char *)ascii, "sunrise  :%s", tp);
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

	strcpy(work, weather.sun_set);
	tp = strtok(work, "T");
	tp = strtok(NULL, ".");
	ESP_LOGD(TAG, "view1 tp=%s", tp);
	sprintf((char *)ascii, "sunset   :%s", tp);
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

	if (weather.daily[0].the_temp > 0.0) {
		sprintf((char *)ascii, "temp     :%4.1f", weather.daily[0].the_temp); 
	} else {
		sprintf((char *)ascii, "temp     :%5.1f", weather.daily[0].the_temp); 
	}
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

#if 1
	if (weather.daily[0].min_temp > 0.0) {
		sprintf((char *)ascii, "temp(min):%4.1f", weather.daily[0].min_temp);
	} else {
		sprintf((char *)ascii, "temp(min):%5.1f", weather.daily[0].min_temp);
	}
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	ypos = ypos + fontHeight;

	if (weather.daily[0].max_temp > 0.0) {
		sprintf((char *)ascii, "temp(max):%4.1f", weather.daily[0].max_temp);
	} else {
		sprintf((char *)ascii, "temp(max):%5.1f", weather.daily[0].max_temp);
	}
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
#endif
}

void view2(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	uint8_t ascii[44];
	char work[64];

	lcdDrawFillRect(dev, 0, (fontHeight*1), SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	show_datetime(dev, weather, fx, fontWidth, fontHeight);

	uint16_t xpos = (fontWidth*4)-1;
	uint16_t ypos = (fontHeight*4)-1;
	strcpy((char *)ascii, "Weather State");
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);

	for(int i=0;i<6;i++) {
		ESP_LOGD(TAG, "applicable_date=%s", weather.daily[i].applicable_date);
		strcpy(work, &(weather.daily[i].applicable_date[5]));
		ESP_LOGD(TAG, "work=%s", work);
		sprintf((char *)ascii, "%.5s:%.12s", work, \
			weather.daily[i].weather_state_name);
		ypos = ypos + fontHeight;
		lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	}
}

void view3(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	uint8_t ascii[44];
	char work[64];

	lcdDrawFillRect(dev, 0, (fontHeight*1), SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	show_datetime(dev, weather, fx, fontWidth, fontHeight);

	uint16_t xpos = (fontWidth*1)-1;
	uint16_t ypos = (fontHeight*4)-1;
	strcpy((char *)ascii, "Temperature(Avr,Max,Min)");
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);

	for(int i=0;i<6;i++) {
		ESP_LOGD(TAG, "applicable_date=%s", weather.daily[i].applicable_date);
		strcpy(work, &(weather.daily[i].applicable_date[5]));
		ESP_LOGD(TAG, "work=%s", work);
		sprintf((char *)ascii, "%.5s:%5.1f %5.1f %5.1f", work, \
			weather.daily[i].the_temp, \
			weather.daily[i].max_temp, \
			weather.daily[i].min_temp);
		ypos = ypos + fontHeight;
		lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	}
}

void bmp_display(TFT_t *dev, char * file, int ypos, int width, int height) {
	// open requested file
	FILE* fp;
	esp_err_t ret;
	fp = fopen(file, "rb");
	if (fp == NULL) {
		ESP_LOGW(TAG, "File not found [%s]", file);
		return;
	}

	// read bmp header
	bmpfile_t *result = (bmpfile_t*)malloc(sizeof(bmpfile_t));
	ret = fread(result->header.magic, 1, 2, fp);
	assert(ret == 2);
	ESP_LOGD(TAG,"result->header.magic=%c %c", result->header.magic[0], result->header.magic[1]);
	if (result->header.magic[0]!='B' || result->header.magic[1] != 'M') {
		ESP_LOGW(TAG, "File is not BMP");
		free(result);
		fclose(fp);
		return;
	}
	ret = fread(&result->header.filesz, 4, 1 , fp);
	assert(ret == 1);
	ESP_LOGD(TAG,"result->header.filesz=%d", result->header.filesz);
	ret = fread(&result->header.creator1, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->header.creator2, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->header.offset, 4, 1, fp);
	assert(ret == 1);

	// read dib header
	ret = fread(&result->dib.header_sz, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.width, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.height, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.nplanes, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.depth, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.compress_type, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.bmp_bytesz, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.hres, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.vres, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.ncolors, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.nimpcolors, 4, 1, fp);
	assert(ret == 1);

	if((result->dib.depth == 24) && (result->dib.compress_type == 0)) {
		// BMP rows are padded (if needed) to 4-byte boundary
		uint32_t rowSize = (result->dib.width * 3 + 3) & ~3;
		int w = result->dib.width;
		int h = result->dib.height;
		ESP_LOGI(TAG,"w=%d h=%d", w, h);
		int _x;
		int _w;
		int _cols;
		int _cole;
		if (width >= w) {
			_x = (width - w) / 2;
			_w = w;
			_cols = 0;
			_cole = w - 1;
		} else {
			_x = 0;
			_w = width;
			_cols = (w - width) / 2;
			_cole = _cols + width - 1;
		}
		ESP_LOGI(TAG,"_x=%d _w=%d _cols=%d _cole=%d",_x, _w, _cols, _cole);

		int _y = ypos;
		int _rows = 0;
		int _rowe = h - 1;
		ESP_LOGI(TAG,"_y=%d _rows=%d _rowe=%d", _y, _rows, _rowe);

#define BUFFPIXEL 20
		uint8_t sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
		uint16_t *colors = (uint16_t*)malloc(sizeof(uint16_t) * w);

		for (int row=0; row<h; row++) { // For each scanline...
			if (row < _rows || row > _rowe) continue;
			// Seek to start of scan line.	It might seem labor-
			// intensive to be doing this on every line, but this
			// method covers a lot of gritty details like cropping
			// and scanline padding.  Also, the seek only takes
			// place if the file position actually needs to change
			// (avoids a lot of cluster math in SD library).
			// Bitmap is stored bottom-to-top order (normal BMP)
#if 1
			int pos = result->header.offset + (h - 1 - row) * rowSize;
			ESP_LOGD(TAG,"fseek start row=%d pos=%d",row, pos);
			fseek(fp, pos, SEEK_SET);
			ESP_LOGD(TAG,"fseek end   row=%d pos=%d",row, pos);
#endif

			int buffidx = sizeof(sdbuffer); // Force buffer reload

			int index = 0;
			for (int col=0; col<w; col++) { // For each pixel...
				if (buffidx >= sizeof(sdbuffer)) { // Indeed
					ESP_LOGD(TAG,"fread start row=%d",row);
					fread(sdbuffer, sizeof(sdbuffer), 1, fp);
					ESP_LOGD(TAG,"fread end   row=%d",row);
					buffidx = 0; // Set index to beginning
				}
				if (col < _cols || col > _cole) continue;
				// Convert pixel from BMP to TFT format, push to display
				uint8_t b = sdbuffer[buffidx++];
				uint8_t g = sdbuffer[buffidx++];
				uint8_t r = sdbuffer[buffidx++];
				colors[index++] = rgb565_conv(r, g, b);
			} // end for col
			ESP_LOGD(TAG,"lcdDrawMultiPixels start row=%d",row);
			lcdDrawMultiPixels(dev, _x, row+_y, _w, colors);
			ESP_LOGD(TAG,"lcdDrawMultiPixels end   row=%d",row);
		} // end for row
		free(colors);
	} // end if 
	ESP_LOGI(TAG,"bmp_display end");
	free(result);
	fclose(fp);
	return;
}

void view4(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	lcdDrawFillRect(dev, 0, (fontHeight*1), SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	show_datetime(dev, weather, fx, fontWidth, fontHeight);

	ESP_LOGI(TAG, "weather.daily[0].weather_state_abbr=%s",weather.daily[0].weather_state_abbr);
	char file[32];
	char dir[10];
	strcpy(dir, "/images1");
	if (strcmp(weather.daily[0].weather_state_abbr, "lr") == 0) {
		strcpy(dir, "/images2");
	} else if (strcmp(weather.daily[0].weather_state_abbr, "s") == 0) {
		strcpy(dir, "/images2");
	} else if (strcmp(weather.daily[0].weather_state_abbr, "sl") == 0) {
		strcpy(dir, "/images2");
	} else if (strcmp(weather.daily[0].weather_state_abbr, "sn") == 0) {
		strcpy(dir, "/images2");
	} else if (strcmp(weather.daily[0].weather_state_abbr, "t") == 0) {
		strcpy(dir, "/images2");
	}
	sprintf(file, "%s/%s.bmp", dir, weather.daily[0].weather_state_abbr);
	ESP_LOGI(TAG, "file=%s", file);
	bmp_display(dev, file, fontHeight*2, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void view5(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	uint8_t ascii[44];
	char work[64];

	lcdDrawFillRect(dev, 0, (fontHeight*1), SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	show_datetime(dev, weather, fx, fontWidth, fontHeight);

	uint16_t xpos = (fontWidth*2)-1;
	uint16_t ypos = (fontHeight*4)-1;
	strcpy((char *)ascii, "Wind Speed/Direction");
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);

	for(int i=0;i<6;i++) {
		ESP_LOGD(TAG, "applicable_date=%s", weather.daily[i].applicable_date);
		strcpy(work, &(weather.daily[i].applicable_date[5]));
		ESP_LOGD(TAG, "work=%s", work);
		sprintf((char *)ascii, "%.5s:%7.4f %5.1f(%.5s)", work, \
			weather.daily[i].wind_speed,\
			weather.daily[i].wind_direction,\
			weather.daily[i].wind_direction_compass);
		ypos = ypos + fontHeight;
		lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	}
}

void view6(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight)
{
	uint8_t ascii[44];
	char work[64];

	lcdDrawFillRect(dev, 0, (fontHeight*1), SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
	show_datetime(dev, weather, fx, fontWidth, fontHeight);

	uint16_t xpos = (fontWidth*4)-1;
	uint16_t ypos = (fontHeight*4)-1;
	strcpy((char *)ascii, "Pressure/Humidity");
	lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);

	for(int i=0;i<6;i++) {
		ESP_LOGD(TAG, "applicable_date=%s", weather.daily[i].applicable_date);
		strcpy(work, &(weather.daily[i].applicable_date[5]));
		ESP_LOGD(TAG, "work=%s", work);
		sprintf((char *)ascii, "%.5s:%6.1f %3d", work, \
			weather.daily[i].air_pressure, \
			weather.daily[i].humidity);
		ypos = ypos + fontHeight;
		lcdDrawString(dev, fx, xpos, ypos, ascii, CYAN);
	}
}

void tft(void *pvParameters)
{
	// Set initial view
	int32_t screen_type = (int32_t)pvParameters;
	ESP_LOGI(pcTaskGetTaskName(0), "Start screen_type=%d", screen_type);

	// Set font file
	FontxFile fx[2];
#if CONFIG_ESP_FONT_GOTHIC
	InitFontx(fx,"/fonts/ILGH24XB.FNT",""); // 12x24Dot Gothic
#endif
#if CONFIG_ESP_FONT_MINCYO
	InitFontx(fx,"/fonts/ILMH24XB.FNT",""); // 12x24Dot Mincyo
#endif

	// Get font width & height
	uint8_t buffer[FontxGlyphBufSize];
	uint8_t fontWidth;
	uint8_t fontHeight;
	GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
	ESP_LOGI(pcTaskGetTaskName(0), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

	// Setup Screen
	TFT_t dev;
	spi_master_init(&dev, CS_GPIO, DC_GPIO, RESET_GPIO, BL_GPIO);
	lcdInit(&dev, 0x9341, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	ESP_LOGI(pcTaskGetTaskName(0), "Setup Screen done");

	int lines = (SCREEN_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGD(pcTaskGetTaskName(0), "SCREEN_HEIGHT=%d fontHeight=%d lines=%d", SCREEN_HEIGHT, fontHeight, lines);
	int ymax = (lines+1) * fontHeight;
	ESP_LOGD(pcTaskGetTaskName(0), "ymax=%d",ymax);

	// Clear Screen
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);

	// Reset scroll area
	lcdSetScrollArea(&dev, 0, 0x0140, 0);

	// Get Weather Information
	ESP_LOGI(pcTaskGetTaskName(0), "woeid=%d",CONFIG_ESP_WOEID);
	char url[64];
	//https://www.metaweather.com/api/location/1118370/
	sprintf(url, "http://www.metaweather.com/api/location/%d/", CONFIG_ESP_WOEID);
	ESP_LOGI(pcTaskGetTaskName(0), "url=%s",url);
	WEATHER_t weather;

	// for test
	//cJSON *root = http_client_get_test(url);

	cJSON *root = http_client_get(url);
	//JSON_Dump(root);
	JSONtoStruct(root, &weather);
	StructSort(&weather);
	cJSON_Delete(root);

	// Show header
	uint8_t ascii[44];
	uint16_t ypos = fontHeight-1;
	if (strlen(weather.title) < 13) {
		sprintf((char *)ascii, "World Weather %.12s", weather.title);
	} else {
		sprintf((char *)ascii, "%.26s", weather.title);
	}
	uint16_t title_len = strlen((char *)ascii) * fontWidth;
	uint16_t xpos_title = 0;
	if (SCREEN_WIDTH > title_len) xpos_title = (SCREEN_WIDTH - title_len) / 2;
	lcdDrawString(&dev, fx, xpos_title, ypos, ascii, YELLOW);

	// Show screen
	//view1(&dev, weather, fx, fontWidth, fontHeight);
	void (*func)(TFT_t *dev, WEATHER_t weather, FontxFile *fx, uint8_t fontWidth, uint8_t fontHeight);
	func = view1;
	if (screen_type == 2) {
		func = view2;
	} else if (screen_type == 3) {
		func = view3;
	} else if (screen_type == 4) {
		func = view4;
	} else if (screen_type == 5) {
		func = view5;
	} else if (screen_type == 6) {
		func = view6;
	}
	(*func)(&dev, weather, fx, fontWidth, fontHeight);

	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGI(pcTaskGetTaskName(0),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_VIEW1) {
			view1(&dev, weather, fx, fontWidth, fontHeight);
			func = view1;
		} else if (cmdBuf.command == CMD_VIEW2) {
			view2(&dev, weather, fx, fontWidth, fontHeight);
			func = view2;
		} else if (cmdBuf.command == CMD_VIEW3) {
			view3(&dev, weather, fx, fontWidth, fontHeight);
			func = view3;
		} else if (cmdBuf.command == CMD_VIEW4) {
			view4(&dev, weather, fx, fontWidth, fontHeight);
			func = view4;
		} else if (cmdBuf.command == CMD_VIEW5) {
			view5(&dev, weather, fx, fontWidth, fontHeight);
			func = view5;
		} else if (cmdBuf.command == CMD_VIEW6) {
			view6(&dev, weather, fx, fontWidth, fontHeight);
			func = view6;
		} else if (cmdBuf.command == CMD_UPDATE) {
#if 0
			// Start wifi
			wifi_start_sta(EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_MAXIMUM_RETRY);
#endif
			cJSON *root = http_client_get(url);
			//JSON_Dump(root);
			JSONtoStruct(root, &weather);
			StructSort(&weather);
			cJSON_Delete(root);
			(*func)(&dev, weather, fx, fontWidth, fontHeight);
		}
	}

	// nerver reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

