#include "geo.h"
#include "input_reader.h"
#include "log_duration.h"
#include "stat_reader.h"
#include "tests.h"
#include "transport_catalogue.h"

#include <fstream>

using namespace std;

int main() {
    //TestTransportCatalogue();

    catalogue::input::QueryInput q;
    catalogue::head::TransportCatalogue tc;
    {
        LOG_DURATION("ReadQueryInput"s);
        ReadQueryInput(q);
        }

        {
            LOG_DURATION("AddStop"s);
            tc.AddStop(q.text_stops_);
        }

        {
            LOG_DURATION("AddDistance"s);
            tc.AddDistance(q.text_stops_);
        }

        {
            LOG_DURATION("AddBus"s);
            tc.AddBus(q.text_buses_);
        }

        {
            LOG_DURATION("CompleteCatalogue"s);
            tc.CompleteCatalogue();
        }

    catalogue::stat::QueryStat q_stat;
        {
            LOG_DURATION("ReadQueryStat"s);
            ReadQueryStat(q_stat);
        }

        {
            LOG_DURATION("GetInfo"s);
            using namespace catalogue::detail;
            cout << tc.GetInfo(q_stat);
        }
}