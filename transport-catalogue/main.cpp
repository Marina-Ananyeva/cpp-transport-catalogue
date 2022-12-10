#include "constants.h"
#include "geo.h"
#include "input_reader.h"
#include "log_duration.h"
#include "stat_reader.h"
#include "tests.h"
#include "transport_catalogue.h"

#include <fstream>

using namespace std;

int main() {
    //std::ifstream in("tsB_case1_input.txt");
    //std::streambuf *cinbuf = std::cin.rdbuf();
    //std::cin.rdbuf(in.rdbuf());

    //std::ofstream out("tsB_output.txt");
    //std::streambuf *coutbuf = std::cout.rdbuf();
    //std::cout.rdbuf(out.rdbuf());
    TestTransportCatalogue();
    catalogue::input::QueryInput q;
    catalogue::head::TransportCatalogue tc;
        {
            LOG_DURATION("ReadQueryInput"s);
            ReadQueryInput(std::cin, q);
        }

        {
            LOG_DURATION("AddStop"s);
            tc.AddStop(q.text_stops_);
        }

        {
            LOG_DURATION("AddBus"s);
            tc.AddBus(q.text_buses_);
        }

        catalogue::stat::QueryStat q_stat;
        {
            LOG_DURATION("ReadQueryStat"s);
            ReadQueryStat(std::cin, q_stat);
        }

        {
            LOG_DURATION("GetInfo"s);
            cout << catalogue::stat::GetStatInfo(tc, q_stat);
        }

        //std::cin.rdbuf(cinbuf);
        //std::cout.rdbuf(coutbuf);
}
/*
Пример
Ввод
13
Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka
Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
Stop Biryusinka: 55.581065, 37.64839, 750m to Universam
Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye
Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
Stop Prazhskaya: 55.611678, 37.603831
6
Bus 256
Bus 750
Bus 751
Stop Samara
Stop Prazhskaya
Stop Biryulyovo Zapadnoye
Вывод
Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature
Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature
Bus 751: not found
Stop Samara: not found
Stop Prazhskaya: no buses
Stop Biryulyovo Zapadnoye: buses 256 828 
*/