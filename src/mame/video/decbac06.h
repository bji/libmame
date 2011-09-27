/* BAC06 */


class deco_bac06_device_config : public device_config
{
	friend class deco_bac06_device;
	deco_bac06_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
	static void set_gfx_region_wide(device_config *device, int region8x8, int region16x16, int wide);

protected:
	UINT8 m_gfxregion8x8;
	UINT8 m_gfxregion16x16;
	int m_wide;
};

class deco_bac06_device : public device_t
{
	friend class deco_bac06_device_config;
	deco_bac06_device(running_machine &_machine, const deco_bac06_device_config &config);
public:
	void set_gfxregion(int region8x8, int region16x16) { m_gfxregion8x8 = region8x8; m_gfxregion16x16 = region16x16; };


	UINT16* pf_data;
	UINT16* pf_rowscroll, *pf_colscroll;

	tilemap_t* pf8x8_tilemap[3];
	tilemap_t* pf16x16_tilemap[3];
	int	   tile_region;
	void create_tilemaps(int region8x8,int region16x16);
	UINT16 pf_control_0[8];
	UINT16 pf_control_1[8];

	void deco_bac06_pf_draw(running_machine &machine,bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT16 penmask, UINT16 pencondition,UINT16 colprimask, UINT16 colpricondition);
	void deco_bac06_pf_draw_bootleg(running_machine &machine,bitmap_t *bitmap,const rectangle *cliprect,int flags, int mode, int type);

	UINT8 get_flip_state(void) { return pf_control_0[0]&0x80; };
	void set_colmask(int data) { m_gfxcolmask = data; }
	void set_bppmultmask( int mult, int mask ) { m_bppmult = mult; m_bppmask = mask; } // stadium hero has 3bpp tiles
	UINT8 m_gfxcolmask;
	int m_rambank; // external connection?

protected:
	virtual void device_start();
	virtual void device_reset();
	const deco_bac06_device_config &m_config;

	UINT8 m_gfxregion8x8;
	UINT8 m_gfxregion16x16;
	int m_wide;

	UINT8 m_bppmult;
	UINT8 m_bppmask;

	void custom_tilemap_draw(running_machine &machine,
							bitmap_t *bitmap,
							const rectangle *cliprect,
							tilemap_t *tilemap_ptr,
							const UINT16 *rowscroll_ptr,
							const UINT16 *colscroll_ptr,
							const UINT16 *control0,
							const UINT16 *control1,
							int flags,
							UINT16 penmask, UINT16 pencondition,UINT16 colprimask, UINT16 colpricondition);

private:


};

/* 16-bit accessors */

WRITE16_DEVICE_HANDLER( deco_bac06_pf_control_0_w );
READ16_DEVICE_HANDLER( deco_bac06_pf_control_1_r );
WRITE16_DEVICE_HANDLER( deco_bac06_pf_control_1_w );

WRITE16_DEVICE_HANDLER( deco_bac06_pf_data_w );
READ16_DEVICE_HANDLER( deco_bac06_pf_data_r );
WRITE16_DEVICE_HANDLER( deco_bac06_pf_rowscroll_w );
READ16_DEVICE_HANDLER( deco_bac06_pf_rowscroll_r );
WRITE16_DEVICE_HANDLER( deco_bac06_pf_colscroll_w );
READ16_DEVICE_HANDLER( deco_bac06_pf_colscroll_r );

/* 8-bit accessors */

/* for dec8.c, pcktgal.c */
READ8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_r );
WRITE8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_w );

WRITE8_DEVICE_HANDLER( deco_bac06_pf_control0_8bit_w );
READ8_DEVICE_HANDLER( deco_bac06_pf_control1_8bit_r );
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control1_8bit_w );

READ8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_r );
WRITE8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_w );

/* for hippodrm (dec0.c) and actfancr / triothep (H6280 based games)*/
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control0_8bit_packed_w );
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control1_8bit_swap_w );
READ8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_swap_r );
WRITE8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_swap_w );
READ8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_swap_r );
WRITE8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_swap_w );

const device_type deco_bac06_ = deco_bac06_device_config::static_alloc_device_config;


