from libcpp cimport bool
from libcpp.string cimport string

cdef extern from "../src/main.h":
    void init(string render_device_name)
    void mainLoop (bool debug_event)
    void cleanup()
    bool done()
    int game_ticks()
    int simulate(int logic_ticks, bool debug_event, int delay)
    void render(int prev_ticks, int delay)
    void delay_loop(int prev_ticks, int delay)

def flare_init(string renderer):
    init(renderer)

def flare_mainLoop(bool debug):
    mainLoop(debug)

def flare_cleanup():
    cleanup()

def flare_done():
    return done()

def flare_game_ticks():
    return game_ticks()

def flare_simulate(int logic_ticks, int delay, bool debug=False):
    return simulate(logic_ticks, debug, delay)

def flare_render(int prev_ticks, int delay):
    render(prev_ticks, delay)

def flare_delay(int prev_ticks, int delay):
    delay_loop(prev_ticks, delay)
