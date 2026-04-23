#pragma once

#include <stdint.h>

typedef struct output{
    uint8_t channel;
    float* (*modefn)(int channel, float* out, int size);
} Output_t;

void Output_init( int channels );
Output_t* Output_ix_to_p( uint8_t index );
const char* Output_mode_name( uint8_t index );

float* Output_step_v( int index, float* out, int size );

void Output_asl( Output_t* self );
