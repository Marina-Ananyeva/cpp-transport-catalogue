#include "stat_reader.h"

namespace catalogue {
namespace stat {
std::istream& operator>>(std::istream& is, QueryStat& q) {
    std::string operation_code;
    is >> operation_code;

    if (operation_code == "Stop") {
        std::string text;
        getline(is, text);
        text.erase(0, 1);
        q.query_stat_.push_back({QueryTypeStat::StatStop, text});
    }

    if (operation_code == "Bus") {
        std::string text;
        getline(is, text);
        text.erase(0, 1);
        q.query_stat_.push_back({QueryTypeStat::StatBus, text});
    }

    return is;
}

void ReadQueryStat(std::istream& is, QueryStat& q) {
    int query_count = input::ReadLineWithNumber(is);
    for (int i = 0; i < query_count; ++i) {
        is >> q;
    }
}

StopsForBusStat::StopsForBusStat(const std::tuple<std::string_view, int, int, uint64_t, double> stops_for_bus) : stops_for_bus_(stops_for_bus) {
}

StopsForBusStat::StopsForBusStat(const StopsForBusStat& other) {
    this->stops_for_bus_ = other.stops_for_bus_;
}

StopsForBusStat& StopsForBusStat::operator=(const StopsForBusStat& rhs) {
    if (&rhs != this) {
        this->stops_for_bus_ = rhs.stops_for_bus_;
    }
    return *this;
}

bool StopsForBusStat::Is_Empty() const {
    return std::get<0>(stops_for_bus_).empty();
}

BusesForStopStat::BusesForStopStat(const std::map<std::string_view, std::set<const head::TransportCatalogue::Bus*>> buses_for_stop)
: buses_for_stop_(buses_for_stop) {
}

BusesForStopStat::BusesForStopStat(const BusesForStopStat &other) {
    this->buses_for_stop_ = other.buses_for_stop_;
}

bool BusesForStopStat::Is_Empty() const {
    return buses_for_stop_.empty();
}

BusesForStopStat& BusesForStopStat::operator=(const BusesForStopStat &rhs) {
    if (&rhs != this) {
        this->buses_for_stop_ = rhs.buses_for_stop_;
    }
    return *this;
}

StopsForBusStat GetStopsForBus(const head::TransportCatalogue& tc, const std::string& stat_bus) {
    StopsForBusStat r;
    if (!stat_bus.empty()) {
        std::string_view str(stat_bus);
        const head::TransportCatalogue::Bus* bus_stat_ptr = tc.FindBus(str);
        if (!bus_stat_ptr) {
            r.stops_for_bus_ = std::make_tuple(str, 0, 0, 0, 0.0);
        } else {
            int r_size = tc.GetBusInfoVec(bus_stat_ptr).size();
            int u_size = tc.GetBusInfoSet(bus_stat_ptr).size();;
            double l_route_geo = tc.ComputeGeoDistance(str, r_size);
            uint64_t l_route_map = tc.ComputeMapDistance(str, r_size);;
            double c_curvature = l_route_map / l_route_geo;

            r.stops_for_bus_ = std::make_tuple(str, r_size, u_size, l_route_map, c_curvature);
        }
    }

    return r;
}

BusesForStopStat GetBusesForStop(const head::TransportCatalogue& tc, const std::string& stat_stop) {
    BusesForStopStat r;
    if (!stat_stop.empty()) {
        std::string_view str(stat_stop);
        const head::TransportCatalogue::Stop* stop_stat_ptr = tc.FindStop(str);
        if (!stop_stat_ptr) { //проверяем есть ли такая остановка
            r.buses_for_stop_.insert({str, {}});//передаем пустой set
            return r;
        }
        else {
            std::unordered_set<const head::TransportCatalogue::Bus*> stop_info = tc.GetStopInfoSet(stop_stat_ptr);
            std::set<const head::TransportCatalogue::Bus*> buses;
            if (stop_info.empty()) {//проверяем есть ли у остановки маршруты
                buses.insert(nullptr);//передаем buses с nullptr
                r.buses_for_stop_.insert({str, buses});
            } else {
                buses.insert(stop_info.begin(), stop_info.end());
                r.buses_for_stop_.insert({str, buses});
            }
        }
    }

    return r;
}

GetInfoStat GetStatInfo (const head::TransportCatalogue& tc, QueryStat& q) {
    GetInfoStat r;
    std::tuple<std::string_view, int, int, uint64_t, double> tmp1;
    std::map<std::string_view, std::set<const head::TransportCatalogue::Bus*>> tmp2;
    if (!q.query_stat_.empty()) {
        for (const auto& query : q.query_stat_) {
            switch (query.first) {
                case QueryTypeStat::StatStop:
                    r.info_.push_back(std::make_pair(StopsForBusStat(tmp1), GetBusesForStop(tc, query.second)));
                    break;
                case QueryTypeStat::StatBus:
                    r.info_.push_back(std::make_pair(GetStopsForBus(tc, query.second), BusesForStopStat(tmp2)));
                    break;
            }
        }
    }
    return r;
}

GetInfoStat ExecuteStatRequests(head::TransportCatalogue& tc, QueryStat& q, std::istream& is, std::ostream& os) {
    GetInfoStat result;
    {
        using namespace std::literals;
        LOG_DURATION("ReadQueryStat"s);
        ReadQueryStat(is, q);
    }
    {
        using namespace std::literals;
        LOG_DURATION("GetInfo"s);
        result = GetStatInfo(tc, q);
    }

    return result;
}

std::ostream& operator<<(std::ostream& os, const GetInfoStat& r) {
    for (const auto& info : r.info_) {
        if (!info.first.Is_Empty()) {
            os << info.first;
        }
        if (!info.second.Is_Empty()) {
            os << info.second;
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const StopsForBusStat& r) {
        using namespace std::literals;
        os << "Bus "s << std::get<0>(r.stops_for_bus_) << ": "s;
        if (std::get<1>(r.stops_for_bus_) == 0) {
            os << "not found"s << std::endl;
        } else {
            os << std::get<1>(r.stops_for_bus_) << " stops on route, "s << std::get<2>(r.stops_for_bus_) 
            << " unique stops, "s << std::get<3>(r.stops_for_bus_) << " route length, " << std::setprecision(6) 
            << std::get<4>(r.stops_for_bus_) << " curvature"s << std::endl;
        }

    return os;
}

std::ostream &operator<<(std::ostream &os, const BusesForStopStat &r) {
    using namespace std::literals;

    for (const auto& [stop, buses] : r.buses_for_stop_) {
        os << "Stop "s << stop << ": "s;

        if (buses.empty() || *buses.begin() == nullptr) {
            if (buses.empty()) {
                os << "not found"sv << std::endl;
                break;
            } else {
                os << "no buses"sv << std::endl;
                break;
            }
        }

        os << "buses "s;

        for (const auto& bus : buses) {
            os << bus->GetBus() << " ";
        }
        os << std::endl;
    }

    return os;
}
}//namespace stat
}//namespace catalogue