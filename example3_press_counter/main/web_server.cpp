#include "esp_log.h"
#include <WiFi.h>
#include <WebServer.h>
#include "nvs_flash.h"
#include "web_server.h"
#include "web_pages.h"

static const char *TAG = "ROBONOMICS";

// Replace with your desired access point credentials
bool credentials_received = false;
UserData user_data;

// Create a WebServer object on port 80
WebServer server(80);


// Function to handle root URL
void handleRoot(const char* robonomicsSs58Address) {
    String htmlResponse = generateHtmlContent(robonomicsSs58Address);
    server.send(200, "text/html", htmlResponse);
}

// Function to handle saving Wi-Fi credentials
void handleSave() {
    user_data.ssid = server.arg("ssid");
    user_data.password = server.arg("password");
    user_data.host_url = server.arg("host_url");

    // Log the received SSID and password
    ESP_LOGI(TAG, "Received SSID: %s", user_data.ssid.c_str());
    ESP_LOGI(TAG, "Received Password: %s", user_data.password.c_str());
    ESP_LOGI(TAG, "Host Url: %s", user_data.host_url.c_str());
    credentials_received = true;

    // Send a response back to the client
    server.send(200, "text/html", "<h1>Credentials Saved!</h1><p>SSID: " + user_data.ssid + "</p><p>Password: " + user_data.password + "</p>");
}

UserData get_wifi_creds_from_user(const char* robonomicsSs58Address) {
    credentials_received = false;
    WiFi.softAP(AP_SSID, NULL);
    ESP_LOGI(TAG, "Access Point started");

    // Update route handlers to include the SS58 address
    server.on("/", [=]() { handleRoot(robonomicsSs58Address); });
    server.on("/save", HTTP_POST, handleSave);

    server.begin();

    while (!credentials_received)
    {
        server.handleClient();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    server.stop();
    WiFi.softAPdisconnect(true);
    
    return user_data;
}