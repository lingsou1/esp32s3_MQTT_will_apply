/*
接线说明:无

程序说明:遗嘱机制的应用(客户端在线状态的检测)

注意事项:无

函数示例:无

作者:灵首

时间:2023_5_25

*/
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "wifiSet.h"
#include "mqttPubAndSub.h"



void setup() {
  Serial.begin(9600);
  Serial.print("serial is OK!!!\n");

  //wifi设置及连接
  wifi_multi_init();
  wifi_multi_con();

  //初始化MQTT服务
  MQTTInit();

  //连接MQTT服务器(若需要订阅还可以订阅主题)
  connectMQTTServer();
}

void loop() {
  loopMQTT();
}