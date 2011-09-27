#define GAL_AUDIO	"discrete"

MACHINE_CONFIG_EXTERN( mooncrst_audio );
MACHINE_CONFIG_EXTERN( galaxian_audio );

WRITE8_DEVICE_HANDLER( galaxian_sound_w );
WRITE8_DEVICE_HANDLER( galaxian_pitch_w );
WRITE8_DEVICE_HANDLER( galaxian_vol_w );
WRITE8_DEVICE_HANDLER( galaxian_noise_enable_w );
WRITE8_DEVICE_HANDLER( galaxian_background_enable_w );
WRITE8_DEVICE_HANDLER( galaxian_shoot_enable_w );
WRITE8_DEVICE_HANDLER( galaxian_lfo_freq_w );

