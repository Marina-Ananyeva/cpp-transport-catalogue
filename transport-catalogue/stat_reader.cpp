#include "stat_reader.h"

namespace catalogue {
namespace stat {
std::istream& operator>>(std::istream& is, QueryStat& q) {
    std::string operation_code;
    is >> operation_code;

    if (operation_code == "Stop") {
        //Stop Samara
        std::string text;
        getline(is, text);
        text.erase(0, 1);
        q.query_stat_.push_back({QueryTypeStat::StatStop, text});
    }

    if (operation_code == "Bus") {
        //Bus 256
        std::string text;
        getline(is, text);
        text.erase(0, 1);
        q.query_stat_.push_back({QueryTypeStat::StatBus, text});
    }

    return is;
}

void ReadQueryStat(QueryStat& q) {
    int query_count = input::ReadLineWithNumber();
    for (int i = 0; i < query_count; ++i) {
        std::cin >> q;
    }
}
}
}