/*************************************************************************

    Targ hardware

*************************************************************************/


/*----------- defined in audio/targ.c -----------*/

WRITE8_HANDLER( targ_audio_1_w );
WRITE8_HANDLER( targ_audio_2_w );
WRITE8_HANDLER( spectar_audio_2_w );

MACHINE_CONFIG_EXTERN( spectar_audio );
MACHINE_CONFIG_EXTERN( targ_audio );

