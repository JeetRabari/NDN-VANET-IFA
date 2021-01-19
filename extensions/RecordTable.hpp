#include <bits/stdc++.h>
#include "RecordTableEntry.hpp"

using namespace std;

class RecordTable {
    
    public:
        RecordTable();

        ~RecordTable();

        void addRecord(string vehicleID);

        bool hasRecord(string vehicleID);

        uint64_t getNoOfReceivedInerest (string vehicleID);

        uint64_t getNoOfSatisfiedInterest (string vehicleID);

        double_t getSatisfactionRatio (string vehicleID);

        void setNoOfReceivedInterest(string vehicleID, uint64_t noOfReceivedInterest);

        void setNoOfSatisfiedInterest (string vehicleID, uint64_t noOfSatisfiedInterest);

        void setSatisfactionRatio (string vehicleID, double_t satisfactionRatio);

        void incrementNoOfReceivedInterest(string vehicleID);

        void incrementNoOfSatisfiedInterest(string vehicleID);

        void decrementNoOfReceivedInterest(string vehicleID);

        void decrementNoOfSatisfiedInterest(string vehicleID);

        void updateSatisfactionRatio(string vehicleID);

        void printTable();

        uint64_t getTotalInterestCnt(void);

        vector<string> keySet(void);

    private:
        unordered_map<string, RecordTableEntry> m_RecordTable; 
};