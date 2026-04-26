#pragma once

#include <stdint.h>
#include <stdbool.h>

// need an Init() fn. send SR as an argument
#define SAMPLE_RATE 48000
#define iSAMPLE_RATE (1.0/(float)SAMPLE_RATE)
#define SAMPLES_PER_MS ((float)SAMPLE_RATE/1000.0)
#define iSAMPLES_PER_MS (1.0/(float)SAMPLES_PER_MS)
#define SPINNER_CLOCK_K 1.0e-3f // constant for clock sync, higher = quicker/jitterier lock

#define SPINNER_CHANNELS 4
void Spinner_init( int channels );

typedef struct{
    int         index;

    // state
    float bottom; // bottom voltage of range
    float top; // top voltage of range
    float time; // ms for one spin
    int direction; // -1 for down ramp, 1 for up ramp, 0 for stopped
    float pos; // 0-1
    float phase_offset; // 0-1
    bool clocked;
    bool clocked_keep_offset; // if set, the spinner will maintain its current position when becoming clocked or changing speed/direction while clocked. If unset, the spinner will lock onto the beat so that pos crosses 0/1 right on the set clock division
    float clock_div_inverse; // internal calculation uses inverse of clock_div

    // internal
    float delta_free;
    float range;
    float sync_offset; // when becoming synced, or changing direction/division while synced, maintain position. Only used when `clocked_keep_offset` is set
} Spinner_t;

float Spinner_get_bottom( int index );
float Spinner_get_top( int index );
float Spinner_get_time( int index );
int Spinner_get_direction( int index );
float Spinner_get_pos( int index );
float Spinner_get_phase_offset( int index );
bool Spinner_get_clocked( int index );
bool Spinner_get_clocked_keep_offset( int index );
float Spinner_get_clock_div( int index );

void Spinner_set_bottom( int index, float v );
void Spinner_set_top( int index, float v );
void Spinner_set_time( int index, float ms );
void Spinner_set_direction( int index, int direction );
void Spinner_set_pos( int index, float pos );
void Spinner_set_phase_offset( int index, float offset );
void Spinner_set_clocked( int index, bool clocked );
void Spinner_set_clocked_keep_offset( int index, bool clocked_keep_offset );
void Spinner_set_clock_div( int index, float clock_div );

void Spinner_delta_pos( int index, float delta);
void Spinner_delta_phase_offset( int index, float delta);

float* Spinner_step_v( int    index
                     , float* out
                     , int    size
                     );

