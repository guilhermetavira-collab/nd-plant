/* plant.h — nd-plant's cross-module type: the map tile a plant grows on.
 *
 * plant_tile_t is read by nd-mob (spawn density) and the engine's map layer,
 * so it lives here rather than in the provider TU. There are no XY_DECLs in
 * this header; nd-plant implements engine hooks only.
 *
 * NOTE: this is a MODULE-OWNED header, not an engine one. The old location was
 * `include/uapi/plant.h`; the old `~/nd/module.mk` installed it as
 * `$(PREFIX)/include/nd/plant.h`, so `nd/` is this header's home and it is
 * installed here with `FOLDER := nd`.
 */

#ifndef ND_PLANT_H
#define ND_PLANT_H

#include <stdint.h>
#include <string.h>
#include <nd/plant-types.h>

#define ND_PLANT_MAX   16u
#define ND_PLANTS_SEED 5u
#define ND_PLANT_MASK  0x3u
#define ND_PLANT_HALF  (ND_PLANT_MASK >> 1)
#define ND_PLANT_N(pln, i) (((pln) >> ((i) * 2)) & 3u)

#ifndef ND_XXH32_U32_DEFINED
#define ND_XXH32_U32_DEFINED 1
static inline uint32_t
nd_xxh32_u32(uint32_t val, uint32_t seed)
{
	uint32_t h32 = seed + 0x165667B1u + 4u;
	h32 += val * 0xC2B2AE3Du;
	h32 = ((h32 << 17) | (h32 >> 15)) * 0x27D4EB2Fu;
	h32 ^= h32 >> 15;
	h32 *= 0x85EBCA77u;
	h32 ^= h32 >> 13;
	h32 *= 0xC2B2AE3Du;
	h32 ^= h32 >> 16;
	return h32;
}
#endif

/* Ground-plane collision footprint tiers relative to bottom-center (0, 0) anchor */
#define ND_PLANT_COL_NONE 0u /* Walkable ground flora / crop / herb */
#define ND_PLANT_COL_STD  1u /* Standard 1-cell tree trunk / cactus base */
#define ND_PLANT_COL_WIDE 2u /* Wide mature trunk base */

/* Packed per-slot wire byte (nd_map_tile_t.plant_id[s]):
 * bits 0..4: 1-based species index (1..16, 0 = none)
 * bit 5:     growth stage (0 = small, 1 = big)
 * bit 6:     art variant index (0..1 -> variants 1..2) */
#define ND_PLANT_PACK_ID(idx, big, var0) \
	((uint8_t)((((idx) + 1u) & 0x1Fu) | ((big) ? 0x20u : 0u) | (((var0) & 1u) << 6)))
#define ND_PLANT_UNPACK_PRESENT(pid) (((pid) & 0x1Fu) != 0u && (((pid) & 0x1Fu) <= ND_PLANT_MAX))
#define ND_PLANT_UNPACK_IDX(pid)     ((unsigned)(((pid) & 0x1Fu) ? (((pid) & 0x1Fu) - 1u) : 0u))
#define ND_PLANT_UNPACK_BIG(pid)     ((unsigned)(((pid) >> 5) & 1u))
#define ND_PLANT_UNPACK_VAR(pid)     ((unsigned)((((pid) >> 6) & 1u) + 1u))

typedef struct {
	int16_t     tmp_min, tmp_max;
	uint16_t    rn_min, rn_max;
	uint8_t     y;
	char        small, big;
	uint8_t     fg;
	uint8_t     small_art, big_art;
	uint8_t     small_col, big_col;
	uint16_t    crown_w; /* Mature overstory crown diameter in world units (0 for understory herbs) */
	const char *name;
	const char *slug;
} nd_plant_spec_t;

