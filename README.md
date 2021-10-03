# eVTOL Simulation Solution


## Building

```
make
```

## Running

Default time scale (60x), one simulated minute is equivalent to one second on the wall clock:

```
./evtol
```

Custom time scale, e.g. 600x:

```
 ./evtol 600
```

## Results

Column | Description
--|--
Callsign | Call sign of the vehicle, starts with the first letter of the company name
AvgFlight | Average flight time in hours
AvgCharge | Average charging time in hours
PMi | Passenger miles, sum of the products obtained by multiplying the number of passengers carried on each flight by the flight distance
Exp. fail | Expected number of failures, total flight time multiplied by failure rate

Example of results table:

```
Callsign AvgFlight AvgCharge   AvgWait    PMi  Exp. fail
      D1      1:40      0:28      0:51    300  0.366667
      B1      0:40      0:12      1:27    666  0.133333
      C1      0:37      0:48      0:57    600  0.062500
      A1      1:40      0:16      1:03    800  0.416667
      D2      1:40      0:00      0:00    300  0.366667
      A2      1:40      0:00      0:00    800  0.416667
      B2      0:40      0:12      1:27    666  0.133333
      C2      0:37      0:48      0:57    600  0.062500
      E1      0:40      0:18      1:21     80  0.815367
      B3      0:40      0:12      1:27    666  0.133333
      C3      0:37      0:48      0:57    600  0.062500
      C4      0:37      0:48      0:57    600  0.062500
      A3      1:40      0:00      0:00    800  0.416667
      E2      0:34      0:18      1:33     68  0.693468
      D3      1:40      0:00      0:00    300  0.366667
      B4      0:40      0:12      1:27    666  0.133333
      A4      1:40      0:00      0:00    800  0.416667
      B5      0:40      0:00      0:00    333  0.066667
      C5      0:37      0:48      0:56    600  0.062500
      B6      0:37      0:12      1:33    620  0.124150
```
