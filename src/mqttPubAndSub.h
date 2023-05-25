#ifndef MQTTPUBANDSUB_H
#define MQTTPUBANDSUB_H

#include <Arduino.h>


void MQTTInit();
void connectMQTTServer();
void pubMQTTmsg(String ID);
void subscribleTopic(String SubTopic);
void recieveCallback(char* topic, byte* payload, unsigned int length);
void publishOnlineStatus();
void loopMQTT();

#endif