/* Canonical 16-species envelope table matching external/nd-plant/data/plants.dsv */
static const nd_plant_spec_t s_nd_plant_specs[ND_PLANT_MAX] = {
	/* 0  */ { -36,  54, 140,  515,  2, 'x', 'X', 10, 2, 2, 1, 1, 140, "pinus sylvestris",         "pinus_sylvestris" },
	/* 1  */ {  -8,  54, 240,  515,  2, 't', 'T', 10, 2, 2, 1, 1, 190, "pseudotsuga menziesii",    "pseudotsuga_menziesii" },
	/* 2  */ { -28,  54, 180,  480,  2, 'x', 'X',  3, 2, 2, 1, 1, 140, "betula pendula",           "betula_pendula" },
	/* 3  */ { -15, 115,  20,  420,  5, 'x', 'X',  3, 2, 2, 0, 0,   0, "linum usitatissimum",      "linum_usitatissimum" },
	/* 4  */ { -36,  54, 250,  515,  2, 'x', 'X',  7, 2, 2, 1, 1, 140, "betula pubescens",         "betula_pubescens" },
	/* 5  */ { -37,  36, 245,  515,  2, 'a', 'A', 10, 2, 2, 1, 2, 190, "abies alba",               "abies_alba" },
	/* 6  */ {  65, 115,   0,  145,  3, 'i', 'I', 10, 2, 2, 1, 1, 160, "arthrocereus rondonianus", "arthrocereus_rondonianus" },
	/* 7  */ {  65, 115,  35,  515,  1, 't', 'T', 11, 2, 2, 1, 1, 270, "acacia senegal",           "acacia_senegal" },
	/* 8  */ {  10, 115,  85,  360,  4, 'x', 'X',  7, 2, 2, 0, 0,   0, "daucus carota",            "daucus_carota" },
	/* 9  */ {  34, 115,  80,  460,  4, 'x', 'X',  1, 2, 2, 0, 0,   0, "solanum lycopersicum",     "solanum_lycopersicum" },
	/* 10 */ {  30,  65, 220,  460,  1, 'x', 'X', 10, 2, 2, 1, 2, 240, "quercus robur",            "quercus_robur" },
	/* 11 */ {  24, 115, 240,  515,  1, 'w', 'W',  2, 2, 2, 1, 2, 240, "salix babylonica",         "salix_babylonica" },
	/* 12 */ {  28,  65, 250,  515,  1, 'x', 'X', 10, 2, 2, 1, 2, 240, "acer pseudoplatanus",      "acer_pseudoplatanus" },
	/* 13 */ {  30, 115, 110,  515,  2, 'v', 'V',  3, 2, 2, 1, 2, 190, "olea europaea",            "olea_europaea" },
	/* 14 */ { -24, 115,  40,  480,  3, 'f', 'F', 15, 2, 2, 0, 0,   0, "bellis perennis",          "bellis_perennis" },
	/* 15 */ { -10, 115, 250,  515,  2, 'r', 'R',  6, 2, 2, 0, 0,   0, "typha latifolia",          "typha_latifolia" },
};

typedef struct {
	int16_t  tmp_lo, tmp_hi; /* Canonical climate temperature bounds for this biome */
	uint16_t rn_lo,  rn_hi;  /* Canonical moisture bounds for this biome */
	uint8_t  scrub_trees;    /* If 1, trees spawn in stunted/scrub stage (cpln = 1) */
	uint8_t  allow_flora;    /* 0 = barren (Water, Permanent Ice, Volcanic), 1 = vegetated */
} nd_biome_flora_profile_t;

/* Declarative per-biome climate envelope table (indexed by BIOME_* 0..18).
 * Overstory tree density is governed procedurally by species crown_w, room area,
 * and local climate (eff_tmp, eff_rn) in nd_plant_noise_eval_bio(). */
