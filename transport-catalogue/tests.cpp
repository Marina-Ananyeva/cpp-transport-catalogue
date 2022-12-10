#include "tests.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void TestTransportCatalogueProcessingQueryInput() {
    //test ReadQueryInput()
    {
        using namespace std::literals;
        catalogue::input::QueryInput q;
        std::istringstream input;
        input.str("1 Stop Tolstopaltsevo: 55.611087, 37.208290"s);
        catalogue::input::ReadQueryInput(input, q);
        input.clear();
        assert(q.text_stops_.size() == 1);
        assert(q.text_stops_[0] == " Tolstopaltsevo: 55.611087, 37.208290"s);

        std::cout << "TestTransportCatalogueProcessingQueryInput of ReadQueryInput is OK"s << std::endl;
    }

    //test read stop
    {
        using namespace std::literals;
        catalogue::input::QueryInput q;
        std::istringstream input;
        input.str("Stop Tolstopaltsevo: 55.611087, 37.208290"s);
        input >> q;
        input.clear();
        assert(q.type == catalogue::input::QueryInputType::NewStop);
        assert(!q.text_stops_.empty());
        assert(q.text_stops_.size() == 1);
        assert(q.text_stops_[0] == " Tolstopaltsevo: 55.611087, 37.208290"s);

        input.str("Stop Marushkino: 55.595884, 37.209755"s);
        input >> q;
        input.clear();
        assert(q.text_stops_.size() == 2);
        assert(q.text_stops_[1] == " Marushkino: 55.595884, 37.209755"s);

        std::cout << "TestTransportCatalogueProcessingQueryInput of NewStop is OK"s << std::endl;
    }
    //test read bus
    {
        using namespace std::literals;
        catalogue::input::QueryInput q;
        std::istringstream input;
        input.str("Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam"s);
        input >> q;
        input.clear();
        assert(q.type == catalogue::input::QueryInputType::NewBus);
        assert(!q.text_buses_.empty());
        assert(q.text_buses_.size() == 1);
        assert(q.text_buses_[0] == " 256: Biryulyovo Zapadnoye > Biryusinka > Universam"s);
        std::cout << "TestTransportCatalogueProcessingQueryInput of NewBus is OK"s << std::endl;
    }
}

