#include <bits/stdc++.h>

using namespace std;


class MaliciousVehicleTable {

    public:
        bool contains (string vehicleID);

        void adddVehicle (string vehicleID);

    private:
        unordered_set<string> m_MaliciousVehicleTable;
};