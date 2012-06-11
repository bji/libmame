#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

#define RELATIVE(x, y)	((((x) >> 3) - (y)) << 3) | (x & 7);

void N64TexturePipeT::SetMachine(running_machine &machine)
{
	n64_state *state = machine.driver_data<n64_state>();

	m_rdp = state->m_rdp;

	for(int i = 0; i < 0x10000; i++)
	{
		Color c;
		c.i.r = m_rdp->ReplicatedRGBA[(i >> 11) & 0x1f];
		c.i.g = m_rdp->ReplicatedRGBA[(i >>  6) & 0x1f];
		c.i.b = m_rdp->ReplicatedRGBA[(i >>  1) & 0x1f];
		c.i.a = (i & 1) ? 0xff : 0x00;
		Expand16To32Table[i] = c.c;
	}
}

void N64TexturePipeT::Mask(INT32* S, INT32* T, INT32 num, const rdp_poly_state& object)
{
	const N64Tile* tile = object.m_tiles;

	if (tile[num].mask_s)
	{
		INT32 wrap = *S >> (tile[num].mask_s > 10 ? 10 : tile[num].mask_s);
		wrap &= 1;
		if (tile[num].ms && wrap)
		{
			*S = (~(*S));
		}
		*S &= m_maskbits_table[tile[num].mask_s];
	}

	if (tile[num].mask_t)
	{
		INT32 wrap = *T >> (tile[num].mask_t > 10 ? 10 : tile[num].mask_t);
		wrap &= 1;
		if (tile[num].mt && wrap)
		{
			*T = (~(*T));
		}
		*T &= m_maskbits_table[tile[num].mask_t];
	}
}

void N64TexturePipeT::MaskCoupled(INT32* S, INT32* S1, INT32* T, INT32* T1, INT32 num, const rdp_poly_state& object)
{
	const N64Tile* tile = object.m_tiles;

	if (tile[num].mask_s)
	{
		INT32 maskbits_s = m_maskbits_table[tile[num].mask_s];
		if (tile[num].ms)
		{
			INT32 swrapthreshold = tile[num].mask_s > 10 ? 10 : tile[num].mask_s;
			INT32 wrap = (*S >> swrapthreshold) & 1;
			INT32 wrap1 = (*S1 >> swrapthreshold) & 1;
			if (wrap)
			{
				*S = (~(*S));
			}
			if (wrap1)
			{
				*S1 = (~(*S1));
			}
		}
		*S &= maskbits_s;
		*S1 &= maskbits_s;
	}

	if (tile[num].mask_t)
	{
		INT32 maskbits_t = m_maskbits_table[tile[num].mask_t];
		if (tile[num].mt)
		{
			INT32 twrapthreshold = tile[num].mask_t > 10 ? 10 : tile[num].mask_t;
			INT32 wrap = (*T >> twrapthreshold) & 1;
			INT32 wrap1 = (*T1 >> twrapthreshold) & 1;
			if (wrap)
			{
				*T = (~(*T));
			}
			if (wrap1)
			{
				*T1 = (~(*T1));
			}
		}
		*T &= maskbits_t;
		*T1 &= maskbits_t;
	}
}

void N64TexturePipeT::ShiftCycle(INT32* S, INT32* T, INT32* maxs, INT32* maxt, UINT32 num, const rdp_poly_state& object)
{
	const N64Tile* tile = object.m_tiles;
	*S = SIGN16(*S);
	*T = SIGN16(*T);
	if (tile[num].shift_s < 11)
	{
		*S >>= tile[num].shift_s;
	}
	else
	{
		*S <<= (16 - tile[num].shift_s);
	}
	*S = SIGN16(*S);
	if (tile[num].shift_t < 11)
	{
		*T >>= tile[num].shift_t;
	}
    else
    {
		*T <<= (16 - tile[num].shift_t);
	}
	*T = SIGN16(*T);

	*maxs = ((*S >> 3) >= tile[num].sh);
	*maxt = ((*T >> 3) >= tile[num].th);
}

