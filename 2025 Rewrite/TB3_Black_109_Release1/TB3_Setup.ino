/*

(c) 2015 Brian Burling eMotimo INC


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

__attribute__ ((const))
__attribute__ ((warn_unused_result))
const char* BoolString(const bool value) {
    return value ? F_STR("ON ") : F_STR("OFF");
}


void Setup_AUX_ON() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Aux Motor:"), 1, 1);
    draw(BoolString(config.AUX_ON), 1, 12);
    draw(F_STR("UpDown  C-Select"), 2, 1);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.AUX_ON = static_cast<bool>(updateWithWraparound(static_cast<int16_t>(config.AUX_ON), 0, 1, yUpDown));
    draw(BoolString(config.AUX_ON), 1, 12);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}

void Setup_PAUSE_ENABLED() {

  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Pause "), 1, 1);
    draw(BoolString(config.PAUSE_ENABLED), 1, 8);
    draw(F_STR("UpDown  C-Select"), 2, 1);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.PAUSE_ENABLED = static_cast<bool>(updateWithWraparound(static_cast<int16_t>(config.PAUSE_ENABLED), 0, 1, yUpDown));
    draw(BoolString(config.PAUSE_ENABLED), 1, 8);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}

void Setup_POWERSAVE_PT() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("PT Motors on"), 1, 1);
    DisplayPowerSave(config.POWERSAVE_PT);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.POWERSAVE_PT = static_cast<Powersave>(updateWithWraparound(static_cast<int16_t>(config.POWERSAVE_PT), 0, static_cast<int16_t>(Powersave::Count) - 1, yUpDown));
    DisplayPowerSave(config.POWERSAVE_PT);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}

void Setup_POWERSAVE_AUX() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("AUX Motors On:"), 1, 1);
    DisplayPowerSave(config.POWERSAVE_AUX);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.POWERSAVE_AUX = static_cast<Powersave>(updateWithWraparound(static_cast<int16_t>(config.POWERSAVE_AUX), 0, static_cast<int16_t>(Powersave::Count) - 1, yUpDown));
    DisplayPowerSave(config.POWERSAVE_AUX);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}

void DisplayPowerSave(const enum Powersave value) {
  lcd.clearRegion(2, 7, 16);
  switch(value) {
    case Powersave::Count:
    case Powersave::Always:         { draw(F_STR("Always"), 2, 1); break; }
    case Powersave::Programs:       { draw(F_STR("Program"), 2, 1); break; }
    case Powersave::Shoot_Accuracy: { draw(F_STR("Shoot (accuracy)"), 2, 1); break; }
    case Powersave::Shoot_Minimum:  { draw(F_STR("Shoot (pwr save)"), 2, 1); break; }
  }
}

void Setup_LCD_BRIGHTNESS() {  //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("BkLite On Run: "), 1, 1);
    lcd.bright(config.LCD_BRIGHTNESS_DURING_RUN);
    lcd.at(1, 15, config.LCD_BRIGHTNESS_DURING_RUN);
    draw(F_STR("UpDown  C-Select"), 2, 1);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.LCD_BRIGHTNESS_DURING_RUN = static_cast<uint8_t>(updateWithWraparound(static_cast<int16_t>(config.LCD_BRIGHTNESS_DURING_RUN), 1, 8, yUpDown));
    lcd.at(1, 15, config.LCD_BRIGHTNESS_DURING_RUN);
    lcd.bright(config.LCD_BRIGHTNESS_DURING_RUN);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}

void Setup_Max_AUX_Motor_Speed() {  //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Max Speed:  "), 1, 1);
    DrawPaddedValue<5>(config.MAX_JOG_STEPS_PER_SEC[2], 1, 12);
    draw(F_STR("UpDown  C-Select"), 2, 1);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.MAX_JOG_STEPS_PER_SEC[2] = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.MAX_JOG_STEPS_PER_SEC[2]), 500, 20000, yUpDown * 500)) / 500 * 500;
    DrawPaddedValue<5>(config.MAX_JOG_STEPS_PER_SEC[2], 1, 12);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}

void Setup_AUX_Motor_DIR() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Aux Reversed:"), 1, 1);
    draw(BoolString(config.AUX_REV), 1, 14);
    draw(F_STR("UpDown  C-Select"), 2, 1);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    config.AUX_REV = static_cast<bool>(updateWithWraparound(static_cast<int16_t>(config.AUX_REV), 0, 1, yUpDown));
    draw(BoolString(config.AUX_REV), 1, 14);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    saveConfigToEEPROM(config);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    saveConfigToEEPROM(config);
  }
}