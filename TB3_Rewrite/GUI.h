#ifndef GUI_H
#define GUI_H

#include "LCDController.h"       // Replace with the actual name of your LCD library header
#include "NunchuckController.h"  // Replace with the actual name of your Nunchuck library header

constexpr uint32_t UI_UPDATE_INTERVAL = 330000;
constexpr uint32_t SHORT_DELAY = 10000;

class GUI {

  public:
    GUI(LCDController& lcdObj, NunchuckController& nunchuckObj);
    uint32_t handle();
    typedef enum {
      NONE, UP, DOWN, ENTER, RETURN
    } Navigation;

    typedef struct {
      const char* displayText;
      void (*actionFunc)();
    } menu_item;

  private:
    int current_position;
    int previous_position;
    LCDController& lcd;           // Storing references to the LCD and Nunchuck instances
    NunchuckController& nunchuck;

    void handle_input();
    void update_display();
    Navigation get_input();
};


#endif
