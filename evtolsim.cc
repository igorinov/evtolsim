#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "charging_station.h"

using namespace std::this_thread;
using namespace std::chrono;

// 1 minute of simulated time is equivalent to 1 second on the wall clock
double time_scale = 60.0;

// The simulation is shutting down
bool game_over = false;

// Condition variable to abort simulated flights
std::condition_variable cv_abort_flights;

auto start_time = std::chrono::high_resolution_clock::now();

std::mutex log_mx;
void log(const char *tag, const char *format, ...)
{
    std::unique_lock<std::mutex> lk(log_mx);
    auto now = std::chrono::high_resolution_clock::now();
    ldiv_t ld;
    int h, m, s;

    // time since start of simulation, in simulated seconds
    s = duration_cast<milliseconds>(now - start_time).count() * time_scale / 1000.0;
    ld = ldiv(s, 60);
    s = ld.rem;
    m = ld.quot;
    ld = ldiv(m, 60);
    m = ld.rem;
    h = ld.quot;
    printf("%4d:%02d:%02d  %s: ", h, m, s, tag);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

ChargingStation charging_station(3);

class Vehicle
{
public:

    double cruise_speed;
    double battery_capacity;
    double time_to_charge;
    double energy_use;
    int passenger_count;
    double p_faults_per_hour;

    int number_of_flights;
    int number_of_charges;
    int number_of_lines;
    double total_flight_time;
    double total_charge_time;
    double total_wait_time;
    const char *callsign = nullptr;

    Vehicle(const char *cs)
    {
        callsign = cs;
        number_of_flights = 0;
        number_of_charges = 0;
        number_of_lines = 0;

        total_flight_time = 0;
        total_charge_time = 0;
        total_wait_time = 0;
    }

    void run()
    {
        std::mutex mx;
        std::unique_lock<std::mutex> lk(mx);

        while(!game_over) {
            double dt = battery_capacity / (energy_use * cruise_speed);
            log(callsign, "back in service");
            number_of_flights += 1;

            // flight time in simulated hours to wall clock milliseconds
            auto p =  milliseconds((int) (dt * 3600000 / time_scale));

            // Flying until the battery is empty, abort if end of simulation is signaled
            auto t0 = std::chrono::high_resolution_clock::now();
            std::cv_status status = cv_abort_flights.wait_for(lk, p);
            auto t1 = std::chrono::high_resolution_clock::now();

            // Wall clock milliseconds to simulated hours
            total_flight_time += duration_cast<milliseconds>(t1 - t0).count() * time_scale / 3600000;

            if (status != std::cv_status::timeout) {
                log(callsign, "flight aborted");
                break;
            }

            log(callsign, "battery empty, in line for charging");
            number_of_lines += 1;
            t0 = std::chrono::high_resolution_clock::now();
            charging_station.wait_for_available_charger(callsign);
            t1 = std::chrono::high_resolution_clock::now();

            // Waiting time in simulated hours
            double wt = duration_cast<milliseconds>(t1 - t0).count() * time_scale / 3600000;
            total_wait_time += wt;

            // Waiting time in simulated minutes
            int wt_min = wt * 60.0;
            log(callsign, "wait time was %d:%02d", wt_min / 60, wt_min % 60);

            if (game_over) {
                break;
            }

            log(callsign, "charging...");
            number_of_charges += 1;
            charging_station.start_charging();

            // time_to_charge in simulated hours to wall clock milliseconds
            p = milliseconds((int) (time_to_charge * 3600000 / time_scale));

            t0 = std::chrono::high_resolution_clock::now();
            status = cv_abort_flights.wait_for(lk, p);
            t1 = std::chrono::high_resolution_clock::now();

            // Charging time in simulated hours
            total_charge_time += duration_cast<milliseconds>(t1 - t0).count() * time_scale / 3600000;

            if (status != std::cv_status::timeout) {
                log(callsign, "charging aborted");
                break;
            }

            charging_station.end_charging();
        }
    }
};

class VehicleAlpha : public Vehicle
{
public:
    VehicleAlpha(const char *callsign) : Vehicle(callsign)
    {
        cruise_speed = 120;
        battery_capacity = 320;
        time_to_charge = 0.6;
        energy_use = 1.6;
        passenger_count = 4;
        p_faults_per_hour = 0.25;
    }
};

class VehicleBeta : public Vehicle
{
public:
    VehicleBeta(const char *callsign) : Vehicle(callsign)
    {
        cruise_speed = 100;
        battery_capacity = 100;
        time_to_charge = 0.2;
        energy_use = 1.5;
        passenger_count = 5;
        p_faults_per_hour = 0.10;
    }
};

class VehicleCharlie : public Vehicle
{
public:
    VehicleCharlie(const char *callsign) : Vehicle(callsign)
    {
        cruise_speed = 160;
        battery_capacity = 220;
        time_to_charge = 0.8;
        energy_use = 2.2;
        passenger_count = 3;
        p_faults_per_hour = 0.05;
    }
};

class VehicleDelta : public Vehicle
{
public:
    VehicleDelta(const char *callsign) : Vehicle(callsign)
    {
        cruise_speed = 90;
        battery_capacity = 120;
        time_to_charge = 0.62;
        energy_use = 0.8;
        passenger_count = 2;
        p_faults_per_hour = 0.22;
    }
};

class VehicleEcho : public Vehicle
{
public:
    VehicleEcho(const char *callsign) : Vehicle(callsign)
    {
        cruise_speed = 30;
        battery_capacity = 150;
        time_to_charge = 0.3;
        energy_use = 5.8;
        passenger_count = 2;
        p_faults_per_hour = 0.61;
    }
};

void charging_run()
{
    charging_station.run();
}

const char letters[] = "ABCDE";

int main(int argc, char **argv)
{
    std::vector<Vehicle> vehicles;
    std::vector<std::thread> vehicle_threads;
    std::thread charging_thread(charging_run);
    char callsign[16];
    int n_types = 5;
    int type_counters[n_types];
    int i, k;

    if (argc > 1) {
        time_scale = atof(argv[1]);
    }

    for (k = 0; k < n_types; k += 1) {
        type_counters[k] = 0;
    }

    srand(duration_cast<milliseconds>(start_time.time_since_epoch()).count());

    for (i = 0; i < 20; i += 1) {
        // Randomly select vehicle type
        k = rand() % n_types;

        // Create a unique call sign
        callsign[0] = letters[k];
        sprintf(callsign + 1, "%d", ++type_counters[k]);

        switch (k) {
        case 0:
            vehicles.emplace_back(VehicleAlpha(strdup(callsign)));
            break;
        case 1:
            vehicles.emplace_back(VehicleBeta(strdup(callsign)));
            break;
        case 2:
            vehicles.emplace_back(VehicleCharlie(strdup(callsign)));
            break;
        case 3:
            vehicles.emplace_back(VehicleDelta(strdup(callsign)));
            break;
        case 4:
            vehicles.emplace_back(VehicleEcho(strdup(callsign)));
            break;
        default:
            printf("Error: unknown vehicle type (%d) for %s\n", k, callsign);
        }
    }

    // Create a simulation thread for each vehicle
    for (auto& v : vehicles) {
        vehicle_threads.push_back(std::thread([&] { v.run(); } ));
    }

    // 3 hours, scaled down to wall clock seconds
    int sim_hours = 3;
    sleep_for(seconds((int) (sim_hours * 3600 / time_scale)));

    log("main", "game over, shutting down vehicles...");

    game_over = true;
    cv_abort_flights.notify_all();
    charging_station.shutdown();

    // Join all vehicle threads
    for (auto& t : vehicle_threads) {
        t.join();
    }

    // Join the charging station thread
    charging_thread.join();

    printf("\nCallsign AvgFlight AvgCharge   AvgWait    PMi  Exp. fail\n");
    for (auto& v : vehicles) {
        int avg_flight_min = 0;
        int avg_charge_min = 0;
        int avg_wait_min = 0;
        int pmi = v.total_flight_time * v.cruise_speed * v.passenger_count;

        if (v.number_of_flights > 0) {
            avg_flight_min = v.total_flight_time * 60.0 / v.number_of_flights;
        }

        if (v.number_of_charges > 0) {
            avg_charge_min = v.total_charge_time * 60.0 / v.number_of_charges;
        }

        if (v.number_of_lines > 0) {
            avg_wait_min = v.total_wait_time * 60.0 / v.number_of_lines;
        }

        printf("%8s %6d:%02d %6d:%02d %6d:%02d %6d  %lf\n",
                v.callsign,
                avg_flight_min / 60, avg_flight_min % 60,
                avg_charge_min / 60, avg_charge_min % 60,
                avg_wait_min / 60, avg_wait_min % 60,
                pmi,
                v.total_flight_time * v.p_faults_per_hour);

        double t = v.total_flight_time + v.total_charge_time + v.total_wait_time;
        if (fabs(t - sim_hours) > 0.01) {
            printf("Error: sum of time counters for %s (%lf hours)"
                    " does not match the simulation time\n",
                    v.callsign, t);
        }
    }

    return 0;
}

