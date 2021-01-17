#include "RecordTable.hpp"

using namespace std;

RecordTable::RecordTable(){

}

RecordTable::~RecordTable(){

}

bool RecordTable::hasRecord (string vehicleID){
    return  (m_RecordTable.find(vehicleID) != m_RecordTable.end());
}

void RecordTable::addRecord(string vehicleID){
    if(hasRecord(vehicleID)) return;
    RecordTableEntry entry( 0, 0, 0);
    m_RecordTable.insert({vehicleID, entry});
}

uint64_t RecordTable::getNoOfReceivedInerest (string vehicleID){
    if(!hasRecord(vehicleID)) {
        cout << "getNoOfReceivedInterest(): invalid vehicleID" << endl;
        return 0;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    return entry->getNoOfReceivedInterest();
}

uint64_t RecordTable::getNoOfSatisfiedInterest (string vehicleID) {
    if (!hasRecord(vehicleID) ) {
        cout << "getNoOfSatisfiedInterest(): invalid vehicleID" <<endl;
        return 0;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    return entry->getNoOfSatisfiedInterest();  
}

double_t RecordTable::getSatisfactionRatio (string vehicleID) {
    if (!hasRecord(vehicleID) ) {
        cout << "getSatisfactionRatio(): invalid vehicleID" <<endl;
        return 0;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    return entry->getSatisfactionRatio();
}

void RecordTable::setNoOfReceivedInterest(string vehicleID, uint64_t noOfReceviedInterest){
    if(!hasRecord(vehicleID)) {
        cout << "getNoOfReceivedInterest(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> setNoOfReceivedInterest (noOfReceviedInterest);
}

void RecordTable::setSatisfactionRatio (string vehicleID, double_t satisfactionRatio) {
    if (!hasRecord(vehicleID) ) {
        cout << "setSatisfactionRatio(): invalid vehicleID" <<endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> setSatisfactionRatio(satisfactionRatio);
}

void RecordTable::setNoOfSatisfiedInterest (string vehicleID, uint64_t satisfiedRatio) {
    if(!hasRecord(vehicleID)) {
        cout << "setNoOfSatisfiedInterest(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> setSatisfactionRatio (satisfiedRatio);
}

void RecordTable::incrementNoOfReceivedInterest(string vehicleID) {
    if(!hasRecord(vehicleID)) {
        cout << "incrementNoOfReceivedInterest(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> incrementNoOfReceivedInterest();  
}

void RecordTable::incrementNoOfSatisfiedInterest (string vehicleID) {
    if(!hasRecord(vehicleID)) {
        cout << "incrementNoOfSatisfiedInterest(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> incrementNoOfSatisfiedInterest();
}

void RecordTable::decrementNoOfReceivedInterest (string vehicleID) {
    if(!hasRecord(vehicleID)) {
        cout << "decrementNoOfReceivedInterest(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> decrementNoOfReceivedInterest();
}

void RecordTable::decrementNoOfSatisfiedInterest (string vehicleID) {
    if(!hasRecord(vehicleID)) {
        cout << "decrementNoOfSatisfiedInterest(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);

    entry -> decrementNoOfSatisfiedInterest();
}

void RecordTable::updateSatisfactionRatio (string vehicleID) {
    if(!hasRecord(vehicleID)) {
        cout << "updateSatisfactionRatio(): invalid vehicleID" << endl;
        return;
    }

    RecordTableEntry *entry;

    entry = &(m_RecordTable.find(vehicleID)->second);  

    double_t new_satisfaction_ratio = (entry->getNoOfSatisfiedInterest()*1.0)/entry->getNoOfReceivedInterest();

    entry->setSatisfactionRatio(new_satisfaction_ratio);   
}