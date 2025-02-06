#include <Arduino.h>
#include <USB.h>
#include <USBMSC.h>
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5Cardputer.h>

// MSC Object
USBMSC msc;

// SPI Instance
SPIClass sdSPI;

// Cardputer SD Card pins
constexpr int sdCardCSPin   = 12;
constexpr int sdCardMISOPin = 39;
constexpr int sdCardMOSIPin = 14;
constexpr int sdCardCLKPin  = 40;

void drawUSBStickIcon() {
    // Port USB
    M5.Lcd.fillRoundRect(155, 55, 40, 40, 5, LIGHTGREY);
    // Body
    M5.Lcd.fillRoundRect(45, 50, 112, 50, 5, DARKCYAN);
    // Small square on port
    M5.Lcd.fillRoundRect(164, 63, 20, 10, 5, DARKGREY);
    M5.Lcd.fillRoundRect(164, 78, 20, 10, 5, DARKGREY);
}

void drawLedIndicator(bool plugged) {
    M5.Lcd.fillRoundRect(54, 60, 5, 30, 5, plugged ? GREEN : RED);
}

void displayWelcome() {
    // Title
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(1.8);
    M5.Lcd.setCursor(20, 17);
    M5.Lcd.print("SD Card -> USB Stick");
    // Icon
    drawUSBStickIcon();
    // Press Mention
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(62, 71);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.print("PRESS ANY KEY");
    // Version
    M5.Lcd.setCursor(69, 120);
    M5.Lcd.setTextColor(TFT_LIGHTGREY);
    M5.Lcd.print("Version 0.1 - Geo");
    // Wait Press
    while(true){
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            break;
        }
        delay(5);
    }
    M5.Lcd.fillScreen(TFT_BLACK);
}

void displayMessage(std::string message) {
    M5.Lcd.fillRect(63, 28, 10, 17, TFT_BLACK);
    M5.Lcd.setCursor(66, 30);
    M5.Lcd.print(message.c_str());
}

void setup_usb_callback() {
  // Write Callback
  auto onWrite = [](uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) -> int32_t {
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0) return -1;  // disk error

    // For each sector in the buffer
    for (uint32_t x = 0; x < bufsize / secSize; ++x) {
      uint8_t blkBuffer[secSize];
      memcpy(blkBuffer, buffer + secSize * x, secSize);
      if (!SD.writeRAW(blkBuffer, lba + x)) {
        return -1;
      }
    }
    return bufsize;
  };

  // Read Callback
  auto onRead = [](uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) -> int32_t {
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0) return -1;  // disk error

    for (uint32_t x = 0; x < bufsize / secSize; ++x) {
      if (!SD.readRAW(reinterpret_cast<uint8_t*>(buffer) + (x * secSize), lba + x)) {
        return -1;
      }
    }
    return bufsize;
  };

  // Start/Stop Callback
  auto onStartStop = [](uint8_t power_condition, bool start, bool load_eject) -> bool {
    return true;
  };

  // USB MSC
  uint32_t secSize = SD.sectorSize();
  uint32_t numSectors = SD.numSectors();
  msc.vendorID("ESP32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  msc.begin(numSectors, secSize);
}

void setup_usb_event() {
  USB.onEvent([](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
      auto* data = reinterpret_cast<arduino_usb_event_data_t*>(event_data);
      switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT:
          drawLedIndicator(true);
          break;
        case ARDUINO_USB_STOPPED_EVENT:
          drawLedIndicator(false);
          break;
        case ARDUINO_USB_SUSPEND_EVENT:
          displayMessage("USB suspend");
          break;
        case ARDUINO_USB_RESUME_EVENT:
          displayMessage("USB resume");
          break;
        default:
          break;
      }
    }
  });
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  // Welcome screen and wait input
  displayWelcome();

  // SPI
  sdSPI.begin(sdCardCLKPin, sdCardMISOPin, sdCardMOSIPin, sdCardCSPin);

  // SD
  while(!SD.begin(sdCardCSPin, sdSPI)) {
    displayMessage("No SD Card found !");
    delay(1000);
  }

  // USB
  setup_usb_callback();
  setup_usb_event();
  displayMessage("USB Stick is ready");
  drawUSBStickIcon();
  drawLedIndicator(false);
  USB.begin();
}

void loop() {
  delay(1);
}