static const nd_biome_flora_profile_t s_nd_biome_flora_profiles[19] = {
	/* 0  water               */ {    0,    0,   0,   0, 0, 0 },
	/* 1  permanent ice       */ {  -50,  -36,   0, 512, 0, 0 },
	/* 2  tundra              */ {  -35,   -3, 128, 383, 0, 1 },
	/* 3  alpine meadow       */ {  -22,   28, 145, 360, 1, 1 },
	/* 4  mediterranean       */ {   32,   64, 128, 255, 0, 1 },
	/* 5  wetland             */ {    0,   88, 310, 500, 0, 1 },
	/* 6  cold desert         */ {  -35,   31,   0, 127, 0, 1 },
	/* 7  shrubland           */ {   -2,   31, 128, 255, 1, 1 },
	/* 8  coniferous forest   */ {   -2,   31, 256, 383, 0, 1 },
	/* 9  boreal forest       */ {  -35,   31, 384, 512, 0, 1 },
	/* 10 temperate grassland */ {   32,   64,  25, 127, 0, 1 },
	/* 11 woodland            */ {   32,   64, 256, 311, 0, 1 },
	/* 12 temperate seasonal  */ {   32,   64, 312, 383, 0, 1 },
	/* 13 temperate rainforest*/ {   32,   64, 384, 512, 0, 1 },
	/* 14 desert              */ {   66,  101,   0, 127, 0, 1 },
	/* 15 savannah            */ {   66,  101, 128, 255, 0, 1 },
	/* 16 tropical seasonal   */ {   66,  101, 256, 383, 0, 1 },
	/* 17 tropical rainforest */ {   66,  101, 384, 512, 0, 1 },
	/* 18 volcanic            */ {  102,  150,   0, 512, 0, 0 },
};

static inline uint8_t
nd_plant_infer_biome(int16_t tmp, uint16_t rn)
{
	static const uint8_t s_grid[4][4] = {
		{  6,  2,  2,  9 },
		{  6,  7,  8,  9 },
		{ 10,  4, 11, 13 },
		{ 14, 15, 16, 17 }
	};
	if (tmp < -36)
		return 1u;
	if (tmp >= 102)
		return 18u;
	uint32_t rn_bit = (uint32_t)rn / 128u;
	if (rn_bit > 3u) rn_bit = 3u;
	int32_t tmp_bit = ((int32_t)tmp - (-36)) / 34;
	if (tmp_bit < 0) tmp_bit = 0;
	if (tmp_bit > 3) tmp_bit = 3;
	uint8_t b = s_grid[tmp_bit][rn_bit];
	if (b == 11u && rn >= 312u)
		b = 12u;
	return b;
}

static inline int
nd_plant_find_spec(const char *name)
{
	if (!name || !name[0])
		return -1;
	for (unsigned i = 0; i < ND_PLANT_MAX; i++) {
		if (!strcmp(s_nd_plant_specs[i].name, name) ||
		    !strcmp(s_nd_plant_specs[i].slug, name))
			return (int)i;
	}
	return -1;
}

/* Compute procedural overstory canopy & root-moisture budget for a room of dimensions
 * (room_w x room_h) given effective temperature (eff_tmp), rainfall (eff_rn), and biome.
 * Returns available canopy area budget in square world units (px^2). */
static inline uint32_t
nd_plant_eval_canopy_budget_dims(uint32_t room_w, uint32_t room_h,
                                 uint8_t eff_bio, int16_t eff_tmp, uint16_t eff_rn,
                                 uint32_t ty)
{
	uint32_t room_area = room_w * room_h;
	if (room_area == 0u)
		room_area = 160u * 128u;

	/* Permafrost tundra (2) and cold desert (6) lack sufficient warm-season thaw
	 * or soil moisture to sustain woody tree roots unless precipitation reaches
	 * boreal snowpack depth (rn >= 384). */
	if (eff_bio == 2u || eff_bio == 6u)
		return 0u;
	if (eff_tmp < -2 && eff_rn < 384u) {
		/* High-altitude / timberline meadow (eff_bio == 3): sparse krummholz establishment */
		if (eff_bio == 3u && ((ty & 0x700u) == 0u))
			return (room_area * 95u) / 100u;
		return 0u;
	}

	/* Alpine timberline meadow (3) and semi-arid temperate steppe (10):
	 * periodic wind/drought exposure limits establishment to sheltered micro-sites (~1/8 rooms) */
	if ((eff_bio == 3u || eff_bio == 10u) && ((ty & 0x700u) != 0u))
		return 0u;

	/* Continuous moisture & thermal canopy closure factor (in hundredths of room_area):
	 *   - Exposed timberline / scrubland (scrub_trees == 1, e.g. Alpine Meadow, Shrubland)
	 *     and Arid/semi-arid margins (rn < 128): wind/cold/drought exposure limits
	 *     sustainable overstory to a single scrub/xerophytic tree (~45% of room_area).
	 *   - Sub-humid Savannah / Mediterranean (128 <= rn < 256):
	 *     - Savannah (eff_tmp >= 65): high evapotranspiration & lateral root competition
	 *       limit capacity to 1 umbrella crown (~65% of room_area).
	 *     - Mediterranean sclerophyll (eff_tmp < 65): supports open olive/maquis groves (~220%).
	 *   - Humid woodland (256 <= rn < 312): moderate multi-tree canopy (~360%).
	 *   - Closed-canopy forest / rainforest (rn >= 312): dense multi-story canopy (~780%). */
	uint32_t closure_pct = 0u;
	if (s_nd_biome_flora_profiles[eff_bio].scrub_trees || eff_rn < 128u) {
		closure_pct = 45u;
	} else if (eff_rn < 256u) {
		if (eff_tmp >= 65) {
			/* Hot tropical savannah: high evapotranspiration restricts root catchment to 1 tree */
			closure_pct = 65u;
		} else {
			/* Warm-temperate Mediterranean woodland: supports up to 2 sclerophyll trees */
			closure_pct = 220u;
		}
	} else if (eff_rn < 312u) {
		closure_pct = 360u;
	} else {
		closure_pct = 780u;
	}

	return (uint32_t)(((uint64_t)room_area * (uint64_t)closure_pct) / 100u);
}

