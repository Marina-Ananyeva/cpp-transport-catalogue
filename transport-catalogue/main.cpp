#include "constants.h"
#include "geo.h"
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "json_reader.h"
#include "log_duration.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <fstream>

using namespace std;

int main() {
    std::ifstream in("input.json");
    std::streambuf *cinbuf = std::cin.rdbuf();
    std::cin.rdbuf(in.rdbuf());

    catalogue::head::TransportCatalogue tc;                                 //справочник
    catalogue::reader::Query q;                                             //запросы
    catalogue::renderer::RenderSetting r;                                   //настройки визуализации
    catalogue::renderer::MapObjects m;                                      //объекты визуализации

    {
        std::ofstream out("output_result.json");
        std::streambuf *coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf());
        
        catalogue::reader::FillCatalogue(tc, q, r, m, std::cin);           //считываем запросы, формируем транспортный каталог и карту
        catalogue::reader::ExecuteStatRequests(tc, q, m, std::cout);       //отвечаем на запросы в формате json
    
        std::cin.rdbuf(cinbuf);
        std::cout.rdbuf(coutbuf);
    
    }
/*
    {
        std::ofstream out("output_result.xml");
        std::streambuf *coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf());
        
        catalogue::renderer::PrintMap(std::cout, m);                        //выводим карту в формате xml

        std::cin.rdbuf(cinbuf);
        std::cout.rdbuf(coutbuf);
    }
*/
}