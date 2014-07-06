from flare import *

if __name__ == "__main__":
    print("Beginning to run Flare via Python!")

    # Init
    render_device_name = "sdl"
    flare_init(render_device_name)

    # Set up
    debug = False
    MAX_FRAMES_PER_SEC = 60.
    delay = int(1000./MAX_FRAMES_PER_SEC+0.5)
    logic_ticks = flare_game_ticks()

    # Main Loop
    while not flare_done():
        now_ticks = flare_game_ticks()
        prev_ticks = now_ticks

        # Execute the game logic
        logic_ticks = flare_simulate(logic_ticks, delay)

        # Render to screen
        flare_render(prev_ticks, delay)

        # delay quick frames
        flare_delay(prev_ticks, delay)

    flare_cleanup()

    print("Python Success!")