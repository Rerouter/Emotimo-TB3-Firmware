char lcdbuffer1[17]; //this used to be 16, but increased to 20 do to overflow when we moved to Arduino 1.6 (stalled and failed)
//                               0123456789ABCDEF
const char setup_0[]  PROGMEM = "Setup Complete";
const char setup_1[]  PROGMEM = "   TB3_109_RF";
const char setup_2[]  PROGMEM = "Connect Joystick";
const char setup_3[]  PROGMEM = "C-Next Z-Go Back";
const char setup_4[]  PROGMEM = "C-Use Stored";
const char setup_5[]  PROGMEM = "Z-Reset";
const char setup_6[]  PROGMEM = "Params Reset";
const char setup_7[]  PROGMEM = "Low Power";
const char setup_8[]  PROGMEM = "Move to Start Pt";
const char setup_9[]  PROGMEM = "Start Pt. Set";
const char setup_10[] PROGMEM = "Move to Point";
const char setup_11[] PROGMEM = "Moving to stored";
const char setup_12[] PROGMEM = "end point";
const char setup_13[] PROGMEM = "Confirm or Move";
const char setup_14[] PROGMEM = "C-Next";
const char setup_15[] PROGMEM = "Move to End Pt.";
const char setup_16[] PROGMEM = "End Point Set";
const char setup_17[] PROGMEM = "Set Sht Interval";
const char setup_18[] PROGMEM = "Intval:   .  sec";
const char setup_19[] PROGMEM = " Ext.Trig";
const char setup_20[] PROGMEM = " Video   ";
const char setup_21[] PROGMEM = "Interval Set";
const char setup_22[] PROGMEM = "Set Static Time";
const char setup_23[] PROGMEM = "Stat_T:   .  sec";
const char setup_24[] PROGMEM = " Video   ";
const char setup_25[] PROGMEM = "Static Time Set";
const char setup_26[] PROGMEM = "Set PreFire Time";
const char setup_27[] PROGMEM = " Pre_T:   .  sec";
const char setup_28[] PROGMEM = "Prefire Time Set";
const char setup_29[] PROGMEM = "    Set Ramp";
const char setup_30[] PROGMEM = "Ramp:     Frames";
const char setup_31[] PROGMEM = "Ramp Set";
const char setup_32[] PROGMEM = "Set Move";
const char setup_33[] PROGMEM = "Duration";
const char setup_34[] PROGMEM = "H:MM:SS";
const char setup_35[] PROGMEM = "Duration Set";
const char setup_36[] PROGMEM = "Set Static Lead";
const char setup_37[] PROGMEM = "In/Out Frames";
const char setup_38[] PROGMEM = "In      Out";
const char setup_39[] PROGMEM = "Lead Frames Set";
const char setup_40[] PROGMEM = " Going to Start";
const char setup_41[] PROGMEM = "Review and";
const char setup_42[] PROGMEM = "Confirm Setting";
const char setup_43[] PROGMEM = "Pan Steps:";
const char setup_44[] PROGMEM = "Tilt Steps:";
const char setup_45[] PROGMEM = "Cam Shots:";
const char setup_46[] PROGMEM = "Time:";
const char setup_47[] PROGMEM = "Ready?";
const char setup_48[] PROGMEM = "Press C Button";
const char setup_49[] PROGMEM = "Program Running";
const char setup_50[] PROGMEM = "Waiting for Ext.";
const char setup_51[] PROGMEM = "LeadIn";
const char setup_52[] PROGMEM = "RampUp";
const char setup_53[] PROGMEM = "Linear";
const char setup_54[] PROGMEM = "RampDn";
const char setup_55[] PROGMEM = "LeadOT";
const char setup_56[] PROGMEM = "Finish";
const char setup_57[] PROGMEM = "Center Joystick";
const char setup_58[] PROGMEM = "Program Complete";
const char setup_59[] PROGMEM = " Repeat Press C";
const char setup_60[] PROGMEM = "Battery too low";
const char setup_61[] PROGMEM = "  to continue";
const char setup_62[] PROGMEM = "Pause ";
const char setup_63[] PROGMEM = "Point X Set";
const char setup_64[] PROGMEM = "Using Set Params";
const char setup_65[] PROGMEM = "UpDown  C-Select";
const char setup_66[] PROGMEM = "New   Point Move";
const char setup_67[] PROGMEM = "Enabled";
const char setup_68[] PROGMEM = "Disabled";
const char setup_69[] PROGMEM = "PowerSave";
const char setup_70[] PROGMEM = "Always";
const char setup_71[] PROGMEM = "Program";
const char setup_72[] PROGMEM = "Shoot (accuracy)";
const char setup_73[] PROGMEM = "Shoot (pwr save)";
const char setup_74[] PROGMEM = "Aux Motor:";
const char setup_75[] PROGMEM = "Set Angle o'View";
const char setup_76[] PROGMEM = "C-Set, Z-Reset";
const char setup_77[] PROGMEM = "Pan AOV: ";
const char setup_78[] PROGMEM = "Tilt AOV: ";
const char setup_79[] PROGMEM = "   % Overlap";
const char setup_80[] PROGMEM = "Overlap Set";
const char setup_81[] PROGMEM = "Rev   Point Move";
const char setup_82[] PROGMEM = "DF Slave Mode";
const char setup_83[] PROGMEM = "Setup Menu";
const char setup_84[] PROGMEM = "Panorama";
const char setup_85[] PROGMEM = "AuxDistance";
const char setup_86[] PROGMEM = "Resume";
const char setup_87[] PROGMEM = "Restart";
const char setup_88[] PROGMEM = "GoTo Frame:";
const char setup_89[] PROGMEM = "Go to End";
const char setup_90[] PROGMEM = "90";


PGM_P const setup_str[] PROGMEM = {setup_0,  setup_1,  setup_2,  setup_3,  setup_4,  setup_5,  setup_6,  setup_7,  setup_8,  setup_9,
                                   setup_10, setup_11, setup_12, setup_13, setup_14, setup_15, setup_16, setup_17, setup_18, setup_19,
                                   setup_20, setup_21, setup_22, setup_23, setup_24, setup_25, setup_26, setup_27, setup_28, setup_29,
                                   setup_30, setup_31, setup_32, setup_33, setup_34, setup_35, setup_36, setup_37, setup_38, setup_39,
                                   setup_40, setup_41, setup_42, setup_43, setup_44, setup_45, setup_46, setup_47, setup_48, setup_49,
                                   setup_50, setup_51, setup_52, setup_53, setup_54, setup_55, setup_56, setup_57, setup_58, setup_59,
                                   setup_60, setup_61, setup_62, setup_63, setup_64, setup_65, setup_66, setup_67, setup_68, setup_69,
                                   setup_70, setup_71, setup_72, setup_73, setup_74, setup_75, setup_76, setup_77, setup_78, setup_79,
                                   setup_80, setup_81, setup_82, setup_83, setup_84, setup_85, setup_86, setup_87, setup_88, setup_89,
                                   setup_90
};
