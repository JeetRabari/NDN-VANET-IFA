#include "ns3/ndnSIM/helper/ndn-strategy-choice-helper.hpp"

namespace ns3 {
namespace ndn {
    class MyStretegyChoiceHelper : public StrategyChoiceHelper{
        public:
            template<class Strategy>
            static void
            InstallAll(const Name& namePrefix, const Name& strategy);    
    };

    template<class Strategy>
    inline void
    MyStretegyChoiceHelper::InstallAll(const Name& namePrefix, const Name& strategy)
    {
        NodeContainer c  = NodeContainer::GetGlobal();
        for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
            Install(*i, namePrefix, strategy.toUri()+"/v"+std::to_string((*i)->GetId()));
        }
    }
}
}