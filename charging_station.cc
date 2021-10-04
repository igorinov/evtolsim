#include "charging_station.h"

void log(const char *tag, const char *format, ...);

bool ChargingStation::queue_is_empty(void)
{
    return now_serving == next_ticket;
}

int ChargingStation::queue_length()
{
    return next_ticket - now_serving;
}

ChargingStation::ChargingStation(int n)
{
    game_over = false;
    available_chargers = n;
    now_serving = 0;
    next_ticket = 0;
}

void ChargingStation::shutdown()
{
    game_over = true;
    cv_ticket_dispenser.notify_all();
    cv_now_serving.notify_all();
    cv_charging_started.notify_all();
    cv_charger_vacated.notify_all();
}

void ChargingStation::run()
{
    const char *tag = "Charging Station";

    while(!game_over) {
        std::unique_lock<std::mutex> lk(mx);

        while (available_chargers == 0) {
            cv_charger_vacated.wait(lk);
            if (game_over) {
                break;
            }
        }

        while (queue_is_empty()) {
            if (available_chargers == 1) {
                log(tag, "one charger available!");
            } else {
                log(tag, "%d chargers available!", available_chargers);
            }
            cv_ticket_dispenser.wait(lk);
            if (game_over) {
                break;
            }
        }

        if (!queue_is_empty() && available_chargers > 0) {
            now_serving += 1;
            available_chargers -= 1;
            log(tag, "now serving ticket #%d; vehicles in line: %d",
                    now_serving, queue_length());
            cv_now_serving.notify_all();
            cv_charging_started.wait(lk);
        }
    }
}

// This method must be called by the vehicle when it starts charging
// in order to update the "now serving" number
void ChargingStation::start_charging(void)
{
    std::unique_lock<std::mutex> lk(mx);
    cv_charging_started.notify_all();
}

// This method must be called by the vehicle when it ends charging
// to give the charger to the next vehicle in line
void ChargingStation::end_charging(void)
{
    std::unique_lock<std::mutex> lk(mx);
    available_chargers += 1;
    cv_charger_vacated.notify_all();
}

void ChargingStation::wait_for_available_charger(const char *callsign)
{
    std::unique_lock<std::mutex> lk(mx);

    // Please take a number
    int my_number = ++next_ticket;
    if (callsign != nullptr) {
        log(callsign, "pulled ticket #%d", my_number);
    }
    cv_ticket_dispenser.notify_all();
    while (now_serving != my_number) {
        cv_now_serving.wait(lk);
        if (game_over) {
            return;
        }
    }
}

