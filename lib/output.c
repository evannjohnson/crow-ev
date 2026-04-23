#include "output.h"
#include "slopes.h"
#include "casl.h"

#include <stdlib.h>
#include <stdio.h>

static uint8_t channel_count = 0;

static Output_t* selves = NULL;

///////////////////////////////////////////
// init

void Output_init( int channels )
{
    for(int i=0; i<SLOPE_CHANNELS; i++){
        casl_init(i);
    }
    S_init( SLOPE_CHANNELS );

    channel_count = channels;
    selves = malloc( sizeof ( Output_t ) * channels );
    for( int j=0; j<channels; j++ ){
        selves[j].channel = j;
        Output_asl( &(selves[j]) ); // init to ASL for backwards compat
    }
}

void Output_deinit( void )
{
    free(selves); selves = NULL;
}

/////////////////////////////////////////
// global helpers

Output_t* Output_ix_to_p( uint8_t index )
{
    if( index < 0 || index >= channel_count ){ return NULL; } // TODO error msg
    return &(selves[index]);
}

const char* Output_mode_name( uint8_t index )
{
    Output_t* o = Output_ix_to_p(index);
    if( ! o ) { return "invalid index"; }
    // use the modefn to determine the mode
    if ( o->modefn == S_step_v ) {
        return "asl";
    } else {
        return "unknown";
    }
}

//////////////////////////////////////////
// mode configuration
void Output_asl( Output_t* self )
{
    self->modefn = S_step_v; 
}

//////////////////////////////////////////////
// signal processing
float* Output_step_v( int index, float* out, int size )
{
    Output_t* self = Output_ix_to_p( index );
    return (*self->modefn)( index, out, size );
}
