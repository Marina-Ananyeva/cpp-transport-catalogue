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

int ReadLineWithNumber(std::istream& is) {
    int result;
    is >> result;
    return result;
}

void ReadQueryInput(std::istream& is, QueryInput& q) {
    int query_count = ReadLineWithNumber(is);
    for (int i = 0; i < query_count; ++i) {
        is >> q;
    }
}

std::tuple<std::string_view, double, double> ParseQueryStop(std::string_view str) {
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

    return std::make_tuple(stop, latitude, longitude);
}

std::vector<std::pair<std::pair<const std::string_view, const std::string_view>, int>> ParseQueryDistance(std::string_view str) {
    std::vector<std::pair<std::pair<const std::string_view, const std::string_view>, int>> stops_distance;

    using namespace std::literals;
    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));
    auto pos = str.find(":"sv);
    std::string_view stop_start = str.substr(0, pos);//выделяем название остановки

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
        
        stops_distance.push_back(std::make_pair(std::make_pair(stop_start, stop_finish), distance));//добавляем данные в вектор
    }

    return stops_distance;
}

std::string_view ParseQueryBus(std::string_view str) {
    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));//переводим начало строки на первый символ не пробел
    using namespace std::literals;
    auto pos = str.find(":"sv);//находим конец названия маршрута (включая пробелы)
    std::string_view bus = str.substr(0, pos);//оставляем строку от начала до элемента, предшествующего :
    bus = bus.substr(0, bus.find_last_not_of(' ') + 1);//оставляем строку от начала до последнего элемента, не пробела (удаляем пробелы в конце)

    return bus;
}

std::vector<std::string_view> MakeRoute(std::string_view str) {
    std::string_view stop_route;
    std::vector<std::string_view> bus_and_stops;

    using namespace std::literals;
    bool is_ring = false;//проверяем кольцевой маршрут или нет
    auto pos_ring = str.find("-"sv);
        if (pos_ring == std::string::npos) {
            is_ring = true;
        }

    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));//переводим начало строки на первый символ не пробел
    auto pos = str.find(":"sv);//находим конец названия маршрута (включая пробелы)
    std::string_view bus = str.substr(0, pos);//оставляем строку от начала до элемента, предшествующего :
    bus = bus.substr(0, bus.find_last_not_of(' ') + 1);//оставляем строку от начала до последнего элемента, не пробела (удаляем пробелы в конце)
    bus_and_stops.push_back(bus);//добавляем первым элементом название маршрута

    str.remove_prefix(std::min(str.find_first_not_of(' ', pos + 1), str.size())); // переводим начало строки на первый символ не пробел после :

    while (pos != str.npos) { // пока не дойдем до конца строки
        pos = str.find("-"sv); // находим -
        if (pos == std::string::npos) {
            pos = str.find(">"sv); // или >
        }
        stop_route = str.substr(0, pos);                                         // обрезаем название остановки до - или > (включая пробелы)
        stop_route = stop_route.substr(0, stop_route.find_last_not_of(' ') + 1); // убираем пробелы с конца
        str.remove_prefix(str.find_first_not_of(' ', pos + 1));                  // передвигаем начало на первый символ не пробел после - или >
        bus_and_stops.push_back(stop_route);
    }

    if (is_ring == false) {//добавляем остановки до кольцевого маршрута
        std::vector<std::string_view> copy_vec(bus_and_stops.size() - 2);//создаем вектор - буфер размером на 2 меньше (минус первый и последний элементы)
        std::reverse_copy(next(bus_and_stops.begin()), prev(bus_and_stops.end()), copy_vec.begin());//копируем элементы в обратном порядке кроме первого и последнего
        bus_and_stops.reserve(bus_and_stops.size() * 2 - 2);
        bus_and_stops.insert(bus_and_stops.end(), copy_vec.begin(), copy_vec.end());
    }
    return bus_and_stops;
}
}//namespace input
}//namespace catalogue
