#include <string>
//#include <string>
#include "Controller.h"

static const int16_t JOYSTICK_MIN_VALUE = 0;
static const int16_t JOYSTICK_MAX_VALUE = 10000;
static const int16_t JOYSTICK_RANGE = JOYSTICK_MAX_VALUE - JOYSTICK_MIN_VALUE;

#if defined USB_JOYSTICK
  #include <Joystick_ESP32S2.h>
  
  Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering
  
  void SetupController() {
    Joystick.setBrakeRange(JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
    delay(100);
    Joystick.begin();
  }
  bool IsControllerReady() { return true; }
  void SetControllerOutputValue(int32_t value) {
    Joystick.setBrake(value);
  }


  
  
#elif defined BLUETOOTH_GAMEPAD
  #include <BleGamepad.h>


  
  // get the max address 
  // see https://arduino.stackexchange.com/questions/58677/get-esp32-chip-id-into-a-string-variable-arduino-c-newbie-here
  char ssid[23];
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  unsigned int chip = (unsigned int)(chipid >> 32);
  std::string bluetoothName_lcl = "DiyActivePedal_" + std::to_string( chip );
  BleGamepad bleGamepad(bluetoothName_lcl, bluetoothName_lcl, 100);
  
  
  void SetupController() {
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setAxesMin(JOYSTICK_MIN_VALUE); // 0 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
    bleGamepadConfig.setAxesMax(JOYSTICK_MAX_VALUE); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal 
    //bleGamepadConfig.setWhichSpecialButtons(false, false, false, false, false, false, false, false);
    //bleGamepadConfig.setWhichAxes(false, false, false, false, false, false, false, false);
    bleGamepadConfig.setWhichSimulationControls(false, false, false, true, false); // only brake active 
    bleGamepadConfig.setButtonCount(0);
    bleGamepadConfig.setHatSwitchCount(0);
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setPid(chip); // product id

    bleGamepad.begin(&bleGamepadConfig);
  }

  bool IsControllerReady() { return bleGamepad.isConnected(); }

  void SetControllerOutputValue(int32_t value) {

    if (bleGamepad.isConnected() )
    {
      // bleGamepad.setBrake(value);
      bleGamepad.setAxes(0, 0, 0, 0, value, 0, 0, 0);
      bleGamepad.sendReport();
    }
    else
    {
      Serial.println("BLE not connected!");
      delay(500);
    }
    
    
  }
  
#endif


int32_t NormalizeControllerOutputValue(float value, float minVal, float maxVal, float maxGameOutput) {
  float valRange = (maxVal - minVal);
  if (abs(valRange) < 0.01) {
    return JOYSTICK_MIN_VALUE;   // avoid div-by-zero
  }

  float fractional = (value - minVal) / valRange;
  int32_t controller = JOYSTICK_MIN_VALUE + (fractional * JOYSTICK_RANGE);
  return constrain(controller, JOYSTICK_MIN_VALUE, (maxGameOutput/100.) * JOYSTICK_MAX_VALUE);
}