void N64TexturePipeT::ShiftCopy(INT32* S, INT32* T, UINT32 num, const rdp_poly_state& object)
{
	const N64Tile* tile = object.m_tiles;
	*S = SIGN16(*S);
	*T = SIGN16(*T);
	if (tile[num].shift_s < 11)//?-? tcu_tile
	{
		*S >>= tile[num].shift_s;
	}
	else
	{
		*S <<= (16 - tile[num].shift_s);
	}
	*S = SIGN16(*S);
	if (tile[num].shift_t < 11)
	{
		*T >>= tile[num].shift_t;
	}
    else
    {
		*T <<= (16 - tile[num].shift_t);
	}
	*T = SIGN16(*T);
}

void N64TexturePipeT::ClampCycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, INT32 num, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff)
{
	const N64Tile* tile = object.m_tiles;
	int dos = tile[num].cs || !tile[num].mask_s;
	int dot = tile[num].ct || !tile[num].mask_t;

	if (dos)
	{
		if (*S & 0x10000)
		{
			*S = 0;
			*SFRAC = 0;
		}
		else if (maxs)
		{
			*S = m_clamp_s_diff[num];
			*SFRAC = 0;
		}
		else
		{
			*S = (SIGN17(*S) >> 5) & 0x1fff;
		}
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

	if (dot)
	{
		if (*T & 0x10000)
		{
			*T = 0;
			*TFRAC = 0;
		}
		else if (maxt)
		{
			*T = m_clamp_t_diff[num];
			*TFRAC = 0;
		}
		else
		{
			*T = (SIGN17(*T) >> 5) & 0x1fff;
		}
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

void N64TexturePipeT::ClampCycleLight(INT32* S, INT32* T, bool maxs, bool maxt, INT32 num, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff)
{
	const N64Tile* tile = object.m_tiles;
	int dos = tile[num].cs || !tile[num].mask_s;
	int dot = tile[num].ct || !tile[num].mask_t;

	if (dos)
	{
		if (*S & 0x10000)
		{
			*S = 0;
		}
		else if (maxs)
		{
			*S = m_clamp_s_diff[num];
		}
		else
		{
			*S = (SIGN17(*S) >> 5) & 0x1fff;
		}
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

	if (dot)
	{
		if (*T & 0x10000)
		{
			*T = 0;
		}
		else if (maxt)
		{
			*T = m_clamp_t_diff[num];
		}
		else
		{
			*T = (SIGN17(*T) >> 5) & 0x1fff;
		}
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

void N64TexturePipeT::Cycle(Color* TEX, Color* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff)
{
	const N64Tile* tile = object.m_tiles;

#define TRELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);
	INT32 bilerp = cycle ? object.OtherModes.bi_lerp1 : object.OtherModes.bi_lerp0;
	int convert = object.OtherModes.convert_one && cycle;
	Color t0;
	Color t1;
	Color t2;
	Color t3;
	if (object.OtherModes.sample_type)
	{
		int sss1, sst1, sss2, sst2;

		INT32 invsf = 0;
		INT32 invtf = 0;
		int center = 0;

		sss1 = SSS;
		sst1 = SST;

		INT32 maxs;
		INT32 maxt;

		ShiftCycle(&sss1, &sst1, &maxs, &maxt, tilenum, object);

		sss1 = TRELATIVE(sss1, tile[tilenum].sl);
		sst1 = TRELATIVE(sst1, tile[tilenum].tl);

		INT32 sfrac = sss1 & 0x1f;
		INT32 tfrac = sst1 & 0x1f;

		ClampCycle(&sss1, &sst1, &sfrac, &tfrac, maxs, maxt, tilenum, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

		sss2 = sss1 + 1;
		sst2 = sst1 + 1;

		MaskCoupled(&sss1, &sss2, &sst1, &sst2, tilenum, object);

		bool upper = ((sfrac + tfrac) >= 0x20);

		if (upper)
		{
			invsf = 0x20 - sfrac;
			invtf = 0x20 - tfrac;
		}

		center = (sfrac == 0x10) && (tfrac == 0x10) && object.OtherModes.mid_texel;

		invsf <<= 3;
		invtf <<= 3;
		sfrac <<= 3;
		tfrac <<= 3;

		t0.c = Fetch(sss1, sst1, tilenum, object, userdata);

		if (bilerp)
		{
			t1.c = Fetch(sss2, sst1, tilenum, object, userdata);
			t2.c = Fetch(sss1, sst2, tilenum, object, userdata);
			t3.c = Fetch(sss2, sst2, tilenum, object, userdata);
			if (!center)
			{
				if (upper)
				{
					TEX->i.r = t3.i.r + (((invsf * (t2.i.r - t3.i.r)) + (invtf * (t1.i.r - t3.i.r)) + 0x80) >> 8);
					TEX->i.g = t3.i.g + (((invsf * (t2.i.g - t3.i.g)) + (invtf * (t1.i.g - t3.i.g)) + 0x80) >> 8);
					TEX->i.b = t3.i.b + (((invsf * (t2.i.b - t3.i.b)) + (invtf * (t1.i.b - t3.i.b)) + 0x80) >> 8);
					TEX->i.a = t3.i.a + (((invsf * (t2.i.a - t3.i.a)) + (invtf * (t1.i.a - t3.i.a)) + 0x80) >> 8);
				}
				else
				{
					TEX->i.r = t0.i.r + (((sfrac * (t1.i.r - t0.i.r)) + (tfrac * (t2.i.r - t0.i.r)) + 0x80) >> 8);
					TEX->i.g = t0.i.g + (((sfrac * (t1.i.g - t0.i.g)) + (tfrac * (t2.i.g - t0.i.g)) + 0x80) >> 8);
					TEX->i.b = t0.i.b + (((sfrac * (t1.i.b - t0.i.b)) + (tfrac * (t2.i.b - t0.i.b)) + 0x80) >> 8);
					TEX->i.a = t0.i.a + (((sfrac * (t1.i.a - t0.i.a)) + (tfrac * (t2.i.a - t0.i.a)) + 0x80) >> 8);
				}
				TEX->i.r &= 0x1ff;
				TEX->i.g &= 0x1ff;
				TEX->i.b &= 0x1ff;
				TEX->i.a &= 0x1ff;
			}
			else
			{
				TEX->i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
				TEX->i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
				TEX->i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
				TEX->i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
			}
		}
		else
		{
			INT32 newk0 = SIGN9(m_rdp->GetK0());
			INT32 newk1 = SIGN9(m_rdp->GetK1());
			INT32 newk2 = SIGN9(m_rdp->GetK2());
			INT32 newk3 = SIGN9(m_rdp->GetK3());
			INT32 invk0 = ~newk0;
			INT32 invk1 = ~newk1;
			INT32 invk2 = ~newk2;
			INT32 invk3 = ~newk3;
			if (convert)
			{
				t0 = *prev;
			}
			t0.i.r = SIGN9(t0.i.r); t0.i.g = SIGN9(t0.i.g); t0.i.b = SIGN9(t0.i.b);
			TEX->i.r = t0.i.b + (((newk0 - invk0) * t0.i.g + 0x80) >> 8);
			TEX->i.g = t0.i.b + (((newk1 - invk1) * t0.i.r + (newk2 - invk2) * t0.i.g + 0x80) >> 8);
			TEX->i.b = t0.i.b + (((newk3 - invk3) * t0.i.r + 0x80) >> 8);
			TEX->i.a = t0.i.b;
			TEX->i.r &= 0x1ff;
			TEX->i.g &= 0x1ff;
			TEX->i.b &= 0x1ff;
			TEX->i.a &= 0x1ff;
		}
	}
	else
	{
		INT32 sss1 = SSS;
		INT32 sst1 = SST;

		INT32 maxs;
		INT32 maxt;

		ShiftCycle(&sss1, &sst1, &maxs, &maxt, tilenum, object);
		sss1 = TRELATIVE(sss1, tile[tilenum].sl);
		sst1 = TRELATIVE(sst1, tile[tilenum].tl);

		ClampCycleLight(&sss1, &sst1, maxs, maxt, tilenum, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

        Mask(&sss1, &sst1, tilenum, object);

		t0.c = Fetch(sss1, sst1, tilenum, object, userdata);
		if (bilerp)
		{
			*TEX = t0;
		}
		else
		{
			INT32 newk0 = SIGN9(m_rdp->GetK0());
			INT32 newk1 = SIGN9(m_rdp->GetK1());
			INT32 newk2 = SIGN9(m_rdp->GetK2());
			INT32 newk3 = SIGN9(m_rdp->GetK3());
			INT32 invk0 = ~newk0;
			INT32 invk1 = ~newk1;
			INT32 invk2 = ~newk2;
			INT32 invk3 = ~newk3;
			if (convert)
			{
				t0 = *prev;
			}
			t0.i.r = SIGN9(t0.i.r);
			t0.i.g = SIGN9(t0.i.g);
			t0.i.b = SIGN9(t0.i.b);
			TEX->i.r = t0.i.b + (((newk0 - invk0) * t0.i.g + 0x80) >> 8);
			TEX->i.g = t0.i.b + (((newk1 - invk1) * t0.i.r + (newk2 - invk2) * t0.i.g + 0x80) >> 8);
			TEX->i.b = t0.i.b + (((newk3 - invk3) * t0.i.r + 0x80) >> 8);
			TEX->i.a = t0.i.b;
			TEX->i.r &= 0x1ff;
			TEX->i.g &= 0x1ff;
			TEX->i.b &= 0x1ff;
			TEX->i.a &= 0x1ff;
		}
	}
}

void N64TexturePipeT::Copy(Color* TEX, INT32 SSS, INT32 SST, UINT32 tilenum, const rdp_poly_state& object, rdp_span_aux *userdata)
{
	const N64Tile* tile = object.m_tiles;
	INT32 sss1 = SSS;
	INT32 sst1 = SST;
	ShiftCopy(&sss1, &sst1, tilenum, object);
	sss1 = TRELATIVE(sss1, tile[tilenum].sl);
	sst1 = TRELATIVE(sst1, tile[tilenum].tl);
	sss1 = (SIGN17(sss1) >> 5) & 0x1fff;
	sst1 = (SIGN17(sst1) >> 5) & 0x1fff;
	Mask(&sss1, &sst1, tilenum, object);
	TEX->c = Fetch(sss1, sst1, tilenum, object, userdata);
}

void N64TexturePipeT::LOD1Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (object.OtherModes.persp_tex_en)
	{
		m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->TCDivNoPersp(nexts, nextt, nextsw, &nexts, &nextt);
	}

	userdata->m_start_span = false;
	userdata->m_precomp_s = nexts;
	userdata->m_precomp_t = nextt;

	int tempanded;
	if (*sss & 0x40000)
	{
		*sss = 0x7fff;
	}
	else if (*sss & 0x20000)
	{
		*sss = 0x8000;
	}
	else
	{
		tempanded = *sss & 0x18000;
		if (tempanded == 0x8000)
		{
			*sss = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sss = 0x8000;
		}
		else
		{
			*sss &= 0xffff;
		}
	}

	if (*sst & 0x40000)
	{
		*sst = 0x7fff;
	}
	else if (*sst & 0x20000)
	{
		*sst = 0x8000;
	}
	else
	{
		tempanded = *sst & 0x18000;
		if (tempanded == 0x8000)
		{
			*sst = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sst = 0x8000;
		}
		else
		{
			*sst &= 0xffff;
		}
	}
}

void N64TexturePipeT::LOD2Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (object.OtherModes.persp_tex_en)
	{
		m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->TCDivNoPersp(nexts, nextt, nextsw, &nexts, &nextt);
	}

	userdata->m_start_span = false;
	userdata->m_precomp_s = nexts;
	userdata->m_precomp_t = nextt;

	INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

	INT32 horstep = SIGN17(nexts & 0x1ffff) - SIGN17(*sss & 0x1ffff);
	INT32 vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(*sst & 0x1ffff);
	if (horstep & 0x20000)
	{
		horstep = ~horstep & 0x1ffff;
	}
	if (vertstep & 0x20000)
	{
		vertstep = ~vertstep & 0x1ffff;
	}

	INT32 lod = (horstep >= vertstep) ? horstep : vertstep;

	INT32 temp_anded;
	if (*sss & 0x40000)
	{
		*sss = 0x7fff;
	}
	else if (*sss & 0x20000)
	{
		*sss = 0x8000;
	}
	else
	{
		temp_anded = *sss & 0x18000;
		if (temp_anded == 0x8000)
		{
			*sss = 0x7fff;
		}
		else if (temp_anded == 0x10000)
		{
			*sss = 0x8000;
		}
		else
		{
			*sss &= 0xffff;
		}
	}

	if (*sst & 0x40000)
	{
		*sst = 0x7fff;
	}
	else if (*sst & 0x20000)
	{
		*sst = 0x8000;
	}
	else
	{
		temp_anded = *sst & 0x18000;
		if (temp_anded == 0x8000)
		{
			*sst = 0x7fff;
		}
		else if (temp_anded == 0x10000)
		{
			*sst = 0x8000;
		}
		else
		{
			*sst &= 0xffff;
		}
	}

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < object.MiscState.MinLevel)
	{
		lod = object.MiscState.MinLevel;
	}

	bool magnify = (lod < 32);
	INT32 l_tile = m_rdp->GetLog2((lod >> 5) & 0xff);
	bool distant = ((lod & 0x6000) || (l_tile >= object.MiscState.MaxLevel));

	userdata->LODFraction = ((lod << 3) >> l_tile) & 0xff;

	if(!object.OtherModes.sharpen_tex_en && !object.OtherModes.detail_tex_en)
	{
		if (distant)
		{
			userdata->LODFraction = 0xff;
		}
		else if (magnify)
		{
			userdata->LODFraction = 0;
		}
	}

	if(object.OtherModes.sharpen_tex_en && magnify)
	{
		userdata->LODFraction = userdata->LODFraction | 0x100;
	}

	if (object.OtherModes.tex_lod_en)
	{
		if (distant)
		{
			l_tile = object.MiscState.MaxLevel;
		}
		if (!object.OtherModes.detail_tex_en)
		{
			*t1 = (prim_tile + l_tile) & 7;
			if (!(distant || (!object.OtherModes.sharpen_tex_en && magnify)))
			{
				*t2 = (*t1 + 1) & 7;
			}
			else
			{
				*t2 = *t1; // World Driver Championship, Stunt Race 64, Beetle Adventure Racing
			}
		}
		else // Beetle Adventure Racing, World Driver Championship (ingame_, NFL Blitz 2001, Pilotwings
		{
			if (!magnify)
			{
				*t1 = (prim_tile + l_tile + 1);
			}
			else
			{
				*t1 = (prim_tile + l_tile);
			}
			*t1 &= 7;
			if (!distant && !magnify)
			{
				*t2 = (prim_tile + l_tile + 2) & 7;
			}
			else
			{
				*t2 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

void N64TexturePipeT::LOD2CycleLimited(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, const rdp_poly_state& object)
{
	INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (object.OtherModes.persp_tex_en)
	{
		m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->TCDivNoPersp(nexts, nextt, nextsw, &nexts, &nextt);
	}

	INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

	INT32 horstep = SIGN17(nexts & 0x1ffff) - SIGN17(*sss & 0x1ffff);
	INT32 vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(*sst & 0x1ffff);
	if (horstep & 0x20000)
	{
		horstep = ~horstep & 0x1ffff;
	}
	if (vertstep & 0x20000)
	{
		vertstep = ~vertstep & 0x1ffff;
	}

	INT32 lod = (horstep >= vertstep) ? horstep : vertstep;

	INT32 tempanded;
	if (*sss & 0x40000)
	{
		*sss = 0x7fff;
	}
	else if (*sss & 0x20000)
	{
		*sss = 0x8000;
	}
	else
	{
		tempanded = *sss & 0x18000;
		if (tempanded == 0x8000)
		{
			*sss = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sss = 0x8000;
		}
		else
		{
			*sss &= 0xffff;
		}
	}

	if (*sst & 0x40000)
	{
		*sst = 0x7fff;
	}
	else if (*sst & 0x20000)
	{
		*sst = 0x8000;
	}
	else
	{
		tempanded = *sst & 0x18000;
		if (tempanded == 0x8000)
		{
			*sst = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sst = 0x8000;
		}
		else
		{
			*sst &= 0xffff;
		}
	}

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < object.MiscState.MinLevel)
	{
		lod = object.MiscState.MinLevel;
	}

	bool magnify = (lod < 32);
	INT32 l_tile = m_rdp->GetLog2((lod >> 5) & 0xff);
	bool distant = (lod & 0x6000) || (l_tile >= object.MiscState.MaxLevel);

	if (object.OtherModes.tex_lod_en)
	{
		if (distant)
		{
			l_tile = object.MiscState.MaxLevel;
		}
		if (!object.OtherModes.detail_tex_en)
		{
			*t1 = (prim_tile + l_tile) & 7;
		}
		else
		{
			if (!magnify)
			{
				*t1 = (prim_tile + l_tile + 1);
			}
			else
			{
				*t1 = (prim_tile + l_tile);
			}
			*t1 &= 7;
		}
	}
}

void N64TexturePipeT::CalculateClampDiffs(UINT32 prim_tile, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff)
{
	const N64Tile* tile = object.m_tiles;
	if (object.OtherModes.cycle_type == CYCLE_TYPE_2)
	{
		if (object.OtherModes.tex_lod_en)
		{
			int start = 0;
			int end = 7;
			for (; start <= end; start++)
			{
				m_clamp_s_diff[start] = (tile[start].sh >> 2) - (tile[start].sl >> 2);
				m_clamp_t_diff[start] = (tile[start].th >> 2) - (tile[start].tl >> 2);
			}
		}
		else
		{
			int start = prim_tile;
			int end = (prim_tile + 1) & 7;
			m_clamp_s_diff[start] = (tile[start].sh >> 2) - (tile[start].sl >> 2);
			m_clamp_t_diff[start] = (tile[start].th >> 2) - (tile[start].tl >> 2);
			m_clamp_s_diff[end] = (tile[end].sh >> 2) - (tile[end].sl >> 2);
			m_clamp_t_diff[end] = (tile[end].th >> 2) - (tile[end].tl >> 2);
		}
	}
	else//1-cycle or copy
	{
		m_clamp_s_diff[prim_tile] = (tile[prim_tile].sh >> 2) - (tile[prim_tile].sl >> 2);
		m_clamp_t_diff[prim_tile] = (tile[prim_tile].th >> 2) - (tile[prim_tile].tl >> 2);
	}
}

#define USE_64K_LUT (1)

static INT32 sTexAddrSwap16[2] = { WORD_ADDR_XOR, WORD_XOR_DWORD_SWAP };
static INT32 sTexAddrSwap8[2] = { BYTE_ADDR_XOR, BYTE_XOR_DWORD_SWAP };

UINT32 N64TexturePipeT::_FetchRGBA_16_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];
	taddr &= 0x7ff;

	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchRGBA_16_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];
	taddr &= 0x7ff;

	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;
	return color.c;
}

UINT32 N64TexturePipeT::_FetchRGBA_16_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];
	taddr &= 0x7ff;

#if USE_64K_LUT
	return Expand16To32Table[((UINT16*)userdata->m_tmem)[taddr]];
#else
	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchRGBA_32_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT32 *tc = ((UINT32*)userdata->m_tmem);
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];

	taddr &= 0x3ff;
	UINT32 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 24) << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchRGBA_32_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT32 *tc = ((UINT32*)userdata->m_tmem);
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];

	taddr &= 0x3ff;
	UINT32 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 24) << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchRGBA_32_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];

	taddr &= 0x3ff;

	UINT32 c = ((UINT16*)userdata->m_tmem)[taddr];
	Color color;
	color.i.r = (c >> 8) & 0xff;
	color.i.g = c & 0xff;
	c = ((UINT16*)userdata->m_tmem)[taddr | 0x400];
	color.i.b = (c >>  8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchNOP(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata) { return 0; }

UINT32 N64TexturePipeT::_FetchYUV(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT16 *tc = ((UINT16*)userdata->m_tmem);

	int taddr = (tbase << 3) + s;
	int taddrlow = taddr >> 1;

	taddr ^= sTexAddrSwap8[t & 1];
	taddrlow ^= sTexAddrSwap16[t & 1];

	taddr &= 0x7ff;
	taddrlow &= 0x3ff;

	UINT16 c = tc[taddrlow];

	INT32 y = userdata->m_tmem[taddr | 0x800];
	INT32 u = c >> 8;
	INT32 v = c & 0xff;

	v ^= 0x80; u ^= 0x80;
	u |= ((u & 0x80) << 1);
	v |= ((v & 0x80) << 1);

	Color color;
	color.i.r = u;
	color.i.g = v;
	color.i.b = y;
	color.i.a = y;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchCI_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	taddr &= 0x7ff;
	UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchCI_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	taddr &= 0x7ff;
	UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchCI_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	p = (tpal << 4) | p;

	Color color;
	color.i.r = color.i.g = color.i.b = color.i.a = p;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchCI_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 p = tc[taddr];
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchCI_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 p = tc[taddr];
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchCI_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	UINT8 p = tc[taddr];

	Color color;
	color.i.r = color.i.g = color.i.b = color.i.a = p;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchIA_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchIA_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchIA_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT8 i = p & 0xe;
	i = (i << 4) | (i << 1) | (i >> 2);

	Color color;
	color.i.r = i;
	color.i.g = i;
	color.i.b = i;
	color.i.a = (p & 1) * 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchIA_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 p = tc[taddr];
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchIA_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 p = tc[taddr];
	UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchIA_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	UINT8 p = tc[taddr];
	UINT8 i = p & 0xf0;
	i |= (i >> 4);

	Color color;
	color.i.r = i;
	color.i.g = i;
	color.i.b = i;
	color.i.a = ((p & 0xf) << 4) | (p & 0xf);

	return color.c;
}

UINT32 N64TexturePipeT::_FetchIA_16_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT16 *tc = ((UINT16*)userdata->m_tmem);
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];
	taddr &= 0x3ff;

	UINT16 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

