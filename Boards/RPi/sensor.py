#!/usr/bin/python

from influxdb import InfluxDBClient
import json
import Adafruit_DHT
import time
import Adafruit_BMP.BMP085 as BMP085
quantum = 5
sensor_press = BMP085.BMP085()
sensor_temp = Adafruit_DHT.AM2302
pin = 4

while 1:
	humidity = None
	temperature = None
	while humidity is None or temperature is None:
		humidity, temperature = Adafruit_DHT.read_retry(sensor_temp, pin)
		time.sleep(1)

	data = [ 
		{
			"measurement": "weather",
			"tags": {
				"host": "rpi_g",
				"node": "unname"
			},
			"fields": {
				"temp": temperature ,
				"humidity":  humidity ,
				"pressure": sensor_press.read_pressure(),
				"temp2": sensor_press.read_temperature(),
				"altitude": sensor_press.read_altitude()
			}
		}
	]

	client = InfluxDBClient('10.150.13.3', '8086', 'root', 'root', 'db1')
	client.write_points(data)
	time.sleep(quantum)

