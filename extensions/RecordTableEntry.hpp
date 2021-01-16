#include <bits/stdc++.h>

using namespace std;

class RecordTableEntry {
    public:
        // Default ctor 
        RecordTableEntry(uint64_t noOfReceivedInterest, uint64_t noOfSatisfiedInterest, double_t SatisfactionRatio);

        //Default dtor
        ~RecordTableEntry();

        uint64_t getNoOfReceivedInterest();

        uint64_t getNoOfSatisfiedInterest();

        double_t getSatisfactionRatio();

        void setNoOfReceivedInterest (uint64_t noOfReceivedInterest);

        void setNoOfSatisfiedInterest (uint64_t noOfSatisfiedInterest);

        void setSatisfactionRatio (double_t satisfactionRatio);

        void incrementNoOfReceivedInterest(); // Increament by 1

        void decrementNoOfReceivedInterest(); // decreament by 1

        void incrementNoOfSatisfiedInterest(); //Increment by 1

        void decrementNoOfSatisfiedInterest(); // Decrement by 1
    private:
        uint64_t m_noOfReceivedInterest; //Count number of received interest packet for a vehicle 
        uint64_t m_noOfSatisfiedInterest; // Count number of satisfied interest for a vehicle
        double_t m_SatisfactionRatio;     // Satisfaction ratio of vehicle
        //TO-DO: Add expiry timer 
};