all: evtolsim

evtolsim: evtolsim.cc
	g++ -o $@ $< -lpthread