/* Compute effective crown + root-catchment area demand (in px^2) for species `spec_idx`
 * at growth stage `cpln` (1 = small/sapling/scrub, 2 = mature) under rainfall `eff_rn`. */
static inline uint32_t
nd_plant_eval_crown_demand(unsigned spec_idx, unsigned char cpln, uint16_t eff_rn)
{
	if (spec_idx >= ND_PLANT_MAX)
		return 0u;
	const nd_plant_spec_t *pl = &s_nd_plant_specs[spec_idx];
	if (pl->big_col == 0u || pl->crown_w == 0u)
		return 0u; /* Ground-layer understory herb: zero overstory canopy demand */

	/* Effective crown diameter: saplings/scrub (cpln == 1) use 65% of mature diameter */
	uint32_t d = (cpln >= 2u) ? (uint32_t)pl->crown_w : (((uint32_t)pl->crown_w * 65u) / 100u);
	/* Circular crown area: (pi / 4) * d^2 ~= (785 * d^2) / 1000 */
	uint32_t area = (uint32_t)(((uint64_t)d * (uint64_t)d * 785u) / 1000u);

	/* Lateral root-catchment expansion in dry climates (rn < 256):
	 * trees in arid/savannah soils extend lateral roots beyond their dripline to capture water */
	if (eff_rn < 256u) {
		uint32_t drought_mult_pct = 100u + ((256u - (uint32_t)eff_rn) * 45u) / 128u;
		area = (uint32_t)(((uint64_t)area * (uint64_t)drought_mult_pct) / 100u);
	}
	return (area > 0u) ? area : 1u;
}

