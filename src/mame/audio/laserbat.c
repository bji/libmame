#include "emu.h"
#include "sound/sn76477.h"
#include "sound/tms3615.h"
#include "includes/laserbat.h"

WRITE8_MEMBER(laserbat_state::laserbat_csound1_w)
{
	m_csound1 = data;
}

WRITE8_MEMBER(laserbat_state::laserbat_csound2_w)
{
	int ksound = 0;

	if (data & 0x01)
	{
		int noise_filter_res = 0, vco_res = 0;

		switch(m_csound1 & 0x07)
		{
		case 0x00:
			noise_filter_res = RES_K(270);
			vco_res = RES_K(47);
			break;
		case 0x01:
			noise_filter_res = RES_K(220);
			vco_res = RES_K(27);
			break;
		case 0x02:
			noise_filter_res = RES_K(150);
			vco_res = RES_K(22);
			break;
		case 0x03:
			noise_filter_res = RES_K(120);
			vco_res = RES_K(15);
			break;
		case 0x04:
			noise_filter_res = RES_K(82);
			vco_res = RES_K(12);
			break;
		case 0x05:
			noise_filter_res = RES_K(68);
			vco_res = RES_K(8.2);
			break;
		case 0x06:
			noise_filter_res = RES_K(47);
			vco_res = RES_K(6.8);
			break;
		case 0x07:
			noise_filter_res = RES_K(33);
			vco_res = RES_K(4.7);
			break;
		}

		sn76477_noise_filter_res_w(m_sn, noise_filter_res);
		sn76477_vco_res_w(m_sn, vco_res);

		sn76477_enable_w(m_sn, (m_csound1 & 0x08) ? 1 : 0); // AB SOUND
		sn76477_mixer_b_w(m_sn, (m_csound1 & 0x10) ? 1 : 0); // _VCO/NOISE

		m_degr = (m_csound1 & 0x20) ? 1 : 0;
		m_filt = (m_csound1 & 0x40) ? 1 : 0;
		m_a = (m_csound1 & 0x80) ? 1 : 0;
		m_us = 0; // sn76477 pin 12
	}

	sn76477_vco_w(m_sn, (data & 0x40) ? 0 : 1);

	switch((data & 0x1c) >> 2)
	{
	case 0x00:
		sn76477_slf_res_w(m_sn, RES_K(27));
		break;
	case 0x01:
		sn76477_slf_res_w(m_sn, RES_K(22));
		break;
	case 0x02:
		sn76477_slf_res_w(m_sn, RES_K(22));
		break;
	case 0x03:
		sn76477_slf_res_w(m_sn, RES_K(12));
		break;
	case 0x04: // not connected
		break;
	case 0x05: // SL1
		m_ksound1 = m_csound1;
		break;
	case 0x06: // SL2
		m_ksound2 = m_csound1;
		break;
	case 0x07: // SL3
		m_ksound3 = m_csound1;
		break;
	}

	ksound = ((data & 0x02) << 23) + (m_ksound3 << 16) + (m_ksound2 << 8) + m_ksound1;

	tms3615_enable_w(m_tms1, ksound & 0x1fff);
	tms3615_enable_w(m_tms2, (ksound >> 13) << 1);

	m_bit14 = (data & 0x20) ? 1 : 0;

	// (data & 0x80) = reset
}
