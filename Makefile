CC = clang++
CXX = clang++
CPPFLAGS = -MMD
CXXFLAGS = -O3 -std=c++11 -m32
LDFLAGS = -m32
LDLIBS = -lm -ldl
SRCS = eval_state.cc game_state.cc number_one_main.cc player.cc zs_wrap.cc

number_one_main: $(SRCS:.cc=.o) liblpsolve55.a

clean:
	rm -f number_one_main *.o *~

.PHONY: clean

-include $(SRCS:.cc=.d)
