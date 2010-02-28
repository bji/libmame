/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

/* globals */
extern UINT8 segaic16_display_enable;
extern UINT16 *segaic16_tileram_0;
extern UINT16 *segaic16_textram_0;
extern UINT16 *segaic16_spriteram_0;
extern UINT16 *segaic16_spriteram_1;
extern UINT16 *segaic16_roadram_0;
extern UINT16 *segaic16_rotateram_0;
extern UINT16 *segaic16_paletteram;

/* misc functions */
void segaic16_set_display_enable(running_machine *machine, int enable);

/* palette handling */
void segaic16_palette_init(int entries);
WRITE16_HANDLER( segaic16_paletteram_w );

/* tilemap systems */
#define SEGAIC16_MAX_TILEMAPS		1

#define SEGAIC16_TILEMAP_HANGON		0
#define SEGAIC16_TILEMAP_16A		1
#define SEGAIC16_TILEMAP_16B		2
#define SEGAIC16_TILEMAP_16B_ALT	3

#define SEGAIC16_TILEMAP_FOREGROUND	0
#define SEGAIC16_TILEMAP_BACKGROUND	1
#define SEGAIC16_TILEMAP_TEXT		2

void segaic16_tilemap_init(running_machine *machine, int which, int type, int colorbase, int xoffs, int numbanks);
void segaic16_tilemap_reset(running_machine *machine, int which);
void segaic16_tilemap_draw(running_device *screen, bitmap_t *bitmap, const rectangle *cliprect, int which, int map, int priority, int priority_mark);
void segaic16_tilemap_set_bank(running_machine *machine, int which, int banknum, int offset);
void segaic16_tilemap_set_flip(running_machine *machine, int which, int flip);
void segaic16_tilemap_set_rowscroll(running_machine *machine, int which, int enable);
void segaic16_tilemap_set_colscroll(running_machine *machine, int which, int enable);

WRITE16_HANDLER( segaic16_tileram_0_w );
WRITE16_HANDLER( segaic16_textram_0_w );

/* sprite systems */
#define SEGAIC16_MAX_SPRITES		2

#define SEGAIC16_SPRITES_HANGON		0
#define SEGAIC16_SPRITES_16A		1
#define SEGAIC16_SPRITES_16B		2
#define SEGAIC16_SPRITES_SHARRIER	3
#define SEGAIC16_SPRITES_OUTRUN		4
#define SEGAIC16_SPRITES_XBOARD		5
#define SEGAIC16_SPRITES_YBOARD		6
#define SEGAIC16_SPRITES_YBOARD_16B	7
#define SEGAIC16_SPRITES_16A_BOOTLEG_WB3BL 8
#define SEGAIC16_SPRITES_16A_BOOTLEG_PASSHTBL 9
#define SEGAIC16_SPRITES_16A_BOOTLEG_SHINOBLD 10

void segaic16_sprites_init(running_machine *machine, int which, int type, int colorbase, int xoffs);
void segaic16_sprites_draw(running_device *screen, bitmap_t *bitmap, const rectangle *cliprect, int which);
void segaic16_sprites_set_bank(running_machine *machine, int which, int banknum, int offset);
void segaic16_sprites_set_flip(running_machine *machine, int which, int flip);
void segaic16_sprites_set_shadow(running_machine *machine, int which, int shadow);
WRITE16_HANDLER( segaic16_sprites_draw_0_w );
WRITE16_HANDLER( segaic16_sprites_draw_1_w );

/* road systems */
#define SEGAIC16_MAX_ROADS			1

#define SEGAIC16_ROAD_HANGON		0
#define SEGAIC16_ROAD_SHARRIER		1
#define SEGAIC16_ROAD_OUTRUN		2
#define SEGAIC16_ROAD_XBOARD		3

#define SEGAIC16_ROAD_BACKGROUND	0
#define SEGAIC16_ROAD_FOREGROUND	1

void segaic16_road_init(running_machine *machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);
void segaic16_road_draw(int which, bitmap_t *bitmap, const rectangle *cliprect, int priority);
READ16_HANDLER( segaic16_road_control_0_r );
WRITE16_HANDLER( segaic16_road_control_0_w );

/* rotation systems */
#define SEGAIC16_MAX_ROTATE			1

#define SEGAIC16_ROTATE_YBOARD		0

void segaic16_rotate_init(running_machine *machine, int which, int type, int colorbase);
void segaic16_rotate_draw(running_machine *machine, int which, bitmap_t *bitmap, const rectangle *cliprect, bitmap_t *srcbitmap);
READ16_HANDLER( segaic16_rotate_control_0_r );
