-- bitmask definitions for commands
local PARAM = 0x0
local IN    = 0x4
local GETTER_BIT = 0x80 -- separate the read commands for different pickling

local QUANT = 0x8
local N     = 0x10

do return
{ module_name  = 'TXi'
, manufacturer = 'bpc'
, i2c_address  = {0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F}
, lua_name     = 'txi'
, getters =
  { { name = 'param'
    , cmd  = GETTER_BIT | PARAM -- 0x0, 0.0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'param_quant'
    , cmd  = GETTER_BIT | PARAM | QUANT -- 0x8, 8.0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'param_N'
    , cmd  = GETTER_BIT | PARAM | N -- 0x10, 16.0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in'
    , cmd  = GETTER_BIT | IN -- 0x4, 4.0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in_quant'
    , cmd  = GETTER_BIT | IN | QUANT -- 0xC, 12.0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in_N'
    , cmd  = GETTER_BIT | IN | N -- 0x14, 20.0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  -- multi-value getter: returns 8 s16V values in one transaction.
  -- TODO: adjust cmd byte (0x98 below) to match TXi firmware's wire cmd.
  -- pickle hook below has a corresponding exception for this cmd byte.
  , { name = 'all'
    , cmd  = 0x98 -- GETTER_BIT | 0x18; wire-level byte = 0x18 (after pickle)
    , retval = { 'volts', s16V, 8 }
    }
  }
, commands =
    -- ti.param_map requires a call to txi.param_top and txi.param_bot
  { { name = 'param_top'
    , cmd  = 0x14
    , docs = 'maps the PARAM values for input x across the range y - z'
    , args = { { 'channel', s8 }
             , { 'top', s16V }
             }
    }
  , { name = 'param_bot'
    , cmd  = 0x15
    , docs = 'maps the PARAM values for input x across the range y - z'
    , args = { { 'channel', s8 }
             , { 'bottom', s16V }
             }
    }
  , { name = 'param_scale'
    , cmd  = 0x13
    , docs = 'select scale for PARAM knob x'
    , args = { { 'channel', s8 }
             , { 'scale', s8 }
             }
    }
  , { name = 'in_top'
    , cmd  = 0x04
    , docs = 'maps the IN values for input x across the range y - z'
    , args = { { 'channel', s8 }
             , { 'top', s16V }
             }
    }
  , { name = 'in_bot'
    , cmd  = 0x05
    , docs = 'maps the IN values for input x across the range y - z'
    , args = { { 'channel', s8 }
             , { 'bottom', s16V }
             }
    }
  , { name = 'in_scale'
    , cmd  = 0x03
    , docs = 'select scale for input x'
    , args = { { 'channel', s8 }
             , { 'scale', s8 }
             }
    }
  , { name = 'param_calib'
    , cmd  = 0x21
    , docs = 'calibrates scaling for PARAM knob, y of 0 sets bottom bound, y of 1 sets top bound'
    , args = { { 'channel', s8 }
             , { 'y', s8 }
             }
    }
  , { name = 'in_calib'
    , cmd  = 0x20
    , docs = 'calibrates scaling for IN jack, y of -1 sets -10V point, y of 0 sets 0V point, y of 1 sets 10V point'
    , args = { { 'channel', s8 }
             , { 'y', s8 }
             }
    }
  , { name = 'store'
    , cmd  = 0x22
    , docs = 'stores the calibration data for the specified TXi to its internal flash memory'
    , args = { 'device', s8 }
    }
  , { name = 'reset'
    , cmd  = 0x23
    , docs = 'resets the calibration data for the specified TXi to its factory defaults (no calibration)'
    , args = { 'device', s8 }
    }
  }
, pickle = -- combine get command & channel into a single byte & set address
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
[[

if (data[0] >= 128 ) {         // if it's a getter command
  if (data[0] == 0x98) {       // multi-value get: no channel arg
    data[0] &= ~(1 << 7);      // strip GETTER_BIT, send 0x18 on wire
    *byte_count = 1;
  } else {
    uint8_t chan = data[1] - 1;  // zero-index the channel
    data[0] |= (chan & 0x3);     // mask channel
    data[0] &= ~(1 << 7);        // remove GETTER_BIT
    *byte_count = 1;             // packed into a single byte
    *address += chan >> 2;       // ascending vals increment address
  }
} else if (data[0] != 0x22 && data[0] != 0x23) {
  data[1] -= 1;                  // zero-index the channel (store/reset use data[1] as device, not channel)
}

]]
, unpickle = -- using the same command to parse the response from any channel
-- void unpickle( uint8_t* address, uint8_t* command, uint8_t* data );
[[

*command &= ~0x3;      // use same command for all 4 channels (by discarding 2LSBs)
*command |= (1 << 7);  // add GETTER_BIT

]]
}
end
