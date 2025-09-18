#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <EEPROM.h>
#include "Geometry.h"

class StorageManager {
public:
    // Constants
    static const int MAX_POINTS = 4;
    static const int EEPROM_SIZE = sizeof(Point) * MAX_POINTS;
    static const uint32_t SETUP_COMPLETE_FLAG = 0xDEADBEEF; // Flag to indicate setup completion

    // Constructor
    StorageManager();

    // Initialize EEPROM
    void begin();

    // Save points to EEPROM
    void savePoints(Point points[MAX_POINTS]);

    // Load points from EEPROM
    bool loadPoints(Point points[MAX_POINTS]);
};

#endif // STORAGE_MANAGER_H