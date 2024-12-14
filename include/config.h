#ifndef _CONFIG_H_
#define _CONFIG_H_

//---- GUI -------------------------------------------------------------------------------------------------------------------------------------------------------

#define GUI_REFRESH_RATE_FPS                60      // screen refresh rate in frames per second
#define GUI_POWER_UP_DELAY_MS               100     // delay from powerup to drawing first frame [ms]
#define GUI_POWERUP_COM_TIMEOUT_MS          3000    // if the load module doesn't respond after power up an error screen will be shown [ms]
#define GUI_LOAD_POWERUP_TEST_DURATION_MS   3000    // duration of load power-up test (for progress bar timing) [ms]

// default increments when enabling the setpoint setting mode
#define GUI_SETPOINT_START_INCREMENT_CC     1000
#define GUI_SETPOINT_START_INCREMENT_CV     1000
#define GUI_SETPOINT_START_INCREMENT_CR     1000
#define GUI_SETPOINT_START_INCREMENT_CP     10000

// minimum increments in the setpoint setting mode
#define GUI_SETPOINT_MIN_INCREMENT_CC       100
#define GUI_SETPOINT_MIN_INCREMENT_CV       100
#define GUI_SETPOINT_MIN_INCREMENT_CR       100
#define GUI_SETPOINT_MIN_INCREMENT_CP       1000

// maximum increments in the setpoint setting mode
#define GUI_SETPOINT_MAX_INCREMENT_CC       1000
#define GUI_SETPOINT_MAX_INCREMENT_CV       10000
#define GUI_SETPOINT_MAX_INCREMENT_CR       10000
#define GUI_SETPOINT_MAX_INCREMENT_CP       10000

#define GUI_AUTO_SETPOINT_DISABLE_TIME_MS   4000    // automatic setpoint setting mode disable time after no action [ms]
#define GUI_AUTO_SETPOINT_LATCH_TIME_MS     500     // pertist the setpoint setting mode by holding the SET button for this duration [ms]
#define GUI_BUG_ENABLE_HOLD_TIME_MS         1000    // key hold time required to enable the animated bug [ms]
#define GUI_FAULT_CLEAR_HOLD_TIME_MS        500     // key hold time required to clear a fault in the fault screen [ms]

//---- LOAD CONTROL ----------------------------------------------------------------------------------------------------------------------------------------------

#define LOAD_MIN_CC_LEVEL_MA    300     // minimum CC level allowed [mA]
#define LOAD_MAX_CC_LEVEL_MA    42000   // maximum CC level allowed [mA]
#define LOAD_MIN_CV_LEVEL_MV    0       // minimum CV level allowed [mV]
#define LOAD_MAX_CV_LEVEL_MV    70000   // maximum CV level allowed [mV]
#define LOAD_MIN_CR_LEVEL_MR    0       // minimum CP level allowed [mOhm]
#define LOAD_MAX_CR_LEVEL_MR    100000  // maximum CP level allowed [mOhm]
#define LOAD_MIN_CP_LEVEL_MW    10      // minimum CW level allowed [mW]
#define LOAD_MAX_CP_LEVEL_MW    400000  // maximum CW level allowed [mW]

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _CONFIG_H_ */