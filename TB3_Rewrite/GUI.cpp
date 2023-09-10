#include "GUI.h"

void startProgram() {
    // Logic for starting the program
    //lcd.at(1, 1, "Program Started");
    delay(1000);
}

void exitProgram() {
    // Logic for exiting the program
    //lcd.at(1, 1, "Exiting...");
    delay(1000);
    // ... Further logic to safely shutdown or reset the system
}

void joggingMode() {
    // Logic for jogging mode
    //JoggingPage();  // Using the previously defined JoggingPage function
}


GUI::menu_item menu_items[] = {
    {"Start Program", startProgram},
    //{"Settings", settingsMenu},
    {"Jogging Mode", joggingMode},
    {"Exit", exitProgram}
};

GUI::GUI(LCDController& lcdObj, NunchuckController& nunchuckObj) : lcd(lcdObj), nunchuck(nunchuckObj) {
    current_position = 0;
    previous_position = -1;
}

GUI::Navigation GUI::get_input() {
    // Fetch and return the current input from Nunchuck
    return ENTER;  // No need for `Navigation::` here since you're within the GUI class scope
}

void GUI::handle_input() {
    Navigation input = get_input();
    switch (input) {
        case Navigation::UP:
            if (current_position > 0)
                current_position--;
            break;
        case Navigation::DOWN:
            if (current_position < (sizeof(menu_items) / sizeof(menu_items[0])) - 1)
                current_position++;
            break;
        case Navigation::ENTER:
            if (menu_items[current_position].actionFunc) {
                menu_items[current_position].actionFunc();
            }
            break;
        case Navigation::RETURN:
            // Handle return logic if needed
            break;
        case Navigation::NONE:
            break;
    }
}

void GUI::update_display() {
    if (current_position != previous_position) {
        lcd.clear();
        lcd.print(menu_items[current_position].displayText);
        previous_position = current_position;
    }
}

uint32_t GUI::handle() {
    if (lcd.isBusy())
        return micros() + SHORT_DELAY;

    handle_input();
    update_display();

    return micros() + UI_UPDATE_INTERVAL;
}
