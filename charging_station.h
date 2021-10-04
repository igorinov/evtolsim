#ifndef __CHARGING_STATION_H
#define __CHARGING_STATION_H

#include <thread>
#include <mutex>
#include <condition_variable>

class ChargingStation
{
private:

    int available_chargers;
    int next_ticket;
    int now_serving;
    bool game_over;
    std::mutex mx;
    std::condition_variable cv_ticket_dispenser;
    std::condition_variable cv_now_serving;
    std::condition_variable cv_charger_vacated;
    std::condition_variable cv_charging_started;

protected:

    bool queue_is_empty(void);
    int queue_length();

public:

    ChargingStation(int n);
    void shutdown();

    // This method must be called from a new thread
    void run();

    // This method must be called by the vehicle when it starts charging
    // in order to update the "now serving" number
    void start_charging(void);

    // This method must be called by the vehicle when it ends charging
    // to give the charger to the next vehicle in line
    void end_charging(void);

    void wait_for_available_charger(const char *callsign);
};

#endif // __CHARGING_STATION_H

