#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ssid = "Hello";
const char* password = "Hello";

const String weatherApiKey = "Hello";
const String stockApiKey = "Hello";
const String octoPrintApiKey = "Hello";

const String city = "Caledon%20East";
const String countryCode = "CA";

String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&appid=" + weatherApiKey + "&units=metric";
String stockBaseUrl = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&apikey=" + stockApiKey + "&symbol=";
String octoPrintUrl = "http://192.168.2.201:5000/api/job";
String octoPrintCommandUrl = "http://192.168.2.201:5000/api/job";
String mjpegStreamUrl = "http://192.168.2.244/img/video.mjpeg?channel=1";

String currentWeather = "clear";
float currentTemp = 0;
String clothingRecommendation = "T-shirt & shorts";
float aaplPrice = 0.0, tslaPrice = 0.0, googPrice = 0.0;
String printJobName = "None";
float printProgress = 0.0;
String userNotes = "";

AsyncWebServer server(80);
WiFiClient wifiClient;

const int timezoneOffsetSeconds = -5 * 3600;
const int dstOffsetSeconds = 3600;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting ESP8266 IoT Dashboard...");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    configTime(timezoneOffsetSeconds, dstOffsetSeconds, "pool.ntp.org", "time.nist.gov");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = generateHTML();
        request->send(200, "text/html", html);
    });

    server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", getTime());
    });

    server.on("/api/stocks", HTTP_GET, [](AsyncWebServerRequest* request) {
        String json = "{\"AAPL\": " + String(aaplPrice) + ", \"TSLA\": " + String(tslaPrice) + ", \"GOOG\": " + String(googPrice) + "}";
        request->send(200, "application/json", json);
    });

    server.on("/api/notes", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("note", true)) {
            userNotes = request->getParam("note", true)->value();
            request->send(200, "text/plain", "Note saved");
        } else {
            request->send(400, "text/plain", "No note provided");
        }
    });

    server.on("/api/octoprint", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("command", true)) {
            String command = request->getParam("command", true)->value();
            HTTPClient http;
            http.begin(wifiClient, octoPrintCommandUrl);
            http.addHeader("X-Api-Key", octoPrintApiKey);
            http.addHeader("Content-Type", "application/json");
            String payload = "{\"command\": \"" + command + "\"}";
            int httpCode = http.POST(payload);
            if (httpCode == 204) {
                request->send(200, "text/plain", "Command executed");
            } else {
                request->send(400, "text/plain", "Command failed");
            }
            http.end();
        } else {
            request->send(400, "text/plain", "No command provided");
        }
    });

    server.begin();
    Serial.println("Web server started at: http://" + WiFi.localIP().toString());
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 60000) {
        fetchWeather();
        fetchStockPrices();
        fetchOctoPrintStatus();
        lastUpdate = millis();
    }
}

void fetchWeather() {
    Serial.println("Fetching weather data...");
    HTTPClient http;
    http.begin(wifiClient, weatherUrl);
    int httpCode = http.GET();

    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        currentWeather = doc["weather"][0]["description"].as<String>();
        currentTemp = doc["main"]["temp"].as<float>();
        Serial.println("Weather updated successfully!");
        Serial.println("Current Weather: " + currentWeather);
        Serial.println("Temperature: " + String(currentTemp) + " C");
        updateClothingRecommendation();
    } else {
        Serial.println("Failed to fetch weather data. HTTP code: " + String(httpCode));
    }
    http.end();
}

void fetchStockPrices() {
    aaplPrice = fetchStockPrice("AAPL");
    tslaPrice = fetchStockPrice("TSLA");
    googPrice = fetchStockPrice("GOOG");
}

float fetchStockPrice(String symbol) {
    HTTPClient http;
    http.begin(wifiClient, stockBaseUrl + symbol);
    int httpCode = http.GET();

    float price = 0.0;
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        price = doc["Global Quote"]["05. price"].as<float>();
    }
    http.end();
    return price;
}