static inline uint32_t
nd_plant_noise_eval_bio_dims(plant_tile_t *pd, uint32_t he, uint32_t w, uint8_t is_water,
                             uint8_t bio_idx, uint32_t tm, int16_t tmp, uint16_t rn,
                             const unsigned *refs, uint32_t room_w, uint32_t room_h)
{
	memset(pd, 0, sizeof(*pd));
	if (he < w && is_water)
		return 0;

	uint8_t eff_bio = (bio_idx < 19u) ? bio_idx : nd_plant_infer_biome(tmp, rn);
	const nd_biome_flora_profile_t *prof = &s_nd_biome_flora_profiles[eff_bio];
	if (!prof->allow_flora)
		return 0;

	/* Clamp effective climate to the resolved biome's envelope so diurnal temperature
	 * swings or floodplain topographic overrides (Alpine Meadow, Wetland) remain stationary */
	int16_t eff_tmp = tmp;
	if (eff_tmp < prof->tmp_lo) eff_tmp = prof->tmp_lo;
	if (eff_tmp > prof->tmp_hi) eff_tmp = prof->tmp_hi;

	uint16_t eff_rn = rn;
	if (eff_rn < prof->rn_lo) eff_rn = prof->rn_lo;
	if (eff_rn > prof->rn_hi) eff_rn = prof->rn_hi;

	uint32_t ty = nd_xxh32_u32(tm ^ he, ND_PLANTS_SEED);
	unsigned cand_id[ND_PLANT_MAX];
	unsigned cand_spec[ND_PLANT_MAX];
	unsigned char cand_sz[ND_PLANT_MAX];
	unsigned char cand_tree[ND_PLANT_MAX];
	uint32_t cand_prio[ND_PLANT_MAX];
	unsigned cand_n = 0;

	uint32_t canopy_budget = nd_plant_eval_canopy_budget_dims(
		room_w, room_h, eff_bio, eff_tmp, eff_rn, ty);

	for (unsigned i = 0; i < ND_PLANT_MAX; i++) {
		const nd_plant_spec_t *pl = &s_nd_plant_specs[i];
		if ((eff_bio == 5u || eff_bio == 12u) && (i == 0u || i == 1u || i == 5u))
			continue;
		if (eff_tmp <= pl->tmp_min || eff_tmp >= pl->tmp_max ||
		    eff_rn <= pl->rn_min || eff_rn >= pl->rn_max)
			continue;

		uint32_t sv = nd_xxh32_u32(ty, i + 1u);
		if (sv >= (0xFFFFFFFFu >> pl->y))
			continue;

		uint32_t pv = (sv >> 5) & ND_PLANT_MASK;
		unsigned char cpln = 0;
		if (pl->big_col > 0) {
			if (prof->scrub_trees) {
				/* Shrubland & timberline krummholz: stunted/scrub growth stage */
				cpln = (pv != 0u) ? 1u : 0u;
			} else if (pv == 3u) {
				cpln = 2u;
			} else {
				cpln = (unsigned char)pv;
			}
		} else {
			if (pv != 3u)
				cpln = (unsigned char)pv;
		}
		if (!cpln)
			continue;

		uint32_t prio = nd_xxh32_u32(sv, tm ^ (i * 0x9E3779B9u));
		/* Favor canopy trees in forest slots while keeping understory diversity;
		 * in Wetlands (5), prioritize hydrophytic Willow (#11) and Bulrush (#15) */
		if (eff_bio == 5u && (i == 11u || i == 15u))
			prio |= 0xC0000000u;
		else if (pl->big_col > 0)
			prio |= 0x80000000u;

		cand_id[cand_n] = refs ? refs[i] : i;
		cand_spec[cand_n] = i;
		cand_sz[cand_n] = cpln;
		cand_tree[cand_n] = (pl->big_col > 0) ? 1u : 0u;
		cand_prio[cand_n] = prio;
		cand_n++;
	}

	/* Sort candidates by priority and select up to 3 slots respecting procedural canopy budget */
	for (unsigned s = 0; s < cand_n; s++) {
		unsigned best = s;
		for (unsigned j = s + 1u; j < cand_n; j++) {
			if (cand_prio[j] > cand_prio[best])
				best = j;
		}
		if (best != s) {
			unsigned tid = cand_id[s]; cand_id[s] = cand_id[best]; cand_id[best] = tid;
			unsigned tsp = cand_spec[s]; cand_spec[s] = cand_spec[best]; cand_spec[best] = tsp;
			unsigned char tsz = cand_sz[s]; cand_sz[s] = cand_sz[best]; cand_sz[best] = tsz;
			unsigned char ttr = cand_tree[s]; cand_tree[s] = cand_tree[best]; cand_tree[best] = ttr;
			uint32_t tpr = cand_prio[s]; cand_prio[s] = cand_prio[best]; cand_prio[best] = tpr;
		}
	}

	unsigned take = 0;
	unsigned trees_taken = 0;
	uint32_t rem_budget = canopy_budget;
	unsigned char apln[3] = { 0, 0, 0 };
	for (unsigned s = 0; s < cand_n && take < 3u; s++) {
		if (cand_tree[s]) {
			if (rem_budget == 0u)
				continue;
			uint32_t demand = nd_plant_eval_crown_demand(cand_spec[s], cand_sz[s], eff_rn);
			if (demand > rem_budget) {
				/* Allow the first solitary tree to establish if the room has positive
				 * overstory budget (e.g., an Acacia whose wide umbrella overhangs neighboring
				 * savannah rooms), consuming the entire remaining overstory budget. */
				if (trees_taken == 0u) {
					rem_budget = 0u;
				} else {
					continue;
				}
			} else {
				rem_budget -= demand;
			}
			trees_taken++;
		}
		pd->id[take] = cand_id[s];
		apln[take] = (unsigned char)(cand_sz[s] & 3u);
		take++;
	}
	if (take < 3u)
		pd->max = (unsigned char)take;

	uint32_t shuf_v = ~(ty >> 8);
	for (unsigned char i = 1; i < 3; i++) {
		if (shuf_v & i)
			continue;
		unsigned aux = pd->id[i - 1];
		pd->id[i - 1] = pd->id[i];
		pd->id[i] = aux;
		unsigned char aux_n = apln[i - 1];
		apln[i - 1] = apln[i];
		apln[i] = aux_n;
	}
	pd->n = (unsigned char)(apln[0] | (apln[1] << 2) | (apln[2] << 4));
	return ty;
}

