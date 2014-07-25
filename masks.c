#include "common.h"

const u64 rowmask[8] = {
	0x00000000000000ff,
	0x000000000000ff00,
	0x0000000000ff0000,
	0x00000000ff000000,
	0x000000ff00000000,
	0x0000ff0000000000,
	0x00ff000000000000,
	0xff00000000000000,
};

const u64 colmask[8] = {
	0x0101010101010101,
	0x0202020202020202,
	0x0404040404040404,
	0x0808080808080808,
	0x1010101010101010,
	0x2020202020202020,
	0x4040404040404040,
	0x8080808080808080,
};

/* NE to SW */
const u64 diag1mask[15] = {
	0x0000000000000001,
	0x0000000000000102,
	0x0000000000010204,
	0x0000000001020408,
	0x0000000102040810,
	0x0000010204081020,
	0x0001020408102040,
	0x0102040810204080,
	0x0204081020408000,
	0x0408102040800000,
	0x0810204080000000,
	0x1020408000000000,
	0x2040800000000000,
	0x4080000000000000,
	0x8000000000000000,
};

/* NW to SE */
const u64 diag2mask[15] = {
	0x0100000000000000,
	0x0201000000000000,
	0x0402010000000000,
	0x0804020100000000,
	0x1008040201000000,
	0x2010080402010000,
	0x4020100804020100,
	0x8040201008040201,
	0x0080402010080402,
	0x0000804020100804,
	0x0000008040201008,
	0x0000000080402010,
	0x0000000000804020,
	0x0000000000008040,
	0x0000000000000080,
};

const u64 dangermask[64] = {
	0x81412111090707ff,
	0x02824222120f0fff,
	0x04048444241f1fff,
	0x08080888493e3eff,
	0x10101011927c7cff,
	0x2020212224f8f8ff,
	0x4041424448f0f0ff,
	0x8182848890e0e0ff,
	0x412111090707ff07,
	0x824222120f0fff0f,
	0x048444241f1fff1f,
	0x080888493e3eff3e,
	0x101011927c7cff7c,
	0x20212224f8f8fff8,
	0x41424448f0f0fff0,
	0x82848890e0e0ffe0,
	0x2111090707ff0707,
	0x4222120f0fff0f0f,
	0x8444241f1fff1f1f,
	0x0888493e3eff3e3e,
	0x1011927c7cff7c7c,
	0x212224f8f8fff8f8,
	0x424448f0f0fff0f0,
	0x848890e0e0ffe0e0,
	0x11090707ff070709,
	0x22120f0fff0f0f12,
	0x44241f1fff1f1f24,
	0x88493e3eff3e3e49,
	0x11927c7cff7c7c92,
	0x2224f8f8fff8f824,
	0x4448f0f0fff0f048,
	0x8890e0e0ffe0e090,
	0x090707ff07070911,
	0x120f0fff0f0f1222,
	0x241f1fff1f1f2444,
	0x493e3eff3e3e4988,
	0x927c7cff7c7c9211,
	0x24f8f8fff8f82422,
	0x48f0f0fff0f04844,
	0x90e0e0ffe0e09088,
	0x0707ff0707091121,
	0x0f0fff0f0f122242,
	0x1f1fff1f1f244484,
	0x3e3eff3e3e498808,
	0x7c7cff7c7c921110,
	0xf8f8fff8f8242221,
	0xf0f0fff0f0484442,
	0xe0e0ffe0e0908884,
	0x07ff070709112141,
	0x0fff0f0f12224282,
	0x1fff1f1f24448404,
	0x3eff3e3e49880808,
	0x7cff7c7c92111010,
	0xf8fff8f824222120,
	0xf0fff0f048444241,
	0xe0ffe0e090888482,
	0xff07070911214181,
	0xff0f0f1222428202,
	0xff1f1f2444840404,
	0xff3e3e4988080808,
	0xff7c7c9211101010,
	0xfff8f82422212020,
	0xfff0f04844424140,
	0xffe0e09088848281,
};

