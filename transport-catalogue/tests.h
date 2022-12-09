#pragma once

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>

//namespace catalogue {
//namespace test {
void TestTransportCatalogueProcessingQueryInput();

void TestTransportCatalogueAddStop();

void TestTransportCatalogueAddDistance();

void TestTransportCatalogueAddBus();

void TestTransportCatalogueGetIndexAndPtr();

void TestTransportCatalogueCompleteCatalogue();

void TestTransportCatalogueProcessingQueryStat();

void TestTransportCatalogueGetStopsForBus();

void TestTransportCatalogueGetBusesForStop();

void TestTransportCatalogue();
//}
//}