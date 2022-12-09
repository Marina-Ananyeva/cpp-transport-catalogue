#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace catalogue {
namespace head {
TransportCatalogue::StopsForBusStat::StopsForBusStat(std::tuple<std::string_view, int, int, uint64_t, double> stops_for_bus) 
: stops_for_bus_(stops_for_bus) {
}

TransportCatalogue::StopsForBusStat::StopsForBusStat(const TransportCatalogue::StopsForBusStat& other) {
    this->stops_for_bus_ = other.stops_for_bus_;
}

TransportCatalogue::StopsForBusStat& TransportCatalogue::StopsForBusStat::operator=(const TransportCatalogue::StopsForBusStat& rhs) {
    if (&rhs != this) {
        this->stops_for_bus_ = rhs.stops_for_bus_;
    }
    return *this;
}

bool TransportCatalogue::StopsForBusStat::Is_Empty() const {
    return std::get<0>(stops_for_bus_).empty();
}

TransportCatalogue::BusesForStopStat::BusesForStopStat(std::map<std::string_view, std::set<std::string_view>> buses_for_stop)
: buses_for_stop_(buses_for_stop) {
}

TransportCatalogue::BusesForStopStat::BusesForStopStat(const BusesForStopStat &other) {
    this->buses_for_stop_ = other.buses_for_stop_;
}

bool TransportCatalogue::BusesForStopStat::Is_Empty() const {
    return buses_for_stop_.empty();
}

TransportCatalogue::BusesForStopStat& TransportCatalogue::BusesForStopStat::operator=(const TransportCatalogue::BusesForStopStat &rhs) {
    if (&rhs != this) {
        this->buses_for_stop_ = rhs.buses_for_stop_;
    }
    return *this;
}

void TransportCatalogue::AddStop(const std::vector<std::string>& text_stops) {
    Stop new_stop;
    if (!text_stops.empty()) {
        for (std::string_view str : std::move(text_stops)) {
            new_stop = ParseQueryStop(str);
            stops_.push_back(new_stop);
        }
    }

    for (auto& stop : stops_) {
        stopname_to_stop_.insert({stop.GetStop(), &stop});
    }
}

void TransportCatalogue::AddDistance(const std::vector<std::string>& text_stops) {
    std::vector<std::pair<std::pair<Stop*, Stop*>, int>> stops_distance;
    if (!text_stops.empty()) {
        for (std::string_view str : std::move(text_stops)) {
            stops_distance = ParseQueryDistance(str);
            for (const auto& info : stops_distance) {
                stops_distance_.insert({info.first, info.second});
            }
        }
    }
}

void TransportCatalogue::AddBus(const std::vector<std::string>& text_buses) {
    if (!text_buses.empty()) {
        for (std::string_view str : std::move(text_buses)) {
            Bus new_bus(ParseQueryBus(str));
            buses_.push_back(new_bus);
            MakeRoute(buses_.back(), str);
        }
    }

    for (auto& bus : buses_) {
        busname_to_bus_.insert({bus.GetBus(), &bus});
    }
}

void TransportCatalogue::CompleteCatalogue() {
    for (auto& bus : buses_) {
        bus_to_stops_[&bus].insert(bus.bus_and_stops_.begin(), bus.bus_and_stops_.end());

        for (auto stop : bus.bus_and_stops_) {
            stop_to_buses_[stop].insert(&bus);
        }
    }
}

TransportCatalogue::GetInfoStat TransportCatalogue::GetInfo (stat::QueryStat& q) {
    GetInfoStat r;
    std::tuple<std::string_view, int, int, uint64_t, double> tmp1;
    std::map<std::string_view, std::set<std::string_view>> tmp2;
    if (!q.query_stat_.empty()) {
        for (const auto& query : q.query_stat_) {
            switch (query.first) {
                case stat::QueryTypeStat::StatStop:
                    r.info_.push_back(std::make_pair(StopsForBusStat(tmp1), GetBusesForStop(query.second)));
                    break;
                case stat::QueryTypeStat::StatBus:
                    r.info_.push_back(std::make_pair(GetStopsForBus(query.second), BusesForStopStat(tmp2)));
                    break;
            }
        }
    }
    return r;
}

TransportCatalogue::Stop::Stop(std::string_view stop, double latitude, double longitude) : stop_(stop), geo_({latitude, longitude}) {
}

TransportCatalogue::Stop::Stop(const TransportCatalogue::Stop& other) {
    this->stop_ = other.stop_;
    this->geo_ = other.geo_;
}

TransportCatalogue::Stop& TransportCatalogue::Stop::operator=(const TransportCatalogue::Stop& rhs) {
    if (&rhs != this) {
        this->stop_ = rhs.stop_;
        this->geo_ = rhs.geo_;
    }
    return *this;
}

std::string_view TransportCatalogue::Stop::GetStop() const {
    return stop_;
}

std::pair<double, double> TransportCatalogue::Stop::GetGeo() const {
    return geo_;
}

TransportCatalogue::Bus::Bus (std::string_view bus) : bus_(bus) {
}

TransportCatalogue::Bus::Bus(std::string_view bus, bool is_ring_bus) : bus_(bus), is_ring_bus_(is_ring_bus) {
}

TransportCatalogue::Bus::Bus(const TransportCatalogue::Bus& other) {
    this->bus_ = other.bus_;
    this->is_ring_bus_ = other.is_ring_bus_;
    this->bus_and_stops_ = other.bus_and_stops_;
}

TransportCatalogue::Bus& TransportCatalogue::Bus::operator=(const TransportCatalogue::Bus& rhs) {
    if (&rhs != this) {
        this->bus_ = rhs.bus_;
        this->is_ring_bus_ = rhs.is_ring_bus_;
        this->bus_and_stops_ = rhs.bus_and_stops_;
    }
    return *this;
}

std::string_view TransportCatalogue::Bus::GetBus() const {
    return bus_;
}

bool TransportCatalogue::Bus::IsRingBus() const {
    return is_ring_bus_;
}

TransportCatalogue::Stop TransportCatalogue::ParseQueryStop(std::string_view str) {
    std::string_view stop;
    double latitude = 0.0;
    double longitude = 0.0;

    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));
    using namespace std::literals;
    auto pos = str.find(":"sv);
    stop = str.substr(0, pos);

    str.remove_prefix(std::min(str.find_first_not_of(' ', pos + 1), str.size()));
    pos = str.find(","sv);
    std::string str_latitude(str.substr(0, pos + 1));
    latitude = std::stod(str_latitude);

    str.remove_prefix(std::min(str.find_first_not_of(' ', pos + 1), str.size()));
    pos = str.find_last_not_of(' ');
    std::string str_longitude(str.substr(0, pos + 1));
    longitude = std::stod(str_longitude);

    return Stop(stop, latitude, longitude);
}

