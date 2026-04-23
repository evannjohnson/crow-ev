#include "spinner.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "stm32f7xx.h"

#include "submodules/wrDsp/wrBlocks.h"

////////////////////////////////
// global vars

static uint8_t spinner_count = 0;
static Spinner_t* selves = NULL;

////////////////////////////////
// public definitions

void Spinner_init( int channels )
{
    spinner_count = channels;
    selves = malloc( sizeof( Spinner_t ) * channels );
    if( !selves ){ printf("spinners malloc failed\n"); return; }
    for( int j=0; j<channels; j++ ){
        selves[j].index  = j;

        selves[j].bottom  = -5;
        selves[j].top  = 5;
        selves[j].time  = 1000;
        selves[j].direction  = 0;
        selves[j].pos  = 0;
        selves[j].phase_offset = 0;

        selves[j].delta  = iSAMPLES_PER_MS / selves[j].time;
        selves[j].range  = selves[j].top - selves[j].bottom;
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
    self->delta = iSAMPLES_PER_MS / ms;
}
void Spinner_set_direction( int index, int direction )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->direction = direction;
}
void Spinner_set_pos( int index, float pos )
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    self->pos = pos - floorf(pos); // keep in range 0-1
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
}
void Spinner_delta_phase_offset( int index, float delta)
{
    if( index < 0 || index >= SPINNER_CHANNELS ){ return; }
    Spinner_t* self = &selves[index]; // safe pointer
    float deltad = self->phase_offset + delta;
    self->phase_offset = deltad - floorf(deltad); // keep in range 0-1
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
    float delta = self->delta * self->direction;
    float phase_offset = self->phase_offset;
    float bottom = self->bottom;
    float range = self->range;

    // *out2++ = self->here + self->delta;
    for( int i=0; i<size; i++ ){
        pos += delta;
        if (pos > 1.0f) {
            pos -= 1.0f;
        } else if (pos < 0.0f) {
            pos += 1.0f;
        }
        float pos_w_offset = pos + phase_offset;
        pos_w_offset -= floorf(pos_w_offset); // keep in range 0-1
        *out2++ = bottom + pos_w_offset * range;
    }

    self->pos = pos;
    return out;
}
