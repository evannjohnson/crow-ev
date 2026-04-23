Documentation for features of my (`evnoj`) firmware fork. See the [monome docs](https://monome.org/docs/crow/) for the full documentation.

# output modes
Outputs can be set into different modes, similar to [input modes](https://monome.org/docs/crow/reference/#input-modes):
```lua
output[1].mode = "some_mode"
```

The current modes are:
- `"asl"`: In the vanilla FW, this is the "mode" that outputs are always in. Outputs default to this mode to preserve compatibility with scripts for the vanilla fw.
- [spinner](#spinner)

## spinner
The spinner mode makes an output run a ramp oscillator, with configurable start and end points, that can be stopped and run in reverse (making it a falling ramp). It was created for the [Mannequins Silhouette](https://www.whimsicalraps.com/products/silhouette)'s SPOT parameter, which is "circular". The knob that controls the param is endless, because the param is a continuous 360° rotation. However, CV cannot rise or fall forever, so to implement "circular" modulation of this parameter requires instantaneous transition of the voltage to the other end of the CV range when the CV hits the limit of the parameter's CV capability. This is a rising ramp for clockwise rotation, and a falling ramp for counterclockwise rotation.

It should work with other modules that have a "circular" parameter, such as the [Unknown Devices Eisei](https://unwn.dev/shop/modules/eisei/). If you try it, let me know!

```lua
output[1].mode = "spinner"
output[1].time = 1000 -- time of a cycle in ms, default 1000, 1-10000000 ms (166 minutes)
output[1].time -- returns 1000, get/set every param this way

-- params:
output[1].pos -- 0-1, automatically moves with time or can be get/set directly
output[1].phase_offset -- 0-1, an additional way to control the rotation, default 0
output[1].phase_offset = 3.3 -- implicit modulo 1, same as output[1].phase_offset = 0.3

output[1].direction -- 0, 1, or -1; 0 is stopped, 1 is rising, -1 is falling, default 0

output[1].bottom -- bottom voltage of oscillation, default -5 (min is -5 due to hw)
output[1].top -- top voltage of oscillation, default 5

output[1]:delta_pos(0.5) -- directly add to pos
output[1]:delta_phase_offset(0.5) -- directly add to phase_offset
```