std::vector<std::pair<std::pair<TransportCatalogue::Stop*, TransportCatalogue::Stop*>, int>> TransportCatalogue::ParseQueryDistance(std::string_view str) {
    std::vector<std::pair<std::pair<Stop*, Stop*>, int>> stops_distance;

    using namespace std::literals;

    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));
    auto pos = str.find(":"sv);
    std::string_view stop_start = str.substr(0, pos);//выделяем название остановки
    Stop* stop_start_ptr = GetStopPtr(stop_start);//получаем указатель на начальную остановку

    pos = str.find(","sv);//ищем первую ,
    pos = str.find(","sv, pos + 1);//ищем вторую ,
    str.remove_prefix(std::min(str.find_first_not_of(' ', pos + 1), str.size()));//переводим начало строки на первый символ не пробел после второй ,

    while (pos != str.npos) {//пока не дойдем до конца строки
        pos = str.find(","sv);//находим ,
        std::string_view stop_info = str.substr(0, pos);//обрезаем название остановки до ,
        stop_info = stop_info.substr(0, stop_info.find_last_not_of(' ') + 1);//убираем пробелы с конца = строка с расстоянием и конечной остановкой
        str.remove_prefix(std::min(pos + 2, str.size()));//обрезаем строку минус , и пробел

        auto pos1 = stop_info.find("m"sv);//находим букву m
        std::string str_distance(stop_info.substr(0, pos1));//обрезаем до нее = расстояние
        int distance = std::stoi(str_distance);

        pos1 = stop_info.find(" to "sv);//находим " to "
        std::string_view stop_finish = stop_info.substr(pos1 + 4, stop_info.size() - pos1 - 4);//обрезаем от " to " до конца строки = конечная остановка
        Stop* stop_finish_ptr = GetStopPtr(stop_finish);//получаем указатель на конечную остановку
        
        stops_distance.push_back(std::make_pair(std::make_pair(stop_start_ptr, stop_finish_ptr), distance));//добавляем данные в вектор
    }

    return stops_distance;
}

