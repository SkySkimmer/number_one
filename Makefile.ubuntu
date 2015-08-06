CC = g++
CXX = g++
CPPFLAGS = -MMD
CXXFLAGS = -O3 -std=c++11
LDLIBS = -lcolamd -lm -ldl -llpsolve55
SRCS = eval_state.cc game_state.cc number_one_main.cc player.cc zs_wrap.cc

number_one_main: $(SRCS:.cc=.o)

clean:
	rm -f number_one_main *.o *~

.PHONY: clean

-include $(SRCS:.cc=.d)