#if USE_64K_LUT
	return Expand16To32Table[c];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchIA_16_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT16 *tc = ((UINT16*)userdata->m_tmem);
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];
	taddr &= 0x3ff;

	UINT16 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchIA_16_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT16 *tc = ((UINT16*)userdata->m_tmem);
	int taddr = (tbase << 2) + s;
	taddr ^= sTexAddrSwap16[t & 1];
	taddr &= 0x7ff;

	UINT16 c = tc[taddr];
	UINT8 i = (c >> 8);

	Color color;
	color.i.r = i;
	color.i.g = i;
	color.i.b = i;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchI_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 byteval = tc[taddr];
	UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | c) << 2];

#if USE_64K_LUT
	return Expand16To32Table[k];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(k);
	color.i.g = GET_MED_RGBA16_TMEM(k);
	color.i.b = GET_LOW_RGBA16_TMEM(k);
	color.i.a = (k & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchI_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 byteval = tc[taddr];
	UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | c) << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (k >> 8) & 0xff;
	color.i.a = k & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchI_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = ((tbase << 4) + s) >> 1;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	UINT8 byteval = tc[taddr];
	UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	c |= (c << 4);

	Color color;
	color.i.r = c;
	color.i.g = c;
	color.i.b = c;
	color.i.a = c;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchI_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 c = tc[taddr];
	UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[c << 2];

