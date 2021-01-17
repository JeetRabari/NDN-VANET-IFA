#include "MaliciousVehicleTable.hpp"

void MaliciousVehicleTable::adddVehicle (string vehicleID){
    m_MaliciousVehicleTable.insert(vehicleID);
}

bool MaliciousVehicleTable::contains (string vehicleID) {
    return (m_MaliciousVehicleTable.find(vehicleID) != m_MaliciousVehicleTable.end());
}