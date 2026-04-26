local Spinner = {}

function Spinner.new(chan)
    local s = { chan = chan or 1 }
    s.getters = {
        clocked = true
    }
    setmetatable(s, Spinner)
    return s
end

function Spinner:delta_pos(delta)
    spinner_delta_pos(self.chan, delta)
end

function Spinner:delta_phase_offset(delta)
    spinner_delta_phase_offset(self.chan, delta)
end

-- metatables
-- getters
Spinner.__index = function(self, ix)
    local method = rawget(Spinner, ix)
    if method then return method end

    if ix == "bottom" then
        return spinner_get_bottom(self.chan)
    elseif ix == "top" then
        return spinner_get_top(self.chan)
    elseif ix == "time" then
        return spinner_get_time(self.chan)
    elseif ix == "direction" then
        return spinner_get_direction(self.chan)
    elseif ix == "pos" then
        return spinner_get_pos(self.chan)
    elseif ix == "phase_offset" then
        return spinner_get_phase_offset(self.chan)
    elseif ix == "clocked" then
        return spinner_get_clocked(self.chan)
    elseif ix == "clocked_keep_offset" then
        return spinner_get_clocked_keep_offset(self.chan)
    elseif ix == "spinner_clock_div" then
        return spinner_get_clock_div(self.chan)
    end
end

-- setters
Spinner.__newindex = function(self, ix, val)
    if ix == "bottom" then
        if val < -5 then
            print("error: cannot set bottom voltage < -5")
        elseif val >= self.top then
            print("error: bottom voltage must be < top voltage")
        end
        spinner_set_bottom(self.chan, val)
    elseif ix == "top" then
        if val > 10 then
            print("error: cannot set top voltage > 10")
            return
        elseif val <= self.bottom then
            print("error: top voltage must be > bottom voltage")
            return
        end
        spinner_set_top(self.chan, val)
    elseif ix == "time" then
        if val < 1 or 10000000 < val then
            print("error: time must be between 1 and 10000000 ms")
            return
        end
        spinner_set_time(self.chan, val)
    elseif ix == "direction" then
        if val ~= -1 and val ~= 0 and val ~= 1 then
            print("error: direction must be -1, 0, or 1")
            return
        end
        spinner_set_direction(self.chan, val)
    elseif ix == "pos" then
        spinner_set_pos(self.chan, val)
    elseif ix == "phase_offset" then
        spinner_set_phase_offset(self.chan, val)
    elseif ix == "clocked" then
        spinner_set_clocked(self.chan, val)
    elseif ix == "clocked_keep_offset" then
        spinner_set_clocked_keep_offset(self.chan, val)
    elseif ix == "spinner_clock_div" then
        if not (val > 0) then
            print("error: clock_div must by > 0")
        end
        spinner_set_clock_div(self.chan, val)
    end
end

setmetatable(Spinner, Spinner)
return Spinner