static inline uint32_t
nd_plant_noise_eval_bio(plant_tile_t *pd, uint32_t he, uint32_t w, uint8_t is_water,
                        uint8_t bio_idx, uint32_t tm, int16_t tmp, uint16_t rn,
                        const unsigned *refs)
{
#ifdef ND_FIELD_H
	uint32_t rw = (uint32_t)nd_field_get_room_w();
	uint32_t rh = (uint32_t)nd_field_get_room_h();
#else
	uint32_t rw = 160u, rh = 128u;
#endif
	return nd_plant_noise_eval_bio_dims(pd, he, w, is_water, bio_idx, tm, tmp, rn, refs, rw, rh);
}

static inline uint32_t
nd_plant_noise_eval(plant_tile_t *pd, uint32_t he, uint32_t w, uint8_t is_water,
                    uint32_t tm, int16_t tmp, uint16_t rn, const unsigned *refs)
{
	return nd_plant_noise_eval_bio(pd, he, w, is_water, 0xFFu, tm, tmp, rn, refs);
}

static inline int
nd_plant_slot_glyph(const plant_tile_t *pd, unsigned side, uint8_t *out_glyph, uint8_t *out_fg)
{
	if (!pd || side >= 3)
		return 0;
	unsigned char sz = (unsigned char)ND_PLANT_N(pd->n, side);
	if (!sz || pd->id[side] >= ND_PLANT_MAX)
		return 0;
	const nd_plant_spec_t *pl = &s_nd_plant_specs[pd->id[side]];
	if (out_glyph) *out_glyph = (uint8_t)(sz > ND_PLANT_HALF ? pl->big : pl->small);
	if (out_fg)    *out_fg    = pl->fg;
	return 1;
}

static inline int
nd_plant_slot_meta(const plant_tile_t *pd, unsigned side, int32_t rx, int32_t ry,
                   int spec_idx_override, uint8_t *out_plant_id, uint8_t *out_col_mask)
{
	if (!pd || side >= 3)
		return 0;
	unsigned char sz = (unsigned char)ND_PLANT_N(pd->n, side);
	if (!sz)
		return 0;
	int idx = (spec_idx_override >= 0) ? spec_idx_override :
	          (pd->id[side] < ND_PLANT_MAX ? (int)pd->id[side] : -1);
	if (idx < 0 || idx >= (int)ND_PLANT_MAX)
		return 0;
	const nd_plant_spec_t *sp = &s_nd_plant_specs[idx];
	unsigned is_big = (sz > ND_PLANT_HALF) ? 1u : 0u;
	unsigned max_v = is_big ? sp->big_art : sp->small_art;
	uint32_t vhash = nd_xxh32_u32((uint32_t)(rx * 73856093 ^ ry * 19349663 ^ (int32_t)side), ND_PLANTS_SEED);
	unsigned var0 = (max_v > 1u) ? (vhash % max_v) : 0u;
	if (out_plant_id)
		*out_plant_id = ND_PLANT_PACK_ID((unsigned)idx, is_big, var0);
	if (out_col_mask)
		*out_col_mask = is_big ? sp->big_col : sp->small_col;
	return 1;
}

#endif /* !ND_PLANT_H */