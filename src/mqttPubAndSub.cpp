/*
该实验使用MQTT的遗嘱服务,通过遗嘱来实现检测客户端的在线状态,
在void connectMQTTServer()函数中发布遗嘱,内容是客户端下线端的提示,
当客户端"死掉",就会发布遗嘱表明客户端已下线;同时在void connectMQTTServer()中有函数
publishOnlineStatus();,该函数在客户端工作后会工作一次(发布服务器在线信息)

*/
#include <PubSubClient.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>

WiFiClient client;
PubSubClient mqttClient(client);

//MQTT服务器,可以自己修改服务器
const char* mqttServer = "test.mosquitto.org";


//遗嘱信息以及用户名密码的设置(MQTT端的设置)
//const char* willTopic = "willTopic";    // 遗嘱主题名称
const char* willMsg = "CLIENT-OFFLINE"; // 遗嘱消息内容
const int willQoS = 0;                   // 遗嘱QoS
const bool willRetain = true;           // 遗嘱保留
const char* mqttUserName = "test-user"; // 服务端连接用户名
const char* mqttPassword = "ranye-iot"; // 服务端连接密码

//函数的声明
void MQTTInit();
void connectMQTTServer();
void pubMQTTmsg(String ID);
void subscribleTopic(String SubTopic);
void recieveCallback(char* topic, byte* payload, unsigned int length);
void publishOnlineStatus();
void loopMQTT();



/**
* @brief MQTT的初始化,有一些是不必要的,需要依据使用情况注释掉一些行
*
* @param 无
* @return 无
*/
void MQTTInit(){
    mqttClient.setServer(mqttServer, 1883); // (必要的)设置MQTT服务器和端口号
    mqttClient.setKeepAlive(10); //(可以不设置)设置心跳间隔时间
    //mqttClient.setCallback(recieveCallback);//如果需要订阅的消息作出反应需要设置,(在发布消息时不用设置)设置回调函数处理订阅消息
    connectMQTTServer();//(需要设置)连接MQTT服务器,这是一个后面写的函数,其中mqttClient.connect()可以有特别多的参数实现不同的功能
}



/**
* @brief 生成客户端名称并连接服务器,同时串口输出信息,发布以及订阅都需要连接服务器
        (但是订阅消息时还需要取消某行的注释)
*
* @param 无
* @return 无
*/
void connectMQTTServer(){
  //生成客户端的名称(同一个服务器下不能存在两个相同的客户端名称),可以修改,但最好别删除 WiFi.macAddress()
  String clientId = "esp32s3---" + WiFi.macAddress();


  // 建立遗嘱主题。主题名称以lingSou-为前缀，后面添加设备的MAC地址，最后
  // 以“-Will”结尾，这是为确保不同ESP32S3客户端的遗嘱主题名称各不相同。
  //需要使用遗嘱时才去掉注释使用遗嘱,遗嘱信息是在宏定义处定义的,可以不需要建立
  //你也可以不需要建立遗嘱主题,只需要宏定义一个遗嘱主题即可
  String willString = "lingSou-" + WiFi.macAddress() + "-Will";
  char willTopic[willString.length() + 1];  
  strcpy(willTopic, willString.c_str());
 


  //尝试连接服务器,并通过串口输出有关信息
  //通过mqttClient.connect()对这个函数输入参数的控制可以改变连接的状态
  /* 连接MQTT服务器
      boolean connect(const char* id, const char* user, 
                      const char* pass, const char* willTopic, 
                      uint8_t willQos, boolean willRetain, 
                      const char* willMessage, boolean cleanSession); 
    若让设备在离线时仍然能够让qos1工作，则connect时的cleanSession需要设置为false                
  */
  //需要用户名及密码时:mqttClient.connect(clientId.c_str(),mqttUserName,mqttPassword)
  //需要使用遗嘱时:mqttClient.connect(clientId.c_str(), willTopic, willQoS, willRetain, willMsg)
  if(mqttClient.connect(clientId.c_str(),willTopic,willQoS,willRetain,willMsg)){
      Serial.println("MQTT Server Connect successfully!!!.\n");
      Serial.println("Server Address: ");
      Serial.println(mqttServer);
      Serial.print("\n");
      Serial.println("ClientId:");
      Serial.println(clientId);
      Serial.print("\n");
      publishOnlineStatus();//发布消息
      //如果在需要订阅主题,在连接成功后应该订阅需要的主题
      //subscribleTopic();
  }
  else{
      Serial.print("MQTT Server Connect Failed. Client State:");
      Serial.println(mqttClient.state());
      Serial.print("\n");
      delay(3000);
  }
}



/**
* @brief 发布MQTT信息,包含建立主题以及发布消息,如果想的话可以自己改输入参数以便接入传感器发布消息
*
* @param String ID:这个参数是用来给发布的消息的主题命名的,不同的主题应该有不同的名字
* @return 无
*/
void pubMQTTmsg(String ID){
  static int value; // 客户端发布信息用数字,测试用

  // //这是使用用户名及密码时发布消息的操作,由于然也物联的限制,发布的主题必须有test-user/
  // // 建立发布主题。主题名称以test-user/Pub-为前缀，后面添加设备的MAC地址。
  // // 这么做是为确保不同用户进行MQTT信息发布时，ESP32S3客户端名称各不相同，
  // String topicString = "test-user/Pub-" + WiFi.macAddress();
  // char publishTopic[topicString.length() + 1];  
  // strcpy(publishTopic, topicString.c_str());
 
  // 建立发布主题。主题名称以lingsou-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP32s3客户端名称各不相同，
  //同时建立主题后的两句程序是将 string 的数据转换为 char[] 字符数组类型
  //因为在之后的发布操作中只支持字符数组作为参数
  String topicString = ID + WiFi.macAddress();  
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());  //将字符串数据 topicString 转换为字符数组类型的数据 publishTopic
 
  // 建立发布信息。信息内容以Hello World为起始，后面添加发布次数。(这个根据自己的需要更改发布消息)
  String messageString = "Hello World " + String(value++); 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  
  // 实现ESP32s3向主题发布信息
  if(mqttClient.publish(publishTopic, publishMsg)){
    Serial.print("Publish Topic:");
    Serial.print(publishTopic);
    Serial.print("\n");
    Serial.print("Publish message:");
    Serial.print(publishMsg);
    Serial.print("\n");    
  } else {
    Serial.print("Message Publish Failed.\n"); 
  }
}