#if USE_64K_LUT
	return Expand16To32Table[k];
#else
	Color color;
	color.i.r = GET_HI_RGBA16_TMEM(k);
	color.i.g = GET_MED_RGBA16_TMEM(k);
	color.i.b = GET_LOW_RGBA16_TMEM(k);
	color.i.a = (k & 1) * 0xff;
	return color.c;
#endif
}

UINT32 N64TexturePipeT::_FetchI_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0x7ff;

	UINT8 c = tc[taddr];
	UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[c << 2];

	Color color;
	color.i.r = color.i.g = color.i.b = (k >> 8) & 0xff;
	color.i.a = k & 0xff;

	return color.c;
}

UINT32 N64TexturePipeT::_FetchI_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata)
{
	UINT8 *tc = userdata->m_tmem;
	int taddr = (tbase << 3) + s;
	taddr ^= sTexAddrSwap8[t & 1];
	taddr &= 0xfff;

	UINT8 c = tc[taddr];

	Color color;
	color.i.r = c;
	color.i.g = c;
	color.i.b = c;
	color.i.a = c;

	return color.c;
}

UINT32 N64TexturePipeT::Fetch(INT32 s, INT32 t, INT32 tilenum, const rdp_poly_state& object, rdp_span_aux *userdata)
{
	const N64Tile* tile = object.m_tiles;
	UINT32 tformat = tile[tilenum].format;
	UINT32 tsize =	tile[tilenum].size;

	UINT32 tbase = (tile[tilenum].line * t) & 0x1ff;
	tbase += tile[tilenum].tmem;
	UINT32 tpal	= tile[tilenum].palette;

	UINT32 index = (tformat << 4) | (tsize << 2) | ((UINT32) object.OtherModes.en_tlut << 1) | (UINT32) object.OtherModes.tlut_type;

	return ((this)->*(TexelFetch[index]))(s, t, tbase, tpal, userdata);
}