void TestTransportCatalogueAddStop() {
    using namespace std::literals;
    catalogue::input::QueryInput q;
    std::istringstream input;
    input.str("Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s);
    input >> q;
    input.clear();

    catalogue::head::TransportCatalogue tc;
    tc.AddStop(q.text_stops_);
    assert(tc.stops_.size() == 3);
    assert(tc.stops_[0].stop_ == "Tolstopaltsevo"sv);
    assert(tc.stops_[0].geo_.first == 55.611087);
    assert(tc.stops_[0].geo_.second == 37.208290);

    assert(tc.stops_[1].stop_ == "Marushkino"sv);
    assert(tc.stops_[1].geo_.first == 55.595884);
    assert(tc.stops_[1].geo_.second == 37.209755);

    assert(tc.stopname_to_stop_.size() == 3);

    std::cout << "TestTransportCatalogueAddStops is OK"s << std::endl;
}

void TestTransportCatalogueAddDistance() {
    using namespace std::literals;
    catalogue::input::QueryInput q;
    std::istringstream input;
    input.str("Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s);
    input >> q;
    input.clear();

    catalogue::head::TransportCatalogue tc;
    tc.AddStop(q.text_stops_);
    auto test = tc.stops_distance_;
    assert(test.size() == 4);
}

void TestTransportCatalogueAddBus() {
    using namespace std::literals;
    catalogue::input::QueryInput q;
    std::istringstream input;
    input.str("Stop Tolstopaltsevo: 55.611087, 37.208290"s);
    input >> q;
    input.clear();
    input.str("Stop Marushkino: 55.595884, 37.209755"s);
    input >> q;
    input.clear();

    input.str("Bus 750: Tolstopaltsevo - Marushkino"s);
    input >> q;
    input.clear();

    catalogue::head::TransportCatalogue tc;
    tc.AddStop(q.text_stops_);
    tc.AddBus(q.text_buses_);
    assert(tc.buses_.size() == 1);
    assert(tc.buses_[0].bus_ == "750"sv);

    assert(tc.buses_[0].bus_and_stops_.size() == 3);
    assert(tc.buses_[0].bus_and_stops_[0]->GetStop() == "Tolstopaltsevo"sv);

    //test add reverse stops
    const catalogue::head::TransportCatalogue::Bus bus = tc.GetBusIndex("750"sv);
    assert(bus.bus_and_stops_.size() == 3);

    assert(tc.busname_to_bus_.size() == 1);
    std::cout << "TestTransportCatalogueAddBuses is OK"s << std::endl;
}

void TestTransportCatalogueGetIndexAndPtr() {
    using namespace std::literals;
    catalogue::input::QueryInput q;
    std::istringstream input;
    input.str("Stop Tolstopaltsevo: 55.611087, 37.208290"s);
    input >> q;
    input.clear();
    input.str("Stop Marushkino: 55.595884, 37.209755"s);
    input >> q;
    input.clear();

    input.str("Bus 750: Tolstopaltsevo - Marushkino"s);
    input >> q;
    input.clear();

    catalogue::head::TransportCatalogue tc;
    tc.AddStop(q.text_stops_);
    tc.AddBus(q.text_buses_);

    //test GetStopIndex
    const catalogue::head::TransportCatalogue::Stop stop_index = tc.GetStopIndex("Marushkino"sv);
    assert(stop_index.GetStop() == "Marushkino"sv);

    //test GetBusIndex
    const catalogue::head::TransportCatalogue::Bus bus_index = tc.GetBusIndex("750"sv);
    assert(bus_index.GetBus() == "750"sv);

    //test GetStopPtr
    const catalogue::head::TransportCatalogue::Stop *stop_ptr = tc.GetStopPtr("Marushkino"sv);
    const catalogue::head::TransportCatalogue::Stop stop_test = *stop_ptr;
    assert(stop_test.GetStop() == "Marushkino"sv);

    //test GetBusPtr
    const catalogue::head::TransportCatalogue::Bus *bus_ptr = tc.GetBusPtr("750"sv);
    const catalogue::head::TransportCatalogue::Bus bus_test = *bus_ptr;
    assert(bus_test.GetBus() == "750"sv);

    std::cout << "TestTransportCatalogueGetIndexAndPtr is OK"s << std::endl;
}

void TestTransportCatalogueProcessingQueryStat() {
    
    {
        using namespace std::literals;
        catalogue::stat::QueryStat q;
        std::istringstream input;
        input.str("1 Bus 256"s);

        catalogue::stat::ReadQueryStat(input, q);
        input.clear();
        assert(q.query_stat_[0].first == catalogue::stat::QueryTypeStat::StatBus);
        assert(q.query_stat_.size() == 1);
        assert(q.query_stat_[0].second == "256"s);
        std::cout << "TestTransportCatalogueProcessingQueryStat of ReadQueryStat is OK"s << std::endl;
    }

    //test read stat bus
    {
        using namespace std::literals;
        catalogue::stat::QueryStat q;
        std::istringstream input;
        input.str("Bus 256"s);
        input >> q;
        input.clear();
        assert(q.query_stat_[0].first == catalogue::stat::QueryTypeStat::StatBus);
        assert(q.query_stat_.size() == 1);
        assert(q.query_stat_[0].second == "256"s);
        std::cout << "TestTransportCatalogueProcessingQueryStat of ReadyBusStat is OK"s << std::endl;
    }
}

void TestTransportCatalogueGetStopsForBus(){
    using namespace std::literals;
    catalogue::input::QueryInput q;
    std::istringstream input;
    input.str("Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s);
    input >> q;
    input.clear();

    input.str("Bus 750: Tolstopaltsevo - Marushkino"s);
    input >> q;
    input.clear();

    catalogue::head::TransportCatalogue tc;
    tc.AddStop(q.text_stops_);
    tc.AddBus(q.text_buses_);

    catalogue::stat::QueryStat q_st;
    {
        input.str("Bus 750"s);
        input >> q_st;
        input.clear();
        auto test = catalogue::stat::GetStatInfo(tc, q_st);
        auto t = test.info_[0].first.stops_for_bus_;
        assert(std::get<1>(t) == 3);
        assert(std::get<2>(t) == 2);
    }
    {
        input.str("Bus 200"s);
        input >> q_st;
        input.clear();
        auto test = catalogue::stat::GetStatInfo(tc, q_st);
        auto t = test.info_[1].first.stops_for_bus_;
        assert(std::get<1>(t) == 0);
        assert(std::get<2>(t) == 0);
        assert(std::get<3>(t) == 0);
        assert(std::get<3>(t) == 0.0);
    }
    std::cout << "TestTransportCatalogueGetStopsForBus is OK"s << std::endl;
}

void TestTransportCatalogueGetBusesForStop() {
    using namespace std::literals;
    catalogue::input::QueryInput q;
    std::istringstream input;
    input.str("Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s);
    input >> q;
    input.clear();
    input.str("Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s);
    input >> q;
    input.clear();

    input.str("Bus 750: Tolstopaltsevo - Marushkino"s);
    input >> q;
    input.clear();

    catalogue::head::TransportCatalogue tc;
    tc.AddStop(q.text_stops_);
    tc.AddBus(q.text_buses_);

    {
        catalogue::stat::QueryStat q_st;
        input.str("Stop Marushkino"s);
        input >> q_st;
        input.clear();
        auto test = catalogue::stat::GetStatInfo(tc, q_st);
        assert(test.info_[0].second.buses_for_stop_.size() == 1);
    }

    {
        catalogue::stat::QueryStat q_st;
        input.str("Stop Perovo"s);
        input >> q_st;
        input.clear();
        auto test = catalogue::stat::GetStatInfo(tc, q_st);
        assert(test.info_.size() == 1);
    }

    {
        catalogue::stat::QueryStat q_st;
        input.str("Stop Perovo"s);
        input >> q_st;
        input.clear();
        input.str("Stop Prazhskaya"s);
        input >> q_st;
        input.clear();
        input.str("Stop Marushkino"s);
        input >> q_st;
        input.clear();
        auto test = catalogue::stat::GetStatInfo(tc, q_st);
    }
    std::cout << "TestTransportCatalogueGetBusesForStop is OK"s << std::endl;
}

void TestTransportCatalogue() {
    TestTransportCatalogueProcessingQueryInput();
    TestTransportCatalogueAddStop();
    TestTransportCatalogueAddDistance();
    TestTransportCatalogueAddBus();
    TestTransportCatalogueGetIndexAndPtr();
    TestTransportCatalogueProcessingQueryStat();
    TestTransportCatalogueGetStopsForBus();
    TestTransportCatalogueGetBusesForStop();

    using namespace std::literals;
    std::cout << "Test passed!"s << std::endl;
}