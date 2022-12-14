#include <Arduino.h>

// Enable support for escaped unicode characters (\u1023)
#define ARDUINOJSON_DECODE_UNICODE 1

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace
{
    const char *ssid = "VTCC";          // your network SSID (name of wifi network)
    const char *password = "vtcc40pbc"; // your network password

    const char *server = "www.reddit.com";
    const char *api_request = "/r/arduino.json";
}

WiFiClientSecure client;

enum class HTTP_CODES
{
    HTTP_OK,
    CONN_FAILED,
    SEND_FAILED,
    STATUS_FAILED,
    HEADER_FAILED,
};

HTTP_CODES requestHTTPs(WiFiClientSecure &client);

void analyzeJson(JsonDocument &doc, WiFiClientSecure &client);

void dump(WiFiClientSecure &client);

void setup_wifi();

void setup()
{
    Serial.begin(115200);
    setup_wifi();
    while (requestHTTPs(client) != HTTP_CODES::HTTP_OK)
        delay(500);

    // dump(client);

    DynamicJsonDocument doc(6 * 1024);
    analyzeJson(doc, client);

    Serial.println("\nDone.");
    client.stop();
}

void loop()
{
    // put your main code here, to run repeatedly:
}

HTTP_CODES requestHTTPs(WiFiClientSecure &client)
{ // Send HTTP request and Consume the header
  // Return: responseCodes
    client.setTimeout(10000);
    client.setInsecure(); // skip verification
    if (!client.connect(server, 443))
    {
        Serial.println("Connection failed!");
        return HTTP_CODES::CONN_FAILED;
    }
    yield();

    // Make a HTTP request:
    client.printf("GET %s HTTP/1.0\n", api_request);
    client.printf("Host: %s\n", server);
    client.println("User-Agent: ESP32");
    client.println("Connection: close");
    client.println("Cache-Control: no-cache");
    if (client.println() == 0)
    {
        Serial.println("Failed to send request");
        client.stop();
        return HTTP_CODES::SEND_FAILED;
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status + 9, "200 OK") != 0)
    {
        Serial.print("Unexptected HTTP status: ");
        Serial.println(status);
        client.stop();
        return HTTP_CODES::STATUS_FAILED;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders))
    {
        Serial.println("Invalid header");
        client.stop();
        return HTTP_CODES::HEADER_FAILED;
    }
    return HTTP_CODES::HTTP_OK;
}

void analyzeJson(JsonDocument &doc, WiFiClientSecure &client)
{
    StaticJsonDocument<128> filter;
    filter["data"]["children"][0]["data"]["title"] = true;
    filter["data"]["children"][0]["data"]["score"] = true;

    Serial.print("filter.memoryUsage(): ");
    Serial.println(filter.memoryUsage());
    serializeJsonPretty(filter, Serial);
    Serial.println();

    DeserializationError err =
        deserializeJson(doc, client, DeserializationOption::Filter(filter));

    if (err)
    {
        Serial.print("deserializeJson() returned: ");
        Serial.println(err.c_str());
        // client.stop();
        return;
    }

    Serial.print("doc.memoryUsage(): ");
    Serial.println(doc.memoryUsage());
    serializeJsonPretty(doc, Serial);
    Serial.println();
    // Extract the list of forecasts
    JsonArray children = doc["data"]["children"];

    for (JsonObject child : children)
    {
        Serial.print(child["data"]["score"].as<long>());
        Serial.print(" : ");
        Serial.println(child["data"]["title"].as<const char *>());
    }
}

/*----- Helper Functions -----*/
void dump(WiFiClientSecure &client)
{
    while (client.connected())
    {
        while (client.available())
        {
            char c = client.read();
            Serial.write(c);
        }
        yield();
    }
}

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.printf("Connected to %s\n", ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
/*----------------------------*/