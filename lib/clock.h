#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum{ CLOCK_SOURCE_INTERNAL = 0
            , CLOCK_SOURCE_MIDI     = 1
            , CLOCK_SOURCE_LINK     = 2
            , CLOCK_SOURCE_CROW     = 3
            , CLOCK_SOURCE_LIST_LENGTH
} clock_source_t;

typedef struct{
    // clock_update_reference is called every beat, so we're constantly updating beat & last_beat_time
    double beat; // set in clock_update_reference(beat,_). this is the refence beat (ie. count since start)
    double last_beat_time; // seconds_since_boot at last clock_update_reference() call
    double beat_duration_inverse; // inverse of beat_duration
    float  beat_duration; // seconds_per_beat (ie tempo)
} clock_reference_t;

void clock_init( int max_clocks );

// MUST ONLY BE CALLED WHEN time_now CHANGES
// Designed to be called on a 1ms tick.
void clock_update(uint32_t time_now);

bool clock_schedule_resume_sleep( int coro_id, float seconds );
bool clock_schedule_resume_sync( int coro_id, float beats, float sync_beat_offset );
bool clock_schedule_resume_beatsync( int coro_id, float beats );
void clock_update_reference( double beats, double beat_duration );
void clock_update_reference_from( double beats, double beat_duration, clock_source_t source);
void clock_start_from( clock_source_t source );
void clock_stop_from( clock_source_t source );
void clock_set_source( clock_source_t source );

// for linking to lua
float clock_get_time_beats(void);
// for C layer access
double clock_get_time_beats_d(void);
double clock_get_time_seconds(void);
float clock_get_tempo(void);
// other libs can use this to stay synced to clock
const clock_reference_t* clock_get_reference(void);

void clock_cancel_coro( int coro_id );
void clock_cancel_coro_all( void );


///////////////////////////////////
// internal

void clock_internal_init(void);
void clock_internal_set_tempo( float bpm );
void clock_internal_start( float new_beat, bool transport_start );
void clock_internal_stop(void);


///////////////////////////////////
// input

// TODO add arg to choose input channel
void clock_crow_init(void);
void clock_input_handler( int id, float freq ); // Called from Detect lib
void clock_crow_handle_clock(void);
void clock_crow_in_div( float div );
double clock_get_crow_last_time(void);
