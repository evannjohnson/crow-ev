#include "spinner.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "clock.h"
#include "stm32f7xx.h"

#include "submodules/wrDsp/wrBlocks.h"

////////////////////////////////
// global vars

static uint8_t spinner_count = 0;
static Spinner_t* selves = NULL;

////////////////////////////////
// private declarations

static void capture_sync_offset( Spinner_t* self );

////////////////////////////////
// public definitions

void Spinner_init( int channels )
{
    spinner_count = channels;
    selves = malloc( sizeof( Spinner_t ) * channels );
    if( !selves ){ printf("spinners malloc failed\n"); return; }
    for( int j=0; j<channels; j++ ){
        selves[j].index  = j;

        selves[j].bottom = -5.0f;
        selves[j].top = 5.0f;
        selves[j].time = 1000.0f;
        selves[j].direction = 0;
        selves[j].pos = 0.5f; // middle of range
        selves[j].phase_offset = 0.0f;
        selves[j].clocked = false;
        selves[j].clocked_keep_offset = true;
        selves[j].clock_div_inverse = 1;

        selves[j].delta_free = iSAMPLES_PER_MS / selves[j].time;
        selves[j].range = selves[j].top - selves[j].bottom;
        selves[j].sync_offset = 0.0f;
    }
}

float Spinner_get_bottom( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->bottom;
}
float Spinner_get_top( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->top;
}
float Spinner_get_time( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->time;
}
int Spinner_get_direction( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->direction;
}
float Spinner_get_pos( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->pos;
}
float Spinner_get_phase_offset( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->phase_offset;
}
bool Spinner_get_clocked( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->clocked;
}
bool Spinner_get_clocked_keep_offset( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return self->clocked_keep_offset;
}
float Spinner_get_clock_div( int index )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return 0.0; }
    Spinner_t* self = &selves[index]; // safe pointer
    return 1.0f / self->clock_div_inverse;
}

void Spinner_set_bottom( int index, float v )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->bottom = v;
    self->range = self->top - self->bottom;
}
void Spinner_set_top( int index, float v )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->top = v;
    self->range = self->top - self->bottom;
}
void Spinner_set_time( int index, float ms )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->time = ms;
    self->delta_free = iSAMPLES_PER_MS / ms;
}
void Spinner_set_direction( int index, int direction )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    bool need_sync_offset = self->clocked
                         && self->clocked_keep_offset
                         && direction != self->direction;

    self->direction = direction;
    if(need_sync_offset){ capture_sync_offset(self); }
}
void Spinner_set_pos( int index, float pos )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->pos = pos - floorf(pos); // keep in range 0-1
    if (self->clocked && self->clocked_keep_offset){
        capture_sync_offset(self);
    }
}
void Spinner_set_phase_offset( int index, float offset )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->phase_offset = offset - floorf(offset); // keep in range 0-1
}

void Spinner_delta_pos( int index, float delta)
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    float deltad = self->pos + delta;
    self->pos = deltad - floorf(deltad); // keep in range 0-1
    if (self->clocked && self->clocked_keep_offset){
        capture_sync_offset(self);
    }
}
void Spinner_delta_phase_offset( int index, float delta)
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    float deltad = self->phase_offset + delta;
    self->phase_offset = deltad - floorf(deltad); // keep in range 0-1
}
void Spinner_set_clocked( int index, bool clocked )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->clocked = clocked;
    if (clocked && self->clocked_keep_offset){
        capture_sync_offset(self);
    }
}
void Spinner_set_clocked_keep_offset( int index, bool clocked_keep_offset )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->clocked_keep_offset = clocked_keep_offset;
    if (!clocked_keep_offset){
        self->sync_offset = 0.0f;
    } else if (self->clocked){
        capture_sync_offset(self);
    }
}
void Spinner_set_clock_div( int index, float clock_div )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    float clock_div_inverse = 1.0f / clock_div;
    bool need_sync_offset = self->clocked
                         && self->clocked_keep_offset
                         && clock_div_inverse != self->clock_div_inverse;

    self->clock_div_inverse = clock_div_inverse;
    if (need_sync_offset){
        capture_sync_offset(self);
    }
}

float* Spinner_step_v( int     index
                     , float*  out
                     , int     size
                     )
{
    // turn index into pointer
    if( index < 0 || index >= SPINNER_CHANNELS ){ return out; }
    Spinner_t* self = &selves[index]; // safe pointer

    float* out2 = out;
    float pos = self->pos;
    float phase_offset = self->phase_offset;
    float bottom = self->bottom;
    float range = self->range;
    float direction = self->direction;

    if (direction != 0) {
        float delta;
        if (self->clocked) {
            const clock_reference_t* clock_ref = clock_get_reference();
            delta = self->clock_div_inverse * (float)clock_ref->beat_duration_inverse * iSAMPLE_RATE;
            double d = clock_get_time_beats_d()
                     * (double)self->clock_div_inverse
                     * (double)direction;
            float target_pos = (float)(d - floor(d));
            target_pos += self->sync_offset;
            target_pos -= floorf(target_pos);

            float err = pos - target_pos;
            err *= direction; // err positive is too fast, negative is too slow
            if (err >  0.5f){ err -= 1.0f; }
            else if (err < -0.5f){ err += 1.0f; }

            delta -= err * SPINNER_CLOCK_K;
        } else {
            delta = self->delta_free;
            // prevent backwards movement, clamp to delta 0 until catchup
            delta = fmaxf(delta, 0.0f);
        }
        delta = delta * direction;

        // *out2++ = self->here + self->delta;
        for( int i=0; i<size; i++ ) {
            pos += delta;
            if (pos > 1.0f) {
                pos -= 1.0f;
            } else if (pos < 0.0f) {
                pos += 1.0f;
            }
            float pos_w_offset = pos + phase_offset;
            // possible optimization: -= (float)(pos_w_offset >= 1.0f);
            pos_w_offset -= floorf(pos_w_offset); // keep in range 0-1
            *out2++ = bottom + pos_w_offset * range;
        }
    } else {
        float pos_w_offset = pos + phase_offset;
        pos_w_offset -= floorf(pos_w_offset); // keep in range 0-1
        float v = bottom + pos_w_offset * range;

        for( int i=0; i<size; i++) {
            *out2++ = v;
        }

        return out;
    }

    self->pos = pos;
    return out;
}

// internal
static void capture_sync_offset(Spinner_t* self)
{
    double d = clock_get_time_beats_d()
             * (double)self->clock_div_inverse
             * (double)self->direction;       // matches the target_pos formula
    float clock_phase = (float)(d - floor(d));
    float off = self->pos - clock_phase;
    self->sync_offset = off - floorf(off);    // wrap into [0,1)
}
