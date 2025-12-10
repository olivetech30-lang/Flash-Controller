// ============================================
// ESP32-S3 Flash Controller Firmware
// Non-blocking LED Blink with Remote Control
// ============================================

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// ============================================
// CONFIGURATION - UPDATE THESE VALUES!
// ============================================

// WiFi Credentials
const char* WIFI_SSID = "HUAWEI Baig";        // <-- Change this!
const char* WIFI_PASSWORD = "cd6c696d"; // <-- Change this!

// Vercel API Endpoint
const char* API_URL = " https://flash-controller-full.vercel.app/api/delay"; // <-- Change this!


// NeoPixel Configuration
// For ESP32-S3 DevKitM-1, the built-in RGB LED is usually on GPIO 48
// Change this if your LED is on a different pin
#define NEOPIXEL_PIN    48
#define NUM_PIXELS      1
#define PIXEL_BRIGHTNESS 50

// Timing Configuration
const unsigned long API_POLL_INTERVAL = 1000;    // Poll API every 1 second
const unsigned long WIFI_RETRY_INTERVAL = 5000;  // WiFi reconnect interval
const unsigned long HTTP_TIMEOUT = 5000;         // HTTP request timeout

// Delay limits (must match API)
const int MIN_DELAY = 50;
const int MAX_DELAY = 2000;
const int DEFAULT_DELAY = 500;

// ============================================
// GLOBAL VARIABLES
// ============================================

// NeoPixel instance
Adafruit_NeoPixel pixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Timing variables (non-blocking)
unsigned long previousBlinkMillis = 0;
unsigned long previousApiMillis = 0;
unsigned long previousWifiCheckMillis = 0;

// State variables
int blinkDelay = DEFAULT_DELAY;    // Current blink delay in ms
bool ledState = false;              // Current LED on/off state
bool wifiConnected = false;         // WiFi connection status

// LED Colors
uint32_t COLOR_ON;
uint32_t COLOR_OFF;
uint32_t COLOR_WIFI_CONNECTING;
uint32_t COLOR_ERROR;
uint32_t COLOR_SUCCESS;

// ============================================
// FUNCTION DECLARATIONS
// ============================================

void setupWiFi();
void checkWiFiConnection();
void fetchDelayFromAPI();
void updateLED();
void setPixelColor(uint32_t color);

// ============================================
// SETUP
// ============================================

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(2000); // Give serial time to initialize
    
    Serial.println();
    Serial.println("============================================");
    Serial.println("   ESP32-S3 Flash Controller Starting...");
    Serial.println("============================================");
    
    // Initialize NeoPixel
    pixel.begin();
    pixel.setBrightness(PIXEL_BRIGHTNESS);
    pixel.clear();
    pixel.show();
    
    // Define colors after pixel.begin()
    COLOR_ON = pixel.Color(0, 0, 255);              // Blue when ON
    COLOR_OFF = pixel.Color(0, 0, 0);               // Off
    COLOR_WIFI_CONNECTING = pixel.Color(255, 165, 0); // Orange
    COLOR_ERROR = pixel.Color(255, 0, 0);           // Red
    COLOR_SUCCESS = pixel.Color(0, 255, 0);         // Green
    
    Serial.println("[LED] NeoPixel initialized on GPIO " + String(NEOPIXEL_PIN));
    
    // Test LED - flash blue 3 times
    Serial.println("[LED] Testing LED...");
    for (int i = 0; i < 3; i++) {
        setPixelColor(COLOR_ON);
        delay(200);
        setPixelColor(COLOR_OFF);
        delay(200);
    }
    Serial.println("[LED] Test complete");
    
    // Connect to WiFi
    setupWiFi();
    
    // Initialize timing
    previousBlinkMillis = millis();
    previousApiMillis = millis() - API_POLL_INTERVAL; // Fetch immediately
    
    Serial.println();
    Serial.println("[SYSTEM] Setup complete!");
    Serial.println("[SYSTEM] Current delay: " + String(blinkDelay) + "ms");
    Serial.println("============================================");
    Serial.println();
}

// ============================================
// MAIN LOOP (Non-blocking!)
// ============================================

void loop() {
    unsigned long currentMillis = millis();
    
    // ----------------------------------------
    // Task 1: Check WiFi connection periodically
    // ----------------------------------------
    if (currentMillis - previousWifiCheckMillis >= WIFI_RETRY_INTERVAL) {
        previousWifiCheckMillis = currentMillis;
        checkWiFiConnection();
    }
    
    // ----------------------------------------
    // Task 2: Poll API for delay value (every 1 second)
    // ----------------------------------------
    if (wifiConnected && (currentMillis - previousApiMillis >= API_POLL_INTERVAL)) {
        previousApiMillis = currentMillis;
        fetchDelayFromAPI();
    }
    
    // ----------------------------------------
    // Task 3: Non-blocking LED blink
    // ----------------------------------------
    if (currentMillis - previousBlinkMillis >= (unsigned long)blinkDelay) {
        previousBlinkMillis = currentMillis;
        updateLED();
    }
}

