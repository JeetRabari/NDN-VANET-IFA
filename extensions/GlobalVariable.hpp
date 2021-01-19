#include <bits/stdc++.h>
/*
This class is to declared global variable that can be accessed by 
Application anywhere.

To add new value, declare a private static variable and its getter, setter method.
*** DO NOT FORGET to initialised newly declared static variable in .cpp file ****
*/
namespace ns3{
    class GlobalVariable {
        public:
            static int getSimulationEnd(void);
            static void setSimulationEnd(int time);
            static double_t getInterestCntTh();
            static void setInterestCntTh(double_t th);
            static double_t getSatisfactionRatioTh(void);
            static void setSatisfactionRatioTh(double_t th);
            static std::string getPrimaryTimer(void);
            static void setPrimaryTimer(std::string time_str);
            static void setSecondaryTImer(std::string time_str);
            static std::string getSecondaryTimer(void);
        private:
            static int simulationEnd;
            static double_t interestCntTh;
            static double_t satisfactionRatioTh;
            static std::string primaryTimer;
            static std::string secondaryTimer;
    };
}