const u64 knightmask[64] = {
	0x0000000000020400,
	0x0000000000050800,
	0x00000000000a1100,
	0x0000000000142200,
	0x0000000000284400,
	0x0000000000508800,
	0x0000000000a01000,
	0x0000000000402000,
	0x0000000002040004,
	0x0000000005080008,
	0x000000000a110011,
	0x0000000014220022,
	0x0000000028440044,
	0x0000000050880088,
	0x00000000a0100010,
	0x0000000040200020,
	0x0000000204000402,
	0x0000000508000805,
	0x0000000a1100110a,
	0x0000001422002214,
	0x0000002844004428,
	0x0000005088008850,
	0x000000a0100010a0,
	0x0000004020002040,
	0x0000020400040200,
	0x0000050800080500,
	0x00000a1100110a00,
	0x0000142200221400,
	0x0000284400442800,
	0x0000508800885000,
	0x0000a0100010a000,
	0x0000402000204000,
	0x0002040004020000,
	0x0005080008050000,
	0x000a1100110a0000,
	0x0014220022140000,
	0x0028440044280000,
	0x0050880088500000,
	0x00a0100010a00000,
	0x0040200020400000,
	0x0204000402000000,
	0x0508000805000000,
	0x0a1100110a000000,
	0x1422002214000000,
	0x2844004428000000,
	0x5088008850000000,
	0xa0100010a0000000,
	0x4020002040000000,
	0x0400040200000000,
	0x0800080500000000,
	0x1100110a00000000,
	0x2200221400000000,
	0x4400442800000000,
	0x8800885000000000,
	0x100010a000000000,
	0x2000204000000000,
	0x0004020000000000,
	0x0008050000000000,
	0x00110a0000000000,
	0x0022140000000000,
	0x0044280000000000,
	0x0088500000000000,
	0x0010a00000000000,
	0x0020400000000000,
};

const u64 kingmask[64] = {
	0x0000000000000302,
	0x0000000000000705,
	0x0000000000000e0a,
	0x0000000000001c14,
	0x0000000000003828,
	0x0000000000007050,
	0x000000000000e0a0,
	0x000000000000c040,
	0x0000000000030203,
	0x0000000000070507,
	0x00000000000e0a0e,
	0x00000000001c141c,
	0x0000000000382838,
	0x0000000000705070,
	0x0000000000e0a0e0,
	0x0000000000c040c0,
	0x0000000003020300,
	0x0000000007050700,
	0x000000000e0a0e00,
	0x000000001c141c00,
	0x0000000038283800,
	0x0000000070507000,
	0x00000000e0a0e000,
	0x00000000c040c000,
	0x0000000302030000,
	0x0000000705070000,
	0x0000000e0a0e0000,
	0x0000001c141c0000,
	0x0000003828380000,
	0x0000007050700000,
	0x000000e0a0e00000,
	0x000000c040c00000,
	0x0000030203000000,
	0x0000070507000000,
	0x00000e0a0e000000,
	0x00001c141c000000,
	0x0000382838000000,
	0x0000705070000000,
	0x0000e0a0e0000000,
	0x0000c040c0000000,
	0x0003020300000000,
	0x0007050700000000,
	0x000e0a0e00000000,
	0x001c141c00000000,
	0x0038283800000000,
	0x0070507000000000,
	0x00e0a0e000000000,
	0x00c040c000000000,
	0x0302030000000000,
	0x0705070000000000,
	0x0e0a0e0000000000,
	0x1c141c0000000000,
	0x3828380000000000,
	0x7050700000000000,
	0xe0a0e00000000000,
	0xc040c00000000000,
	0x0203000000000000,
	0x0507000000000000,
	0x0a0e000000000000,
	0x141c000000000000,
	0x2838000000000000,
	0x5070000000000000,
	0xa0e0000000000000,
	0x40c0000000000000,
};
