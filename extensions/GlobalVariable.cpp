#include "GlobalVariable.hpp"


// Initialised static variables
int ns3::GlobalVariable::simulationEnd;
double_t ns3::GlobalVariable::interestCntTh;
double_t ns3::GlobalVariable::satisfactionRatioTh;
std::string ns3::GlobalVariable::primaryTimer;
std::string ns3::GlobalVariable::secondaryTimer;


// Define getter, setter method

std::string ns3::GlobalVariable::getSecondaryTimer(void){
    return secondaryTimer;
}

void ns3::GlobalVariable::setSecondaryTimer(std::string time_str){
    secondaryTimer = time_str;
}

void ns3::GlobalVariable::setPrimaryTimer(std::string time_str){
    primaryTimer = time_str;
}

std::string ns3::GlobalVariable::getPrimaryTimer(){
    return primaryTimer;
}

double_t ns3::GlobalVariable::getSatisfactionRatioTh(void){
    return satisfactionRatioTh;
}

void ns3::GlobalVariable::setSatisfactionRatioTh(double_t th){
    satisfactionRatioTh = th;
}

double_t ns3::GlobalVariable::getInterestCntTh(void){
    return interestCntTh;
}

void ns3::GlobalVariable::setInterestCntTh(double_t th){
    interestCntTh = th;
}

void ns3::GlobalVariable::setSimulationEnd(int time) {
    simulationEnd = time;
}

int ns3::GlobalVariable::getSimulationEnd(void){
    return simulationEnd;
}

