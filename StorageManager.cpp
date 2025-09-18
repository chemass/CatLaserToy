#include "StorageManager.h"

// Define static const members
const int StorageManager::MAX_POINTS;
const int StorageManager::EEPROM_SIZE;
const uint32_t StorageManager::SETUP_COMPLETE_FLAG;

StorageManager::StorageManager() {
    // Constructor - nothing to initialize
}

void StorageManager::begin() {
    EEPROM.begin(EEPROM_SIZE);
}

void StorageManager::savePoints(Point points[MAX_POINTS]) {
    EEPROM.put(0, SETUP_COMPLETE_FLAG); // Store marker
    for (int i = 0; i < MAX_POINTS; i++) {
        EEPROM.put(sizeof(SETUP_COMPLETE_FLAG) + i * sizeof(Point), points[i]);
    }
    EEPROM.commit();
}

bool StorageManager::loadPoints(Point points[MAX_POINTS]) {
    uint32_t marker;
    EEPROM.get(0, marker);
    if (marker != SETUP_COMPLETE_FLAG) {
        return false; // Data not initialized
    }
    for (int i = 0; i < MAX_POINTS; i++) {
        EEPROM.get(sizeof(SETUP_COMPLETE_FLAG) + i * sizeof(Point), points[i]);
    }
    return true;
}