#ifndef __wifi_sta_h__
#define __wifi_sta_h__

esp_err_t wifi_start_sta(char * ssid, char * pass, int maximun_retry);
esp_err_t wifi_init_sta(char * ssid, char * pass, int maximun_retry);
void wifi_stop_sta(void);

#endif /* __wifi_sta_h__ */
