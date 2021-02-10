--
-- Enable frameskip until the attract sequence is displayed
--
-- This script requires MAME 0.228 or newer
--

-- Amount of emulated time to disable frameskip for. Note that some of these
-- were computed using an older version of MAME and may need to be adjusted.
local wait_times = {
    ddr4m = 78,
	ddr4mp = 81,
	ddr5m = 68,
	ddrmax2 = 72,
	ddrextrm = 95,
	ddrexproc = 95,
}

wait_time = wait_times[emu.romname()] or 0

manager.machine:popmessage("Please wait while the game boots up")
manager.machine.video.frameskip = 12

ff_active = true

fast_forward = coroutine.create(function ()
    -- while(true) do
	while(emu.time() < wait_time) do
        emu.wait(1)
        -- print(emu.time())
    end

	ff_active = false
    manager.machine.video.frameskip = 0
end)

screen = manager.machine.screens[":screen"]

function draw_overlay()
	if ff_active then
		screen:draw_text("right", 220, ">>", 0xFFFF0000, 0xFF000000)
	end
end

coroutine.resume(fast_forward)

emu.register_frame_done(draw_overlay)