/**
* @brief 发布MQTT信息,利用遗嘱消息发布开发板在线状态
*
* @param 无
* @return 无
*/
void publishOnlineStatus(){
  // 建立遗嘱主题。主题名称以lingSou-为前缀，后面添加设备的MAC地址，最后
  // 以“-Will”结尾，这是为确保不同ESP32S3客户端的遗嘱主题名称各不相同。
  String willString = "lingSou-" + WiFi.macAddress() + "-Will";
  char willTopic[willString.length() + 1];  
  strcpy(willTopic, willString.c_str());
 
  // 建立设备在线的消息。此信息将以保留形式向遗嘱主题发布
  String onlineMessageString = "CLIENT-ONLINE"; 
  char onlineMsg[onlineMessageString.length() + 1];   
  strcpy(onlineMsg, onlineMessageString.c_str());
  
  // 向遗嘱主题发布设备在线消息
  if(mqttClient.publish(willTopic, onlineMsg, true)){
    Serial.print("Published Online Message: ");Serial.println(onlineMsg);    
  } else {
    Serial.println("Online Message Publish Failed."); 
  }
}



/**
* @brief 订阅相关的主题(一共订阅了3个主题),一个普通主题,一个使用单级通配符,一个使用多级通配符,自己根据实际情况更改订阅主题的名称
*
* @param String SubTopic:需要将订阅的主题名称以字符串的形式输入,可以根据情况自行修改
* @return 无
*/
void subscribleTopic(String SubTopic){
  //建立订阅主题1。主题名称根据发布消息的开发板决定
  //这么做是为确保不同设备使用同一个MQTT服务器测试消息订阅时，所订阅的主题名称不同
  //需要将字符串转换为字符数组满足库的要求
  //这个主题由另一个开发板建立并发送
  String topicString1 = SubTopic;
  char subTopic1[topicString1.length() + 1];  
  strcpy(subTopic1, topicString1.c_str());
  
//   // 建立订阅主题2,使用单级通配符
//   //这个主题由MQTTfx建立并发送
//   String topicString2 = "lingsou-" + WiFi.macAddress() + "/+/data";
//   char subTopic2[topicString2.length() + 1];  
//   strcpy(subTopic2, topicString2.c_str());

//   // 建立订阅主题3,使用多级通配符
//   //这个主题由MQTTfx建立并发送
//   String topicString3 = "lingsou-" + WiFi.macAddress() + "/sensor/#";
//   char subTopic3[topicString3.length() + 1];  
//   strcpy(subTopic3, topicString3.c_str());
  
  // 通过串口监视器输出是否成功订阅主题1以及订阅的主题1名称
  if(mqttClient.subscribe(subTopic1)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic1);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }  
 
//   // 通过串口监视器输出是否成功订阅主题2以及订阅的主题2名称
//   if(mqttClient.subscribe(subTopic2)){
//     Serial.println("Subscrib Topic:");
//     Serial.println(subTopic2);
//     Serial.print("\n");
//   } else {
//     Serial.print("Subscribe Fail...");
//     Serial.print("\n");
//   }

//   // 通过串口监视器输出是否成功订阅主题3以及订阅的主题3名称
//   if(mqttClient.subscribe(subTopic3)){
//     Serial.println("Subscrib Topic:");
//     Serial.println(subTopic3);
//     Serial.print("\n");
//   } else {
//     Serial.print("Subscribe Fail...");
//     Serial.print("\n");
//   }
}



/**
* @brief 这是一个回调函数,当订阅的主题有消息发布时就会调用该函数,参数是固定的(PunSubClient中固定的),不能自己修改
*
* @param char* topic :这是订阅的主题名
* @param byte* payload :这是传回的消息
* @param unsigned int length :这是消息长度
* @return 无
*/
void recieveCallback(char* topic, byte* payload, unsigned int length){
  //输出订阅的主题名称
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("\n");

  //输出订阅的主题中的消息
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
  Serial.print("\n");
    
//  // 展示回调函数的用法
//  //根据主题信息控制LED灯的亮灭
//   if ((char)payload[0] == '1') {     // 如果收到的信息以“1”为开始
//     digitalWrite(LED_A, 1);  // 则点亮LED。
//     digitalWrite(LED_B, 1);  // 则点亮LED。
//   } else {                           
//     digitalWrite(LED_A, 0); // 否则熄灭LED。
//     digitalWrite(LED_B, 0); // 否则熄灭LED。
//   }
}



//
void loopMQTT(){
  if(!mqttClient.connected()){
    connectMQTTServer();
  }
  else{
    //自定义操作
  }

  //处理信息以及心跳
  mqttClient.loop();
}