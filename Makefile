all: evtolsim

charging_station.o: charging_station.cc
	g++ -c -o $@ $^

evtolsim: evtolsim.cc charging_station.o
	g++ -o $@ $^ -lpthread

