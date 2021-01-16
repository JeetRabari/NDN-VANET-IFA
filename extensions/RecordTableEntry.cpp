#include <bits/stdc++.h>
#include "RecordTableEntry.hpp"

using namespace std;

/*
Default constructor to create RecordTableEntry
*/
RecordTableEntry::RecordTableEntry(uint64_t noOfReceivedInterest, uint64_t noOfSatisfiedInterest, double_t SatisfactionRatio){
    this -> m_noOfReceivedInterest = noOfReceivedInterest;
    this -> m_noOfSatisfiedInterest = noOfSatisfiedInterest;
    this -> m_SatisfactionRatio = SatisfactionRatio;
}

/*
Default dtor
*/
RecordTableEntry::~RecordTableEntry(){

}

/*
get number of recevied interest for a vehicle
*/
uint64_t RecordTableEntry::getNoOfReceivedInterest(){
    return this -> m_noOfReceivedInterest;
}


/*
get number of satisfied interest for a vehicle
*/
uint64_t RecordTableEntry::getNoOfSatisfiedInterest(){
    return this -> m_noOfSatisfiedInterest;
}


/*
get satisfaction ratio of a vehicle
*/
double_t RecordTableEntry::getSatisfactionRatio(){
    return this -> m_SatisfactionRatio;
}


/*
set number of recevied interest for a vehicle
*/
void RecordTableEntry::setNoOfReceivedInterest (uint64_t noOfReceivedInterest){
    this -> m_noOfReceivedInterest = noOfReceivedInterest;
}


/*
set number of satisfied interest for a vehicle
*/
void RecordTableEntry::setNoOfSatisfiedInterest (uint64_t noOfSatisfiedInterest){
    this -> m_noOfSatisfiedInterest = noOfSatisfiedInterest;
}


/*
set satisfaction ratio of a vehicle
*/
void RecordTableEntry::setSatisfactionRatio (double_t satisfactionRatio){
    this -> m_SatisfactionRatio = satisfactionRatio;
}

/*
Increment number of recevied interest by 1
usefull to update values when interest is received
*/
void RecordTableEntry::incrementNoOfReceivedInterest(){
    this -> m_noOfReceivedInterest ++;
}

/*
Increment number of satisfied interest by 1
usefull to update values when interest is received
*/
void RecordTableEntry::incrementNoOfSatisfiedInterest(){
    this -> m_noOfSatisfiedInterest ++;
}


/*
Decrement number of recevied interest by 1.

If value is 0 then decrement is not performed.
*/
void RecordTableEntry::decrementNoOfReceivedInterest(){
    if(this -> m_noOfReceivedInterest <= 0){
        this -> m_noOfReceivedInterest = 0; //probably of no need 
        return;
    }

    this -> m_noOfReceivedInterest --;
}

/*
Decrement number of satisfied interest by 1.

If value is 0 then decrement is not performed.
*/
void RecordTableEntry::decrementNoOfSatisfiedInterest(){
    if( this -> m_noOfSatisfiedInterest <= 0 ) {
        this -> m_noOfSatisfiedInterest = 0;
        return;
    }

    this -> m_noOfSatisfiedInterest --;
}