TransportCatalogue::Bus TransportCatalogue::ParseQueryBus(std::string_view str) {
    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));//переводим начало строки на первый символ не пробел
    using namespace std::literals;
    auto pos = str.find(":"sv);//находим конец названия маршрута (включая пробелы)
    std::string_view bus = str.substr(0, pos);//оставляем строку от начала до элемента, предшествующего :
    bus = bus.substr(0, bus.find_last_not_of(' ') + 1);//оставляем строку от начала до последнего элемента, не пробела (удаляем пробелы в конце)

    bool is_ring_bus = false;//проверяем кольцевой маршрут или нет
    size_t pos_ring = str.find("-"sv);
    if (pos_ring == std::string::npos) {
        is_ring_bus = true;
    }

    return Bus(bus, is_ring_bus);
}

void TransportCatalogue::MakeRoute(Bus& bus, std::string_view str) {
    std::string_view stop_route;
    using namespace std::literals;

    size_t pos = str.find(":"sv);
    str.remove_prefix(std::min(str.find_first_not_of(' ', pos + 1), str.size()));//переводим начало строки на первый символ не пробел после :

    while (pos != str.npos) {//пока не дойдем до конца строки
        pos = str.find("-"sv);//находим -
        if (pos == std::string::npos) {
            pos = str.find(">"sv);//или >
        }
        stop_route = str.substr(0, pos);//обрезаем название остановки до - или > (включая пробелы)
        stop_route = stop_route.substr(0, stop_route.find_last_not_of(' ') + 1);//убираем пробелы с конца
        str.remove_prefix(str.find_first_not_of(' ', pos + 1));//передвигаем начало на первый символ не пробел после - или >
        Stop* stop_ptr = GetStopPtr(stop_route);
        bus.bus_and_stops_.push_back(stop_ptr);
    }

    if (bus.IsRingBus() == false) {//добавляем остановки до кольцевого маршрута
        std::vector<Stop*> copy_vec(bus.bus_and_stops_.size() - 1);
        std::reverse_copy(bus.bus_and_stops_.begin(), prev(bus.bus_and_stops_.end()), copy_vec.begin());
        bus.bus_and_stops_.reserve(bus.bus_and_stops_.size() * 2 - 1);
        bus.bus_and_stops_.insert(bus.bus_and_stops_.end(), copy_vec.begin(), copy_vec.end());
    }
}

const TransportCatalogue::Stop& TransportCatalogue::GetStopIndex(const std::string_view stop) {
    auto it_stop = stopname_to_stop_.find(stop);
    using namespace std::literals;
    if (it_stop == stopname_to_stop_.end()) {
        throw std::invalid_argument("Stop not found"s);
    }
    return *stopname_to_stop_[stop];
}

TransportCatalogue::Stop* TransportCatalogue::GetStopPtr(const std::string_view stop) {
    auto it_stop = stopname_to_stop_.find(stop);
    using namespace std::literals;
    if (it_stop == stopname_to_stop_.end()) {
        throw std::invalid_argument("Stop not found"s);
    }
    return stopname_to_stop_[stop];
}

const TransportCatalogue::Bus& TransportCatalogue::GetBusIndex(const std::string_view bus) {
    auto it_bus = busname_to_bus_.find(bus);
    using namespace std::literals;
    if (it_bus == busname_to_bus_.end()) {
        throw std::invalid_argument("Bus not found"s);
    }
    return *busname_to_bus_[bus];
}

