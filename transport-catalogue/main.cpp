#include "constants.h"
#include "geo.h"
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "json_reader.h"
#include "log_duration.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <transport_catalogue.pb.h>

using namespace std::literals;
/*
int main() {
    {
        catalogue::head::TransportCatalogue tc;                                     //справочник
        catalogue::reader::Query q;                                                 //запросы
        catalogue::renderer::RenderSettings r;                                      //настройки визуализации
        catalogue::renderer::MapObjects m;                                          //объекты визуализации
        catalogue::routing::RoutingSettings rt;                                     //настройки маршрутизации  

        std::ifstream in("input1.json");
        std::streambuf *cinbuf = std::cin.rdbuf();
        std::cin.rdbuf(in.rdbuf());

        catalogue::reader::FillCatalogue(tc, q, r, m, rt, std::cin);                    //считываем запросы, формируем транспортный каталог и карту

        std::ofstream out_file(tc.GetSerializeFileName(), std::ios::binary);

        catalogue::serialize::SerializeTransportCatalogue(tc, q, r, m, rt, out_file);    //сериализуем транспортный каталог в proto файл

        std::cin.rdbuf(cinbuf);
    } 

    {
        catalogue::head::TransportCatalogue tc;                                     //справочник
        catalogue::reader::Query q;                                                 //запросы
        catalogue::renderer::RenderSettings r;                                      //настройки визуализации
        catalogue::renderer::MapObjects m;                                          //объекты визуализации
        catalogue::routing::RoutingSettings rt;                                     //настройки маршрутизации

        {
            std::ifstream in("input2.json");
            std::streambuf *cinbuf = std::cin.rdbuf();
            std::cin.rdbuf(in.rdbuf());
            std::ofstream out("output_result.json");
            std::streambuf *coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());

            catalogue::reader::ReadRequest(tc, q, std::cin);                                //считываем запросы в формате json

            std::ifstream input_file(tc.GetSerializeFileName(), std::ios::binary);

            catalogue::serialize::DeserializeTransportCatalogue(input_file, tc, r, m, rt);  //десериализуем все данные из proto файлов

            catalogue::reader::ExecuteStatRequests(tc, q, m, rt, std::cout);                //отвечаем на запросы в формате json

            std::cout.rdbuf(coutbuf);
            std::cin.rdbuf(cinbuf);
        }

        {
            std::ofstream out("output_result.xml");
            std::streambuf *coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());
            
            catalogue::renderer::PrintMap(std::cout, m);                                    //выводим карту в формате xml

            std::cout.rdbuf(coutbuf);
        }
    }
}
*/
///*
void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        catalogue::head::TransportCatalogue tc;                                     //справочник
        catalogue::reader::Query q;                                                 //запросы
        catalogue::renderer::RenderSettings r;                                      //настройки визуализации
        catalogue::renderer::MapObjects m;                                          //объекты визуализации
        catalogue::routing::RoutingSettings rt;                                     //настройки маршрутизации  

        //std::ifstream in("input1.json");
        //std::streambuf *cinbuf = std::cin.rdbuf();
        //std::cin.rdbuf(in.rdbuf());

        catalogue::reader::FillCatalogue(tc, q, r, m, rt, std::cin);                //считываем запросы, формируем транспортный каталог и карту

        std::ofstream out_file(tc.GetSerializeFileName(), std::ios::binary);

        catalogue::serialize::SerializeTransportCatalogue(tc, q, r, m, rt, out_file);                    //сериализуем транспортный каталог в proto файл

        //std::cin.rdbuf(cinbuf);
    } else if (mode == "process_requests"sv) {
        catalogue::head::TransportCatalogue tc;                                     //справочник
        catalogue::reader::Query q;                                                 //запросы
        catalogue::renderer::RenderSettings r;                                      //настройки визуализации
        catalogue::renderer::MapObjects m;                                          //объекты визуализации
        catalogue::routing::RoutingSettings rt;                                     //настройки маршрутизации         

        //std::ifstream in("input2.json");
        //std::streambuf *cinbuf = std::cin.rdbuf();
        //std::cin.rdbuf(in.rdbuf());
        //std::ofstream out("output_result.json");
        //std::streambuf *coutbuf = std::cout.rdbuf();
        //std::cout.rdbuf(out.rdbuf());

        catalogue::reader::ReadRequest(tc, q, std::cin);                            //считываем запросы в формате json

        std::ifstream input_file(tc.GetSerializeFileName(), std::ios::binary);

        catalogue::serialize::DeserializeTransportCatalogue(input_file, tc, r, m, rt);  //десериализуем все данные из proto файлов

        catalogue::reader::ExecuteStatRequests(tc, q, m, rt, std::cout);            //отвечаем на запросы в формате json

        //std::cout.rdbuf(coutbuf);
        //std::cin.rdbuf(cinbuf);
    } else {
        PrintUsage();
        return 1;
    }
}
//*/