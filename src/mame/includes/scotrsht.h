class scotrsht_state : public driver_device
{
public:
	scotrsht_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"){ }

	int m_irq_enable;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll;
	tilemap_t *m_bg_tilemap;
	int m_charbank;
	int m_palette_bank;
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_WRITE8_MEMBER(scotrsht_soundlatch_w);
	DECLARE_WRITE8_MEMBER(scotrsht_videoram_w);
	DECLARE_WRITE8_MEMBER(scotrsht_colorram_w);
	DECLARE_WRITE8_MEMBER(scotrsht_charbank_w);
	DECLARE_WRITE8_MEMBER(scotrsht_palettebank_w);
};


/*----------- defined in video/scotrsht.c -----------*/


PALETTE_INIT( scotrsht );
VIDEO_START( scotrsht );
SCREEN_UPDATE_IND16( scotrsht );