TransportCatalogue::Bus* TransportCatalogue::GetBusPtr(const std::string_view bus) {
    auto it_bus = busname_to_bus_.find(bus);
    using namespace std::literals;
    if (it_bus == busname_to_bus_.end()) {
        throw std::invalid_argument("Bus not found"s);
    }
    return busname_to_bus_[bus];
}

TransportCatalogue::StopsForBusStat TransportCatalogue::GetStopsForBus(const std::string& stat_bus) {
    StopsForBusStat r;
    std::string_view str(stat_bus);
    if (!stat_bus.empty()) {
        Bus bus_stat;
        try {
            bus_stat = GetBusIndex(str);
        } catch (const std::invalid_argument &e) {
            r.stops_for_bus_ = std::make_tuple(str, 0, 0, 0, 0.0);
            return r;
        }

        int r_size = bus_stat.bus_and_stops_.size();
        int u_size = bus_to_stops_[GetBusPtr(str)].size();
        double l_route_geo = 0.0;
        uint64_t l_route_map = 0;

        for (auto i = 0; i < r_size - 1; ++i) {
            Stop from = *(bus_stat.bus_and_stops_[i]);
            Stop to = *(bus_stat.bus_and_stops_[i + 1]);
            l_route_geo += geo::ComputeDistance(geo::Coordinates(from.GetGeo()), geo::Coordinates(to.GetGeo()));
        }

        for (auto i = 0; i < r_size - 1; ++i) {
            Stop* from = bus_stat.bus_and_stops_[i];
            Stop* to = bus_stat.bus_and_stops_[i + 1];
            
            auto it = stops_distance_.find(std::make_pair(from, to));
            if (it == stops_distance_.end()) {
                it = stops_distance_.find(std::make_pair(to, from));
            }
            uint64_t distance = it -> second;
            l_route_map += distance;
        }

        double c_curvature = l_route_map / l_route_geo;

        r.stops_for_bus_ = std::make_tuple(str, r_size, u_size, l_route_map, c_curvature);
    }

    return r;
}

TransportCatalogue::BusesForStopStat TransportCatalogue::GetBusesForStop(const std::string& stat_stop) {
    BusesForStopStat r;
    std::string_view str(stat_stop);
    if (!stat_stop.empty()) {
        Stop* stop_stat_ptr;
        std::set <std::string_view> buses;
        try {
            stop_stat_ptr = GetStopPtr(str);
        } catch (const std::invalid_argument &e) {
            using namespace std::literals;
            buses.insert("not found"sv);//проверяем есть ли такая остановка
            r.buses_for_stop_.insert({str, buses});
            return r;
            }

            auto it = stop_to_buses_.find(stop_stat_ptr);
            if (it == stop_to_buses_.end()) {//проверяем есть ли у остановки маршруты
                using namespace std::literals;
                buses.insert("no buses"sv);
                r. buses_for_stop_.insert({str, buses});
            } else {
                for (const auto& bus : stop_to_buses_[stop_stat_ptr]) {//итерируем по списку маршрутов для этой остановки
                    buses.insert(bus->GetBus());//добавляем названия маршрутов в set
                }
                r.buses_for_stop_.insert({str, buses});
            }
    }

    return r;
}
}

namespace detail {
std::ostream& operator<<(std::ostream& os, const head::TransportCatalogue::GetInfoStat& r) {
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

std::ostream& operator<<(std::ostream& os, const head::TransportCatalogue::StopsForBusStat& r) {
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

std::ostream &operator<<(std::ostream &os, const head::TransportCatalogue::BusesForStopStat &r) {
    using namespace std::literals;

    for (const auto& [stop, buses] : r.buses_for_stop_) {
        os << "Stop "s << stop << ": "s;

        if (*buses.begin() == "not found"sv || *buses.begin() == "no buses"sv) {
            os << *buses.begin() << std::endl;
            break;
        }

        os << "buses "s;

        for (const auto& bus : buses) {
            os << bus << " ";
        }
        os << std::endl;
    }

    return os;
}
}
}