# -*- coding: utf-8 -*-
import json
import requests

response = requests.get('https://www.metaweather.com/api/location/1118370/')
#r = requests.get('https://github.com/timeline.json')
print(response)
print(response.status_code)
print(response.headers)
print(response.text)
data = response.json()
print json.dumps(data, indent=4)