void fetchOctoPrintStatus() {
    Serial.println("Fetching OctoPrint data...");
    HTTPClient http;
    http.begin(wifiClient, octoPrintUrl);
    http.addHeader("X-Api-Key", octoPrintApiKey);
    int httpCode = http.GET();

    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        printJobName = doc["job"]["file"]["name"].as<String>();
        printProgress = doc["progress"]["completion"].as<float>();
        Serial.println("OctoPrint updated successfully!");
        Serial.println("Current Job: " + printJobName);
        Serial.println("Progress: " + String(printProgress) + "%");
    } else {
        Serial.println("Failed to fetch OctoPrint data. HTTP code: " + String(httpCode));
    }
    http.end();
}

void updateClothingRecommendation() {
    if (currentTemp > 25) {
        clothingRecommendation = "T-shirt & shorts";
    } else if (currentTemp > 15) {
        clothingRecommendation = "Light jacket";
    } else {
        clothingRecommendation = "Coat & warm layers";
    }
}

String generateHTML() {
    String html = "<!DOCTYPE html><html><head><title>ESP8266 IoT Dashboard</title>";
    html += "<style>body { font-family: Arial; background: #fff; color: #000; text-align: center; margin: 0; padding: 0; }";
    html += ".header { background: #4caf50; color: white; padding: 10px 0; font-size: 24px; }";
    html += ".content { padding: 20px; }";
    html += ".card { background: #4caf50; color: white; margin: 20px auto; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.3); max-width: 400px; }";
    html += ".card h2 { color: white; } .footer { background: #4caf50; color: white; padding: 10px 0; margin-top: 20px; }";
    html += "</style>";

    html += "<script>setInterval(async()=>{document.getElementById('time').textContent=await(await fetch('/api/time')).text();},1000);</script>";
    html += "<script>setInterval(async()=>{let s=await fetch('/api/stocks').then(r=>r.json());document.getElementById('stocks').innerHTML=\"<p><strong>AAPL:</strong> $\"+s.AAPL+\"</p><p><strong>TSLA:</strong> $\"+s.TSLA+\"</p><p><strong>GOOG:</strong> $\"+s.GOOG+\"</p>\";},60000);</script>";

    html += "</head><body>";
    html += "<div class='header'>ESP8266 IoT Dashboard</div>";

    html += "<div class='content'>";
    html += "<div class='card'><h2>Current Time</h2><p id='time'></p></div>";

    html += "<div class='card'><h2>Weather</h2>";
    html += "<p><strong>Condition:</strong> " + currentWeather + "</p>";
    html += "<p><strong>Temperature:</strong> " + String(currentTemp) + " &#8451;</p></div>";

    html += "<div class='card'><h2>What to Wear</h2>";
    html += "<p>" + clothingRecommendation + "</p></div>";

    html += "<div class='card'><h2>Stock Prices</h2><div id='stocks'>";
    html += "<p><strong>AAPL:</strong> $" + String(aaplPrice) + "</p>";
    html += "<p><strong>TSLA:</strong> $" + String(tslaPrice) + "</p>";
    html += "<p><strong>GOOG:</strong> $" + String(googPrice) + "</p></div></div>";

    html += "<div class='card'><h2>3D Printer Progress</h2>";
    html += "<p><strong>Job:</strong> " + printJobName + "</p>";
    html += "<p><strong>Progress:</strong> " + String(printProgress) + "%</p>";
    html += "<div style='background: #666; border-radius: 5px; margin-top: 10px;'>";
    html += "<div style='background: #4caf50; height: 20px; border-radius: 5px; width:" + String(printProgress) + "%'></div></div></div>";

    html += "<div class='card'><h2>3D Printer Stream</h2>";
    html += "<img src='" + mjpegStreamUrl + "' style='width:100%; border-radius:10px;' /></div>";

html += "<div class='card'><h2>3D Printer Controls</h2>";
html += "<button onclick=\"sendCommand('start')\">Start</button>";
html += "<button onclick=\"sendCommand('pause')\">Pause</button>";
html += "<button onclick=\"sendCommand('cancel')\">Cancel</button>";
html += "</div>";

html += "<script>";
html += "function sendCommand(command) {";
html += "  fetch('/api/octoprint', {";
html += "    method: 'POST',";
html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },";
html += "    body: 'command=' + command";
html += "  }).then(response => response.text()).then(alert).catch(console.error);";
html += "}";
html += "</script>";

html += "</body></html>";

    return html;
}

String getTime() {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    return String(buf);
}
  
