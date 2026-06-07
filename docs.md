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

## telexi
new `ii.txi` commands:
- the equivalent of `TI.PARAM.MAP x y z` or `TI.IN.MAP` requires two commands, a `bot` to set y and a `top` to set z. `ii.txi.in_bot 1 -5; ii.txi.in_top 1 5` is like `TI.IN.MAP 1 V -5 V 5` (map input 1 from -5 to 5). The behavior from teletype is a bit different because all the values sent to the telexi are implicitly representing "volts", like prefacing with `V` on teletype. You can't get Telexi to map to integers from crow, you'll need to use `math.ceil` on the crow to do that.
  - `param_bot(x, y)`: maps the PARAM values for input x across the range y - z
  - `param_top(x, z)`: maps the PARAM values for input x across the range y - z
  - `in_bot(x, y)`: maps the IN values for input x across the range y - z
  - `in_top(x, z)`: maps the IN values for input x across the range y - z
- `param_scale(x, scale)`: select scale for PARAM knob x
- `in_scale(x, scale)`: select scale for input x
- `param_calib(channel, y)`: calibrates scaling for PARAM knob, y of 0 sets bottom bound, y of 1 sets top bound
- `in_calib(channel, y)`: calibrates scaling for IN jack, y of -1 sets -10V point, y of 0 sets 0V point, y of 1 sets 10V point
- `store(device)`: stores the calibration data for the specified TXi to its internal flash memory
- `reset(device)`: resets the calibration data for the specified TXi to its factory defaults (no calibration)

new commands requiring my [Telexi firmware fork](https://github.com/evannjohnson/telex-ev):
- ii.txi.get('all'): gets all params and ins at once to reduce ii overhead when polling all params/ins from crow at a fast rate
```lua
-- when using txi.get('all'), receives table where values 1-4 are params 1-4, 5-8 are ins 1-4
ii.txi.event = function(e, data)
    for i=1,8 do
        local handler = txi_poll_handlers[i]
        if handler then
            handler(data[i])
        end
    end
end

txi_poll_handlers = {
    -- param 1
    [1] = function(val)
    end,
    -- param 2
    [2] = function(val)
        print(val)
    end,
    -- param 3
    [3] = function(val)
    end,
    -- param 4
    [4] = function(val)
    end,
    -- in 1
    [5] = function(val)
    end,
    -- in 2
    [6] = function(val)
    end,
    -- in 3
    [7] = function(val)
    end,
    -- in 4
    [8] = function(val)
    end,
}

txi_metro = metro.init{
    time  = 0.2, -- can go to at least 0.002
    count = -1,
    event = function()
        ii.txi.get('all')
    end,
}
```

## telexo
new `ii.txo` commands:
- `m_sync(port, state)`

