#include "input_reader.h"

namespace catalogue {
namespace input {
std::istream& operator>>(std::istream& is, QueryInput& q) {
    std::string operation_code;
    is >> operation_code;

    if (operation_code == "Stop") {
        q.type = QueryInputType::NewStop;
        std::string text;
        getline(is, text);
        q.text_stops_.push_back(text);
    }

    if (operation_code == "Bus") {
        q.type = QueryInputType::NewBus;
        std::string text;
        getline(is, text);
        q.text_buses_.push_back(text);
    }

    return is;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    return result;
}

void ReadQueryInput(QueryInput& q) {
    int query_count = ReadLineWithNumber();
    for (int i = 0; i < query_count; ++i) {
        std::cin >> q;
    }
}
}
}