// ============================================
// WiFi FUNCTIONS
// ============================================

void setupWiFi() {
    Serial.println();
    Serial.print("[WiFi] Connecting to: ");
    Serial.println(WIFI_SSID);
    
    // Show connecting status on LED
    setPixelColor(COLOR_WIFI_CONNECTING);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection (with timeout)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        // Blink orange while connecting
        if (attempts % 2 == 0) {
            setPixelColor(COLOR_WIFI_CONNECTING);
        } else {
            setPixelColor(COLOR_OFF);
        }
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("[WiFi] ✓ Connected!");
        Serial.print("[WiFi] IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WiFi] Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.print("[WiFi] API URL: ");
        Serial.println(API_URL);
        
        // Flash green to indicate success
        for (int i = 0; i < 3; i++) {
            setPixelColor(COLOR_SUCCESS);
            delay(100);
            setPixelColor(COLOR_OFF);
            delay(100);
        }
    } else {
        wifiConnected = false;
        Serial.println("[WiFi] ✗ Connection FAILED!");
        Serial.println("[WiFi] Check your SSID and password");
        
        // Flash red to indicate error
        for (int i = 0; i < 5; i++) {
            setPixelColor(COLOR_ERROR);
            delay(200);
            setPixelColor(COLOR_OFF);
            delay(200);
        }
    }
}

void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        if (wifiConnected) {
            Serial.println("[WiFi] Connection lost! Attempting reconnect...");
            wifiConnected = false;
        }
        
        // Attempt to reconnect
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        // Quick non-blocking check
        delay(100);
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("[WiFi] Reconnected!");
            Serial.print("[WiFi] IP: ");
            Serial.println(WiFi.localIP());
        }
    } else if (!wifiConnected) {
        wifiConnected = true;
        Serial.println("[WiFi] Connection restored!");
    }
}

// ============================================
// API FUNCTIONS
// ============================================

// WiFi Secure Client for HTTPS
WiFiClientSecure secureClient;

void fetchDelayFromAPI() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[API] WiFi not connected, skipping fetch");
        return;
    }
    
    HTTPClient http;
    
    Serial.print("[API] Fetching from: ");
    Serial.println(API_URL);
    
    // IMPORTANT: Skip certificate verification for simplicity
    // In production, you should use a proper root CA certificate
    secureClient.setInsecure();
    
    // Use the secure client for HTTPS
    http.begin(secureClient, API_URL);
    http.setTimeout(HTTP_TIMEOUT);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.GET();
    
    Serial.print("[API] HTTP Response Code: ");
    Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        Serial.print("[API] Response: ");
        Serial.println(payload);
        
        // Parse JSON response using ArduinoJson v6
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            int newDelay = doc["delay"].as<int>();
            
            // Validate - check if value is valid
            if (newDelay == 0) {
                newDelay = DEFAULT_DELAY;
            }
            
            if (newDelay < MIN_DELAY || newDelay > MAX_DELAY) {
                Serial.print("[API] Invalid delay value: ");
                Serial.println(newDelay);
                newDelay = constrain(newDelay, MIN_DELAY, MAX_DELAY);
            }
            
            // Only log and update if changed
            if (newDelay != blinkDelay) {
                Serial.println("════════════════════════════════");
                Serial.print("[API] ★ DELAY CHANGED: ");
                Serial.print(blinkDelay);
                Serial.print("ms → ");
                Serial.print(newDelay);
                Serial.println("ms");
                Serial.println("════════════════════════════════");
                
                blinkDelay = newDelay;
            } else {
                Serial.println("[API] Delay unchanged: " + String(blinkDelay) + "ms");
            }
            
        } else {
            Serial.print("[API] JSON Parse Error: ");
            Serial.println(error.c_str());
        }
    } else if (httpCode < 0) {
        Serial.print("[API] Connection Error: ");
        Serial.println(http.errorToString(httpCode));
        Serial.println("[API] Tip: Check WiFi connection and API URL");
    } else {
        Serial.print("[API] HTTP Error: ");
        Serial.println(httpCode);
        String errorResponse = http.getString();
        Serial.print("[API] Error Response: ");
        Serial.println(errorResponse);
    }
    
    http.end();
}

// ============================================
// LED FUNCTIONS
// ============================================

void updateLED() {
    // Toggle LED state
    ledState = !ledState;
    
    if (ledState) {
        setPixelColor(COLOR_ON);
    } else {
        setPixelColor(COLOR_OFF);
    }
}

void setPixelColor(uint32_t color) {
    pixel.setPixelColor(0, color);
    pixel.show();
}

// ============================================
// END OF FIRMWARE
// ============================================
