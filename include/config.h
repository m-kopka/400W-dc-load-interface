#ifndef _CONFIG_H_
#define _CONFIG_H_

//---- LOAD CONTROL ----------------------------------------------------------------------------------------------------------------------------------------------

#define LOAD_MIN_CC_LEVEL_MA    300     // minimum CC level allowed [mA]
#define LOAD_MAX_CC_LEVEL_MA    42000   // maximum CC level allowed [mA]
#define LOAD_MIN_CV_LEVEL_MV    0       // minimum CV level allowed [mV]
#define LOAD_MAX_CV_LEVEL_MV    70000   // maximum CV level allowed [mV]
#define LOAD_MIN_CR_LEVEL_MR    0       // minimum CP level allowed [mOhm]
#define LOAD_MAX_CR_LEVEL_MR    100000  // maximum CP level allowed [mOhm]
#define LOAD_MIN_CP_LEVEL_MW    10      // minimum CW level allowed [mW]
#define LOAD_MAX_CP_LEVEL_MW    400000  // maximum CW level allowed [mW]

#define LOAD_START_CC_LEVEL_MA      1000    // CC level on startup [mA]
#define LOAD_START_CV_LEVEL_MV      5000    // CV level on startup [mV]
#define LOAD_START_CR_LEVEL_MR      10000   // CR level on startup [mOhm]
#define LOAD_START_CP_LEVEL_MW      50000   // CP level on startup [mW]

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* _CONFIG_H_ */