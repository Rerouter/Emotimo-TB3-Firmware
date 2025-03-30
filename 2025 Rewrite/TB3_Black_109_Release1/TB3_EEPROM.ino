/* 
  ========================================
  EEPROM write/read functions
  ========================================
  
*/
/*

This code started with Chris Church's OpenMoco Time Lapse Engine and was modified by Brian Burling
to work with the eMotimo products.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <EEPROM.h>

// Constants for the saved state
const uint8_t SAVED_FLAG_ADDR = 0;  // Address to store the "saved" flag
const uint8_t VERSION_ADDR = 1;     // Address to store version info
const uint8_t CONFIG_ADDR = 2;      // Address to store version info

// Ensure ConfigData fits into EEPROM
constexpr size_t EEPROM_SIZE = E2END + 1 - CONFIG_ADDR;
static_assert(sizeof(ConfigData) <= EEPROM_SIZE, "ConfigData size exceeds EEPROM capacity!");


void setDefaultsInRAM(struct ConfigData &intconfig) {
  intconfig.setDefaults();  // Use the struct's default initializer
}

void saveConfigToEEPROM(const struct ConfigData &intconfig) {
  EEPROM.put(CONFIG_ADDR, intconfig);
}

void loadConfigFromEEPROM(struct ConfigData &intconfig) {
  EEPROM.get(CONFIG_ADDR, intconfig);
}

__attribute__ ((warn_unused_result))
uint16_t check_version() {
  uint16_t check_version_from_eeprom;
  eepromRead(1, check_version_from_eeprom);
  return check_version_from_eeprom;
}

__attribute__ ((warn_unused_result))
bool getSavedState() {
  return EEPROM.read(SAVED_FLAG_ADDR) == 1;
}


void setSavedState(const bool saved) {
  // EEPROM memory is by default set to 1, so we
  // set it to zero if we've written data to eeprom
  EEPROM.write(SAVED_FLAG_ADDR, !saved);
}


// Generic function to write data to EEPROM
template<typename T>
void eepromWrite(const uint8_t pos, const T &value) {
  EEPROM.put(pos, value);
  setSavedState(true);  // Update saved state
}


// Generic function to read data from EEPROM
template<typename T>
void eepromRead(const uint8_t pos, T &value) {
  EEPROM.get(pos, value);
}


// restore memory

void review_RAM_Contents() {
  Serial.println(build_version);
  Serial.println(first_ui);
  Serial.println(config.shot_interval_time);
  Serial.println(config.camera_fired);
  Serial.println(config.camera_total_shots);
  Serial.println(config.overaldur);
  Serial.println(config.rampval);
  Serial.println(config.lead_in);
  Serial.println(config.lead_out);
  Serial.println(config.motor_steps_pt[2][0]);
  Serial.println(config.motor_steps_pt[2][1]);
  Serial.println(config.motor_steps_pt[2][2]);
  Serial.println(config.linear_steps_per_shot[0]);
  Serial.println(config.linear_steps_per_shot[1]);
  Serial.println(config.linear_steps_per_shot[2]);
  Serial.println(config.ramp_params_steps[0]);
  Serial.println(config.ramp_params_steps[1]);
  Serial.println(config.ramp_params_steps[2]);
  Serial.println(static_cast<uint8_t>(config.POWERSAVE_PT));
  Serial.println(static_cast<uint8_t>(config.POWERSAVE_AUX));
  Serial.println(config.AUX_ON);
  Serial.println(config.PAUSE_ENABLED);
  Serial.println(config.LCD_BRIGHTNESS_DURING_RUN);
  Serial.println(config.MAX_JOG_STEPS_PER_SEC[2]);
}
