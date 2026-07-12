//#include <Arduino.h>
//#include <TinyGsmClient.h>
//#include <PubSubClient.h>

//#include "MQTT.h"
//#include "Secrets.h"
//#include "Modem.h"

//TinyGsmClientSecure secureClient(modem);
//PubSubClient client(secureClient);

//MQTT mqtt;

//bool MQTT::begin()
//{
    // TEMPORARY for initial testing

  //  client.setServer(MQTT_HOST, MQTT_PORT);

    //Serial.println("Connecting to MQTT...");

    //if (!client.connect(
    //        MQTT_CLIENT_ID,
    //        MQTT_USER,
     //       MQTT_PASSWORD))
    //{
    //    Serial.print("MQTT failed, rc=");
    //    Serial.println(client.state());
    //    return false;
    //}

    //Serial.println("MQTT Connected!");

    //return true;
//}

//void MQTT::loop()
//{
//    client.loop();
//}

//bool MQTT::publishAmbient(float temp)
//{
//    char buffer[16];
//    snprintf(buffer, sizeof(buffer), "%.1f", temp);

//    return client.publish("camper/ambient", buffer, true);
//}
#include <Arduino.h>

#include "MQTT.h"

MQTT mqtt;

bool MQTT::begin()
{
    Serial.println("MQTT TLS not configured yet.");
    return false;
}

void MQTT::loop()
{
}

bool MQTT::publishAmbient(float temperatureF)
{
    (void)temperatureF;
    return false;
} 