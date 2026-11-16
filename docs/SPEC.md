# `nd-plant` Art & Botanical Sprite Specification (`SPEC.md`)

This document specifies the camera perspective, physical scale, root anchor placement, shader/lighting constraints, target pixel dimensions, and botanical species catalog for creating tree and vegetation PNG sprites in **NeverDark** (`external/nd-plant/art/plant/`).

---

## 1. Camera Angle & Perspective

- **Camera Elevation (Pitch)**: **$55^\circ$ above the southern horizon** (`ND_SEVEN_DIORAMA_PITCH = 55.0f` in `mods/nd/ux/nd_seven.h`), looking North with a $45^\circ$ vertical field of view (`ND_SEVEN_CAM_FOV = 45.0f`) at default camera distance $D = 280.0\text{ wu}$.
- **Sprite Perspective ($3/4$ Top-Down Diorama)**:
  - Draw plants from a **classic $3/4$ top-down RPG / diorama perspective** ($35^\circ$ tilt back from vertical, or $55^\circ$ downward view angle from horizontal).
  - The viewer sees both the **vertical front of the trunk/stems** and the **top-front $3/4$ volume of the crown/canopy**.
  - Tree canopies should look three-dimensional and slightly elliptical in depth rather than flat side-profile silhouettes: the front foliage overhang sits slightly lower, revealing the lower trunk and root flare underneath, while the upper/rear crown arches behind it.
- **3D Billboard Orientation**:
  - In the WebGL2 renderer (`nd_seven_gl_billboard_basis` in `mods/nd/ux/nd_seven_gl.c`), each billboard quad is tilted back by $\theta = 55^\circ$ around the East–West axis so that it stands perpendicular to the camera's line of sight at ground level without vertical foreshortening.

---

## 2. File Format, Lighting, Transparency & Shader Rules

1. **Image Format**:
   - **8-bit/channel RGBA PNG**, non-interlaced.
   - Background must be **100% transparent (`alpha = 0`)**.
2. **Do NOT Bake Ground Drop Shadows into the PNG**:
   - The WebGL2 billboard fragment shader (`s_fs_billboard` in `mods/nd/ux/nd_seven_gl.c`) automatically projects a real 3D elliptical ground-contact shadow (`shadow_rx`, `shadow_ry`) onto the terrain with dappled penumbra that responds to sun angle, cloud cover, and water submersion.
3. **Internal Shading & Horizontal Mirroring**:
   - The engine randomly **mirrors sprites horizontally** per instance (`flip = (variant >> 2) & 1` in `mods/nd/ux/nd_seven.c`) for natural variety.
   - Keep baked pixel-art shading **top-lit / zenith-lit** (sunlit crown tips at the top, deeper ambient occlusion inside lower leaf clusters and under the front canopy overhang) rather than harsh left-only or right-only directional shadows, so the sprite reads naturally whether flipped or unflipped.
   - The GPU shader dynamically applies 3D sunlight, sky hemisphere ambient, cloud shadows, subsurface leaf rim scattering, ground bounce light, and twilight/night color grading on top of the sprite's painted albedo.
4. **Live Wind Sway & Lamb Interaction**:
   - The renderer pins the bottom vertices (`qy = 0`) to the ground and shears the top vertices (`qy = 1`) using the live 2D recursive wind field (`eval_billboard_quad_with_origin` in `mods/nd/ux/nd_seven_gl.c`):
     - **Woody trunked trees (`col_mask > 0`)**: Slow, stiff pendulum trunk sway + secondary bough nod.
     - **Walkable herbs, reeds & wildflowers (`col_mask == 0`)**: Full flexible meadow-grass wind bend + physical parting when the lamb walks through them.

---

## 3. Physical Scale, Visual Scale Ladder & Root Anchor Placement

### 3.1 The Fundamental World Scale (`1 px = 1 wu = 4 cm`)
Every pixel (`texel`) in an unscaled sprite equals **1 World Unit (`1 wu = 4.0 cm`)** in the 3D diorama (`25 px = 1.00 m`):

| Reference Unit | Pixel / World Units (`px` = `wu`) | Physical Size (`m`) | Relative to the Lamb (`18 px` tall) | Code Constant / Source |
| :--- | :--- | :--- | :--- | :--- |
| **1 Texel / World Unit** | `1 × 1 px` | `0.04 × 0.04 m` (`4 cm`) | `1/18` of Lamb height | `1 wu` in `mods/nd/ux/nd_seven.c` |
| **1 Grid Cell (`NDW_CELL_PX`)** | `16 × 16 px` | `0.64 × 0.64 m` | `0.89×` Lamb height | `NDW_CELL_PX = 16` (`seven-world.h`) |
| **The Lamb (`NDS_LAMB` body)** | **`22.5 px` W × `18 px` H**<br>*(inside a `32 × 32 px` frame)* | **`0.90 m` long × `0.72 m` tall** | **`1.0×` (Baseline Reference)** | `LAMB_SIZE = 32`, `LAMB_FOOT_Y = 25` (`nd_seven_atlas_draw.h`) |
| **1 Room (`10 × 8` cells)** | **`160 × 128 px`** | **`6.40 × 5.12 m`** | `~7.1` Lambs wide × `7.1` Lambs tall | `NDW_TILE_W = 160`, `NDW_TILE_H = 128` (`seven-world.h`) |
| **7×7 Visible Room Window** | `1120 × 896 px` | `44.8 × 35.8 m` | Default camera view at `1.0x` zoom | `ND_SEVEN_CAM_DIST = 280.0f` |

- **How `lamb.png` is framed (`32 × 32 px` cell)**:
  - `htdocs/nd/lamb.png` is a `192 × 256 px` sheet (`6 columns × 8 rows` of `32 × 32 px` animation cells: rows `0..3` walk, rows `4..7` idle).
  - Inside each `32 × 32 px` cell, the Lamb's hooves touch **`y = 25` (`6 px` above the bottom edge**, `bwz_off = (32 - 1) - 25 = 6 px`) and its head crown reaches **`y = 7`**, making the Lamb's visible body **`18 px` tall** and **`22.5 px` long** nose-to-tail.

### 3.2 Visual Scale Ladder (Every Object Compared to the Lamb)

```text
Height (px / wu)
  68 px (2.72 m)  ┬  [Emergent Jungle Tree: Kapok / Giant Douglas Fir (big: 62x66 to 64x68 px) — 3.7x Lamb height]
  60 px (2.40 m)  ┼  [Standard Canopy Tree: Oak / Pine / Maple / Willow / Palm (big: 56x60 to 62x62 px) — 3.3x Lamb]
  54 px (2.16 m)  ┼  [Wide-Crown Tree: Umbrella Acacia / Olive (big: 60x52 px) & Large Boulder 'O' (46x60 to 53x54 px)]
  44 px (1.76 m)  ┼  [Mature Columnar Cactus (big: 26x44 px) & Giant Tree Fern (big: 44x44 px) — 2.4x Lamb height]
  35 px (1.40 m)  ┼  [Young / Small Tree Sapling (small: 32x35 to 42x36 px) & Medium Rock 'o' (26x31 to 41x42 px) — 1.9x Lamb]
  30 px (1.20 m)  ┼  [Tall Bulrush Reeds (big: 22x30 px) & Bushy Nightshade/Heliconia (big: 28x30 px) — 1.6x Lamb]
  25 px (1.00 m)  ┼  [Young Cactus (small: 20x25 px) & Flowering Meadow Herb: Flax/Daisy/Carrot (big: 26x25 px) — 1.4x Lamb]
  18 px (0.72 m)  ┼  ★ THE LAMB (18 px tall, 22.5 px long inside 32x32 frame) ★ / Young Shrub / Barrel Cactus (18x18 px)
  15 px (0.60 m)  ┼  [Small Pebble / Flat Stone '*' (14x13 to 17x15 px) & Young Basal Herb (small: 14x13 to 16x17 px)]
  11 px (0.44 m)  ┼  [Meadow Grass Tuft / Wildflower Bloom (10x12 to 20x18 px) — Hoof-to-Belly height on the Lamb]
   0 px (0.00 m)  ┴─────────────────────────────────────── Ground Contact Line (anchor_y) ───────────────────────────────────────
```

### 3.3 Root / Trunk Base Placement (`anchor_y`)
- **Horizontal Center**: Place the center of the trunk base, taproot, or stem clump horizontally at **`x = width / 2`**.
- **Vertical Ground Contact (`anchor_y`)**: Place the ground contact point of the trunk/roots **`5\text{ to }8\text{ px}$ above the bottom edge of the PNG** (`ndw_find_content` and `ndw_obj_anchor_y` in `external/nd-seven/include/nd/seven-world.h`):
  - **Large trees, emergent trees & mature cacti (`big`)**: `anchor_y = 8 px` from the bottom edge.
  - **Small trees, saplings, shrubs & rocks (`small`)**: `anchor_y = 6 px` from the bottom edge.
  - **Walkable herbs, wildflowers, reeds & pebbles (`f`, `F`, `r`, `R`, `*`)**: `anchor_y = 5–6 px` from the bottom edge.
- **Why $5\text{–}8\text{ px}$ above the bottom?**:
  - In a $55^\circ$ $3/4$ view, a tree trunk base, boulder, or clump of stems has front-to-back thickness on the ground plane (just like the Lamb's hooves sit `6 px` above the bottom of its `32×32` cell). Placing the anchor center $5\text{–}8\text{ px}$ above the bottom row allows the front roots, basal leaves, or rock base to sit naturally in front of the anchor point and keeps the base firmly planted when the upper quad sways in the wind.
- **No Extra Transparent Margin Needed**:
  - `nd_seven_plant_rect()` (`mods/nd/ux/nd_seven_atlas_draw.c`) automatically adds `4 px` left/right and `2 px` top transparent padding (`0 px` at the bottom so `anchor_y` is preserved) when packing PNGs into the runtime texture atlas.

---

## 4. Target PNG Dimensions by Object Category (Plants & Stones)

`seven_calc_obj_billboard_dims()` (`mods/nd/ux/nd_seven.c:905-948`) applies per-category world-unit bounding boxes (`max_w`, `max_h`), and the runtime atlas packer adds `8 px` to width (`4 px` left + `4 px` right) and `2 px` to height (`2 px` top). Keeping PNG dimensions within the target ranges below guarantees **1:1 crisp pixel-art rendering with zero downscaling**:

| Object Category | Stage / Tier | Ideal PNG Size (`W × H` px) | Unscaled Cap (`W × H` px before pad) | Display Cap (`max_w × max_h` wu) | Ground Anchor (`anchor_y`) | Size vs. Lamb (`18 px` H) | Collision (`col_mask`) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Emergent & Canopy Trees**<br>(Oak, Pine, Fir, Birch, Maple, Willow, Kapok, Palm, Bamboo) | `big`<br>`small` | `56 × 60` to `62 × 66`<br>`32 × 33` to `42 × 36` | `56 × 60`<br>`32 × 36` | `64 × 62 wu` ($2.56 \times 2.48\text{ m}$)<br>`40 × 38 wu` ($1.60 \times 1.52\text{ m}$) | `8 px`<br>`6 px` | `3.3× – 3.7×` Lamb<br>`1.8× – 2.0×` Lamb | `1` or `2` (Solid trunk)<br>`1` (Solid trunk) |
| **Wide-Crown Trees**<br>(Umbrella Acacia `#7`, Olive `#13`) | `big`<br>`small` | `58 × 52` to `60 × 52`<br>`32 × 32` to `38 × 34` | `60 × 52`<br>`36 × 32` | `68 × 54 wu` ($2.72 \times 2.16\text{ m}$)<br>`44 × 34 wu` ($1.76 \times 1.36\text{ m}$) | `8 px`<br>`6 px` | `2.9×` Lamb (`3×` wide)<br>`1.8×` Lamb | `1` or `2` (Solid trunk)<br>`1` (Solid trunk) |
| **Tree Ferns & Large Understory**<br>(`cyathea_cooperi` `#19`) | `big`<br>`small` | `40 × 40` to `44 × 44`<br>`26 × 24` to `28 × 26` | `40 × 44`<br>`26 × 26` | `48 × 46 wu` ($1.92 \times 1.84\text{ m}$)<br>`34 × 28 wu` ($1.36 \times 1.12\text{ m}$) | `6 px`<br>`6 px` | `2.4×` Lamb<br>`1.4×` Lamb | `1` (Solid caudex)<br>`0` or `1` |
| **Columnar & Prickly-Pear Cacti**<br>(`arthrocereus` `#6`, `opuntia` `#21`) | `big`<br>`small` | `20 × 42` to `26 × 44` (`28 × 26` pad)<br>`16 × 23` to `22 × 25` (`18 × 18` barrel) | `20 × 44`<br>`12 × 26` | `28 × 46 wu` ($1.12 \times 1.84\text{ m}$)<br>`20 × 28 wu` ($0.80 \times 1.12\text{ m}$) | `8 px`<br>`6 px` | `2.4×` Lamb<br>`1.0× – 1.4×` Lamb | `1` (Solid stem)<br>`1` (Solid stem) |
| **Wetland Reeds / Bulrush**<br>(`typha_latifolia` `#15`) | `big`<br>`small` | `20 × 30` to `22 × 30`<br>`14 × 20` to `16 × 22` | `20 × 32`<br>`12 × 22` | `28 × 34 wu` ($1.12 \times 1.36\text{ m}$)<br>`20 × 24 wu` ($0.80 \times 0.96\text{ m}$) | `6 px`<br>`5 px` | `1.7×` Lamb<br>`1.2×` Lamb | `0` (Walkable)<br>`0` (Walkable) |
| **Ground Herbs, Crops & Flowers**<br>(Flax, Carrot, Tomato, Daisy, Heliconia, Sagebrush) | `big`<br>`small` | `22 × 20` to `28 × 26`<br>`14 × 13` to `20 × 18` | `22 × 26`<br>`14 × 18` | `30 × 28 wu` ($1.20 \times 1.12\text{ m}$)<br>`22 × 20 wu` ($0.88 \times 0.80\text{ m}$) | `6 px`<br>`5 px` | `1.1× – 1.4×` Lamb<br>`0.7× – 1.0×` Lamb | `0` (Walkable)<br>`0` (Walkable) |
| **Stones, Rocks & Boulders**<br>(`external/nd-stone`, `stone.h`) | `O` Boulder (`1/8`)<br>`o` Rock (`2/8`)<br>`*` Pebble (`5/8`) | `46 × 54` to `53 × 60`<br>`26 × 28` to `41 × 42`<br>`14 × 13` to `22 × 23` | `53 × 60`<br>`41 × 42`<br>`22 × 23` | `53 × 60 wu` ($2.12 \times 2.40\text{ m}$)<br>`41 × 42 wu` ($1.64 \times 1.68\text{ m}$)<br>`22 × 23 wu` ($0.88 \times 0.92\text{ m}$) | `6 px`<br>`6 px`<br>`5 px` | `3.0× – 3.3×` Lamb<br>`1.5× – 2.3×` Lamb<br>`0.7× – 1.2×` Lamb | `2` (Solid boulder)<br>`1` (Solid rock)<br>`0` (Walkable pebble) |
| **Meadow Grass Tufts & Blooms**<br>(`NDB_*`, `NDS_DECOR_*`) | Floor tuft<br>Wildflower | `16 × 18` to `20 × 22`<br>`10 × 12` to `15 × 12` | `20 × 22`<br>`15 × 12` | `20 × 22 wu` ($0.80 \times 0.88\text{ m}$)<br>`15 × 12 wu` ($0.60 \times 0.48\text{ m}$) | `4 px`<br>`4 px` | `0.6× – 1.0×` Lamb<br>`0.5× – 0.7×` Lamb | `0` (Walkable)<br>`0` (Walkable) |

*(Note: The runtime PNG decoder in `nd_seven_atlas_draw.c` accepts images up to `160 × 160 px`. Larger old-growth or emergent jungle trees up to `160 × 160 px` can be enabled at any time by raising `max_w` / `max_h` in `seven_calc_obj_billboard_dims`).*

---

## 5. Directory Layout & The 16 Canonical Plant Species

Each species in `s_nd_plant_specs` (`external/nd-plant/include/nd/plant.h` and `external/nd-plant/data/plants.dsv`) has **2 growth stages** (`small` and `big`) and **2 art variants per stage** (`1.png` and `2.png` = **4 PNGs per species**, **64 PNGs total**), stored as standalone PNG assets at:

```text
external/nd-plant/art/plant/<slug>/small/1.png
external/nd-plant/art/plant/<slug>/small/2.png
external/nd-plant/art/plant/<slug>/big/1.png
external/nd-plant/art/plant/<slug>/big/2.png
```
*(Mirrored to `htdocs/nd/art/plant/<slug>/<small|big>/<1..2>.png` and loaded dynamically at runtime via `lam_image_request()` / `lam_image_read()` in `mods/nd/ux/nd_seven_atlas_draw.c`. **Never** overwrite these PNGs by cropping or procedurally kitbashing the legacy `tiles.png` tilemap; standalone flora artwork is authored directly in these folders.)*

### Canonical 16-Species Table (`s_nd_plant_specs`)

| ID | Folder Slug (`<slug>`) | Botanical Name (Common Name) | Glyphs (`small`/`big`) | Collision (`small`/`big`) | Harvest Drop | Climate (`tmp` / `rn`) & Primary Biomes | Visual & Botanical Silhouette Notes | Current PNG Sizes (`small` / `big`) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **0** | `pinus_sylvestris` | *Pinus sylvestris* (**Scots Pine**) | `x` / `X` | `1` / `1` | `stick` | `-36..44`, `140..515`<br>Boreal, Coniferous, Alpine, Mediter. | Warm flaky orange-red upper trunk bark, rugged dark lower bark, layered blue-green needle boughs. Excluded from warm floodplains (`5`) & broadleaf (`12`). | `46×36`, `32×35` /<br>`62×70` |
| **1** | `pseudotsuga_menziesii` | *Pseudotsuga menziesii* (**Douglas Fir**) | `t` / `T` | `1` / `1` | `stick` | `-8..50`, `240..515`<br>Coniferous, Temp. Rainforest | Tall, stately pyramidal conifer spire with dense dark-emerald tiers and deeply furrowed dark-brown trunk. Excluded from warm floodplains (`5`) & broadleaf (`12`). | `46×36` /<br>`62×66` |
| **2** | `betula_pendula` | *Betula pendula* (**Silver Birch**) | `x` / `X` | `1` / `1` | `stick` | `-28..58`, `180..480`<br>Boreal, Woodland, Temp. Seasonal | Slender **white peeling bark with dark diamond clefts**, graceful drooping twigs, airy golden-green foliage. | `36×33` /<br>`60×63`, `58×60` |
| **3** | `linum_usitatissimum` | *Linum usitatissimum* (**Common Flax**) | `x` / `X` | `0` / `0` | `stick` | `-15..95`, `55..420`<br>Grassland, Meadow, Shrubland | Walkable fibrous herb: slender upright pale-green stems with delicate 5-petalled sky-blue flowers (`small`) and golden seed bolls (`big`). Never spawns in hyper-arid desert (`rn < 55`). | `16×17` /<br>`28×25` |
| **4** | `betula_pubescens` | *Betula pubescens* (**Downy / Arctic Birch**) | `x` / `X` | `1` / `1` | `stick` | `-36..44`, `220..515`<br>Tundra edge, Boreal, Alpine Meadow | Hardy subarctic birch with matte greyish-white bark and more compact, wind-sculpted / multi-stemmed krummholz habit than *B. pendula*. | `36×33` /<br>`60×63`, `58×60` |
| **5** | `abies_alba` | *Abies alba* (**European Silver Fir**) | `a` / `A` | `1` / `2` | `stick` | `-37..34`, `245..515`<br>Boreal, Montane Coniferous | Straight silver-grey trunk with horizontal tiered branches and cool blue-green / silvery needle highlights. Strictly cold boreal/montane. | `46×36` /<br>`62×66` |
| **6** | `arthrocereus_rondonianus` | *Arthrocereus rondonianus* (**Columnar Cactus**) | `i` / `I` | `1` / `1` | `stick` | `60..115`, `0..135`<br>Hot Desert, Arid Shrubland | Ribbed spiny green columnar cactus stems (`small` = young 1–2 stem cluster; `big` = taller multi-arm candelabrum with pale pink/white night blooms). Primary xerophyte of Hot Desert (`14`). | `20×25`, `22×23` /<br>`26×44`, `26×42` |
| **7** | `acacia_senegal` | *Acacia senegal* (**Gum Arabic Acacia**) | `t` / `T` | `1` / `1` | `stick` | `64..115`, `95..420`<br>Tropical Savannah, Tropical Seasonal | Iconic **wide flat-topped umbrella crown**, forked thorny warm-brown branches, feathery olive-green foliage. `rn_min = 95` keeps it in Savannah/Seasonal woodland and out of barren sand dunes. | `38×34`, `38×36` /<br>`60×52`, `66×54` |
| **8** | `daucus_carota` | *Daucus carota* (**Wild Carrot / Queen Anne's Lace**) | `x` / `X` | `0` / `0` | `carrot` | `10..95`, `95..380`<br>Temperate Grassland, Woodland, Savannah | Walkable root vegetable: finely divided feathery fern-like green basal leaves (`small`, with hint of orange taproot crown) + white lacy flat-topped umbel flower clusters (`big`). | `14×13`, `20×14` /<br>`26×21`, `26×20` |
| **9** | `solanum_lycopersicum` | *Solanum lycopersicum* (**Wild Tomato**) | `x` / `X` | `0` / `0` | `tomato` | `34..115`, `110..515`<br>Warm Temperate, Mediter., Tropical | Walkable sprawling/bushy nightshade with serrated deep-green leaves and yellow star blossoms (`small`) and bright red ripe tomatoes (`big`). Serves as lush tropical/warm understory scrub. | `20×27`, `22×26` /<br>`28×32` |
| **10** | `quercus_robur` | *Quercus robur* (**English Pedunculate Oak**) | `x` / `X` | `1` / `2` | `stick` | `28..65`, `220..480`<br>Woodland, Temperate Seasonal, Temp. Rain | Stout, heavily buttressed dark-grey trunk with gnarled boughs and a broad, billowy deep-green lobed-leaf dome. Strictly temperate (`tmp < 65`). | `42×42`, `32×35` /<br>`74×71`, `60×61` |
| **11** | `salix_babylonica` | *Salix babylonica* (**Weeping Willow**) | `w` / `W` | `1` / `2` | `stick` | `24..115`, `280..515`<br>Wetland, Riparian, Rainforest | Leaning grey-brown trunk with long, sweeping curtain-like chartreuse/emerald fronds draping down toward the ground/waterline. Requires high moisture (`rn > 280`), never spawning in dry savannahs. | `36×33` /<br>`60×61`, `60×63` |
| **12** | `acer_pseudoplatanus` | *Acer pseudoplatanus* (**Sycamore Maple**) | `x` / `X` | `1` / `2` | `stick` | `28..65`, `250..515`<br>Temperate Seasonal, Woodland, Temp. Rain | Smooth grey-tan flaky bark with a dense, rounded crown of 5-lobed palmate leaf clusters and winged samara seed keys. | `42×42`, `32×35` /<br>`60×61`, `74×71` |
| **13** | `olea_europaea` | *Olea europaea* (**Mediterranean Olive**) | `v` / `V` | `1` / `2` | `stick` | `30..105`, `115..390`<br>Mediterranean, Savannah, Dry Woodland | Ancient, gnarled, twisted pale-ash trunk with an irregular spreading crown of dusty silvery-sage leaves. | `42×42`, `32×35` /<br>`60×61`, `58×54` |
| **14** | `bellis_perennis` | *Bellis perennis* (**Common Meadow Daisy**) | `f` / `F` | `0` / `0` | `none` | `-24..90`, `90..480`<br>Alpine Meadow, Grassland, Woodland | Walkable wildflower tuft: spoon-shaped basal green leaves with white-rayed, pink-tipped, golden-disk daisy flowers. Excluded from deserts (`rn < 90`). | `16×17` /<br>`28×25` |
| **15** | `typha_latifolia` | *Typha latifolia* (**Broadleaf Cattail / Bulrush**) | `r` / `R` | `0` / `0` | `stick` | `-10..115`, `290..515`<br>Wetland, Marsh, Rainforest Floor | Walkable hydrophytic marsh reed & lush jungle floor sedge: upright fan of flat blue-green sword blades (`small`) + tall stalks topped with dark-brown velvety cigar spikes (`big`). | `16×22` /<br>`22×30` |

---

## 6. Per-Biome Canopy, Sparsity & Multi-Story Density Rules (`s_nd_biome_flora_profiles`)

To ensure that a **Jungle looks like a dense, multi-layered Jungle** and a **Desert looks like a vast, barren Desert**, vegetation density operates at three coordinated levels across the **19 Whittaker biomes**:

1. **Slot Selection & Room Sparsity (`nd_plant_noise_eval_bio` in `external/nd-plant/include/nd/plant.h`)**:
   - `plant_tile_t` carries `3` slots per room (`side = 0, 1, 2`), compatible with the 3-cell map tile contract (`ndw_tile_t.glyph[3]`, `plant_id[3]`, `col_mask[3]`).
   - `sparse_room_mask`: Controls what fraction of rooms in arid/extreme biomes contain *any* flora at all (`(ty & ((uint32_t)sparse_room_mask << 12u)) == 0`). In **Hot Desert (`14`)** and **Cold Desert (`6`)**, `sparse_room_mask = 0x7` means **7 out of 8 rooms (`87.5%`) are 100% barren sand/rock (`0` objects)**, and only `1 in 8` rooms spawns an isolated xerophyte (`arthrocereus_rondonianus` `#6`).
   - `sparse_tree_mask`: Controls what fraction of vegetated rooms allow canopy trees (`(ty & ((uint32_t)sparse_tree_mask << 8u)) == 0`). In **Tropical Savannah (`15`)**, `sparse_tree_mask = 0x1` places umbrella-thorn acacia groves in `50%` of rooms while leaving open grassland vistas in the other `50%`.
2. **Biome-Scaled Cluster Expansion & Understory Layering (`ndw_tile_objects` in `external/nd-seven/include/nd/seven-world.h`)**:
   - Room object capacity is **`NDW_MAX_OBJ = 32`** (and `VIRGIN_MEMO_MAX_OBJ = 32` in `mods/nd/ux/nd_seven_motion.c`).
   - In **Rainforests (`13`, `17`)** and **Dense Forests (`8`, `9`, `12`, `16`)**, each active tree slot expands into a **multi-story cluster**:
     - **Canopy Layer (`big` trees)**: `4 to 6` overlapping canopy trees per slot (`12–18` big trees per room in rainforests; `3–4` per slot = `9–12` in seasonal/boreal forests) with reduced minimum spacing (`min_d2 = 196`) and no artificial center clearing in wilderness rooms so crowns interlock into a continuous canopy.
     - **Understory Layer (`small` saplings / shrubs / herbs)**: Automatically accompanies canopy clusters in rainforests and dense forests (`2–3` understory plants per slot at tight spacing `min_d2 = 81`), filling the vertical space beneath the tree crowns.
   - In **Deserts (`6`, `14`)**, when the rare `1-in-8` room roll succeeds, cluster count is capped to `1` single isolated cactus/scrub plant (`count = 1`).
3. **Continuous Climate-Driven Ground-Cover Carpet (`nd_seven_eval_meadow_area` in `mods/nd/ux/nd_seven.c`)**:
   - **Deserts (`6`, `14`), Ice (`1`), Alpine (`3`), & Arid Zones (`rn < 90`)**: **Zero grass tufts (`0.0` density)** on open sand/rock.
   - **Savannah (`15`), Shrubland (`7`), Mediterranean (`4`), Grassland (`10`)**: High-threshold clumped tussock gate (`patch_n > 0.64`), leaving `75–85%` open sun-baked soil between golden grass clumps.
   - **Rainforests (`13`, `17`) & Wetlands (`5`)**: Fine `16 wu` sampling grid (`2.6×` cell density) with lush fern/sedge carpet and dappled overhead canopy floor shading in `s_fs_terrain`.

| Biome ID | Biome Name | Temp (`tmp`) | Rain (`rn`) | `max_trees` (`0..3`) | `sparse_room_mask` (Empty Rooms) | `sparse_tree_mask` (Tree Groves) | Objects / Room (`ndw_tile_objects`) | Ground-Cover Tuft Carpet (`nd_seven.c`) | Dominant Species |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **0** | Water | — | — | `0` | — | — | `0` | None | *(Aquatic)* |
| **1** | Permanent Ice | `-50..-36` | `0..512` | `0` | — | — | `0` | **0% (Barren)** | *(Barren)* |
| **2** | Tundra | `-35..-3` | `128..383` | `0` | `0x1` (50% barren) | — | `0–2` herbs | Sparse moss/sedge | Daisy (`14`), Flax (`3`) |
| **3** | Alpine Meadow | `-22..28` | `145..360` | `1` *(scrub)* | `0x1` (50% barren) | `0x3` (1/4 rooms) | `0–3` krummholz/herbs | Patchy alpine turf | Stunted Pine (`0`), Birch (`2`,`4`), Fir (`5`), Daisy (`14`) |
| **4** | Mediterranean | `32..64` | `128..255` | `2` | `0x0` | `0x1` (1/2 rooms) | `2–6` olive/cypress & herbs | Clumped dry herbs (25%) | Olive (`13`), Scots Pine (`0`), Tomato (`9`), Carrot (`8`) |
| **5** | Wetland | `0..88` | `310..500` | `2` | `0x0` | `0x0` | `10–16` willows + reed beds | **Dense sedge/reed (140%)** | **Weeping Willow (`11`)**, **Bulrush (`15`)**, Oak (`10`) |
| **6** | Cold Desert | `-35..31` | `0..127` | `0` | **`0x7` (87.5% barren)** | — | **`0` (1 herb in 1/8 rooms)** | **0% (Barren sand/reg)** | Rare Flax (`3`) / Daisy (`14`) in 1/8 rooms |
| **7** | Shrubland | `-2..31` | `128..255` | `1` *(scrub)* | `0x1` (50% barren) | `0x1` (1/2 rooms) | `0–3` scrub bushes | Clumped dry tussocks (20%) | Scrub Pine (`0`), Scrub Birch (`2`,`4`), Flax (`3`) |
| **8** | Coniferous Forest | `-2..31` | `256..383` | `3` | `0x0` | `0x0` | `10–16` pines/firs + saplings | Moderate forest floor | Scots Pine (`0`), Douglas Fir (`1`), Silver Fir (`5`), Birch (`2`) |
| **9** | Boreal Forest (Taiga) | `-35..31` | `384..512` | `3` | `0x0` | `0x0` | `12–18` firs/pines/birch | Mossy taiga floor | Silver Fir (`5`), Scots Pine (`0`), Downy Birch (`4`), Douglas Fir (`1`) |
| **10** | Temperate Grassland | `32..64` | `25..127` | `1` | `0x1` (50% open) | `0x7` (1/8 rooms) | `0–3` meadow herbs / lone tree | Open prairie tussocks (35%) | Flax (`3`), Wild Carrot (`8`), Daisy (`14`), lone Olive (`13`) |
| **11** | Woodland | `32..64` | `256..311` | `2` | `0x0` | `0x0` | `6–10` oaks/birch + herbs | Dappled meadow (75%) | Pedunculate Oak (`10`), Sycamore Maple (`12`), Birch (`2`) |
| **12** | Temperate Seasonal | `32..64` | `312..383` | `3` | `0x0` | `0x0` | `12–18` broadleaf canopy + shrubs | Rich woodland floor | Pedunculate Oak (`10`), Sycamore Maple (`12`), Birch (`2`), Willow (`11`) |
| **13** | Temperate Rainforest | `32..64` | `384..512` | `3` | `0x0` | `0x0` | **`16–24` giant conifers/oaks + understory** | **Lush fern/moss carpet (160%)** | Douglas Fir (`1`), Oak (`10`), Maple (`12`), Willow (`11`), Bulrush (`15`) |
| **14** | Hot Desert | `66..101` | `0..127` | `1` | **`0x7` (87.5% barren)** | `0x0` | **`0` (1 cactus in 1/8 rooms)** | **0% (Pure sand dunes)** | **Columnar Cactus (`6`)** *(isolated 1 in 8 rooms)* |
| **15** | Tropical Savannah | `66..101` | `128..255` | `1` | `0x0` | `0x1` (1/2 rooms) | `1–4` umbrella acacias / scrub | **Clumped golden tussocks (20%)** | **Gum Acacia (`7`)**, Olive (`13`), Carrot (`8`), Tomato (`9`) |
| **16** | Tropical Seasonal | `66..101` | `256..383` | `3` | `0x0` | `0x0` | `12–18` tropical trees + understory | Lush seasonal floor | Gum Acacia (`7`), Weeping Willow (`11`), Olive (`13`), Tomato (`9`) |
| **17** | Tropical Rainforest | `66..101` | `384..512` | `3` | `0x0` | `0x0` | **`18–28` overlapping canopy + understory + floor** | **Dense jungle carpet (200%)** | Weeping Willow (`11`), Gum Acacia (`7`), Olive (`13`), Tomato (`9`), Bulrush (`15`) |
| **18** | Volcanic | `102..150` | `0..512` | `0` | — | — | `0` | **0% (Barren basalt/ash)** | *(Barren)* |

---

## 7. Priority Art Roadmap & Biome Decoration Plan (For Artist Collaboration)

Our goal is for every biome—especially **Tropical Jungles (`16, 17`)** versus **Hot & Cold Deserts (`14, 6`)**—to be unmistakably recognizable at first glance through both **engine placement rules** (documented in [`IMPROVE.md`](file:///home/quirinpa/site/IMPROVE.md)) and **dedicated botanical & decorative artwork**.

### 7.1 How the Engine Will Make Jungles & Deserts Look Authentic

1. **True Multi-Story Jungles (`BIOME_TROPICAL_RAINFOREST = 17` & `BIOME_TROPICAL_SEASONAL = 16`)**:
   - **Canopy & Understory Layering**: The engine raises room object capacity to `NDW_MAX_OBJ = 32` and removes the artificial empty box in the center of wilderness rooms. Each room in a Tropical Rainforest spawns **`12–18` overlapping `big` canopy trees** (`min_d2 = 196`) intermixed with **`6–10` `small` understory plants** (`min_d2 = 81`) right underneath the crowns (`18–28` plants per room).
   - **Lush Jungle Floor & Dappled Shade**: The ground shader (`s_fs_terrain`) casts cool, dappled overhead canopy shade (`0.84–0.96×`) over dark humus and warm red-ochre tropical laterite soil, carpeted by a `2.6×` denser (`16 wu` grid) floor layer of ferns, broad-leaf sedges, and bright rainforest blooms.
   - **Why Dedicated Tropical Art Matters**: Right now, the 16-species table temporarily borrows *Salix babylonica* (`#11`), *Acacia senegal* (`#7`), and *Olea europaea* (`#13`) to fill tropical canopy slots. Adding true tropical species (Kapok/Ceiba with buttress roots, Palms, Bamboo thickets, Tree Ferns, and Heliconia/Monstera understory) will replace those stand-ins completely.

2. **Truly Desertic Deserts (`BIOME_DESERT = 14` & `BIOME_COLD_DESERT = 6`)**:
   - **87.5% Barren Rooms (`sparse_room_mask = 0x7`)**: `7 out of 8` desert rooms spawn **zero plants (`0` objects)**—just sweeping, wind-sculpted 2D crescentic (`barchanoid`) sand dunes in Hot Deserts (`14`) or wind-scoured alkali gravel/reg in Cold Deserts (`6`).
   - **Isolated Xerophytes Only (`count = 1`)**: In the `1-in-8` rooms where desert flora does appear, the engine caps the cluster to **a single isolated plant (`count = 1`)**—never a grove or forest—and strictly blocks temperate herbs (`linum`, `bellis`, `daucus`) and savannah trees (`acacia_senegal`, `olea_europaea`) from spawning below `rn = 95`.
   - **Zero Grass Carpet**: Below `rn = 95`, meadow grass tuft density is strictly **`0.0`** (unless immediately touching an oasis shoreline).

---

### 7.2 Priority 1: Deduplicating & Refining the Existing 16 Species
Several folders in `external/nd-plant/art/plant/` currently share identical placeholder PNGs or 2×2 upscaled pixels:
1. **`abies_alba` (`#5`) vs. `pseudotsuga_menziesii` (`#1`)**: Currently byte-for-byte identical. Give *Abies alba* a cool silvery-blue horizontal-tiered subalpine fir silhouette and *Pseudotsuga menziesii* a tall, rugged Pacific-Northwest Douglas fir spire.
2. **`betula_pubescens` (`#4`) vs. `betula_pendula` (`#2`)**: Currently identical. Make *Betula pubescens* a compact, multi-stemmed, wind-sculpted subarctic/krummholz birch, while *Betula pendula* is tall and weeping with crisp white-and-black diamond bark.
3. **`acer_pseudoplatanus` (`#12`), `quercus_robur` (`#10`), and `olea_europaea` (`#13`)**: Currently share several `small` and `big` PNGs. Differentiating the gnarled, silvery-sage Mediterranean *Olea europaea*, the broad buttressed *Quercus robur*, and the warm 5-lobed *Acer pseudoplatanus* will immediately distinguish Mediterranean (`4`), Woodland (`11`), and Temperate Seasonal (`12`).
4. **`linum_usitatissimum` (`#3`) vs. `bellis_perennis` (`#14`)**: Currently identical (`16×17` and `28×25`). *Linum* should show slender upright stalks with sky-blue 5-petalled flowers / golden bolls, while *Bellis* is a low basal rosette with white-and-gold daisy blooms.

---

### 7.3 Priority 2: Planned Botanical Expansion (`IDs 16..25`, Wire Capacity up to `31`)
`ND_PLANT_PACK_ID` packs the 1-based species ID into `5` bits (`0x1F`), supporting up to **`31` canonical species** without changing the network/map binary format. Adding any of the species below only requires dropping `small/1..2.png` and `big/1..2.png` into `external/nd-plant/art/plant/<slug>/` and adding the row to `plants.dsv` / `plant.h`:

| Planned ID | Folder Slug (`<slug>`) | Species / Common Name | Target Biomes | Target PNG Size (`small` / `big`) | Visual & Silhouette Brief for Artist |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **16** | `ceiba_pentandra` | **Kapok / Ceiba Emergent Tree** | Tropical Rainforest (`17`), Tropical Seasonal (`16`) | `40×38` /<br>`64×68` (`anchor_y = 8`) | Massive flaring **plank buttress roots** at the base, tall smooth pale-grey column trunk, wide umbrella tier of deep emerald foliage with hanging lianas/epiphytes. |
| **17** | `roystonea_regia` | **Tropical Feather / Coconut Palm** | Tropical Rainforest (`17`), Tropical Seasonal (`16`), Oasis (`14`) | `36×36` /<br>`56×64` (`anchor_y = 7`) | Ringed, slightly curving grey-brown palm trunk crowned by a radiating rosette of arching glossy-green feather fronds (and coconut cluster at the crown base). |
| **18** | `bambusa_vulgaris` | **Giant Monsoon Bamboo Clump** | Tropical Seasonal / Monsoon Forest (`16`), Rainforest (`17`) | `32×36` /<br>`52×62` (`anchor_y = 7`) | Tight cluster of segmented golden-green & chartreuse culm stalks arching outward at the top into feathery lance-leaved foliage. Iconic for Monsoon Forest (`16`). |
| **19** | `cyathea_cooperi` | **Giant Tree Fern / Broadleaf Understory** | Tropical Rainforest (`17`), Temperate Rainforest (`13`) | `28×26` /<br>`44×44` (`anchor_y = 6`) | Fibrous dark-brown caudex trunk topped by a wide, lacy star-burst of bright emerald unfurling fern fronds (`small` = low rainforest fern/monstera bush). |
| **20** | `heliconia_rostrata` | **Jungle Heliconia / Bird-of-Paradise** | Tropical Rainforest (`17`), Tropical Seasonal (`16`) | `20×20` /<br>`30×28` (`col = 0`, walkable) | Broad paddle-shaped banana-like leaves with vibrant hanging crimson-and-gold lobster-claw bracts. Breaks up the green jungle understory with tropical color. |
| **21** | `opuntia_ficus_indica` | **Prickly Pear / Barrel Cactus** | Hot Desert (`14`), Arid Shrubland (`7`), Mediter. (`4`) | `18×18` /<br>`28×26` (`col = 1`, solid) | Low, sprawling paddle cactus (`small` = round golden-spined barrel cactus; `big` = branching flat blue-green pads with bright yellow/magenta desert blooms). |
| **22** | `artemisia_tridentata` | **Desert Sagebrush & Dry Scrub** | Cold Desert (`6`), Shrubland (`7`), Hot Desert (`14`) | `18×16` /<br>`28×24` (`col = 0`, walkable) | Gnarled, twisted dry grey-brown woody stems with dusty silvery-sage / sun-bleached ochre foliage (`small` = dry desert tumbleweed/thorn scrub). |
| **23** | `taxodium_distichum` | **Swamp Bald Cypress / Mangrove** | Wetland (`5`), Tropical Floodplain (`17`) | `38×36` /<br>`60×64` (`anchor_y = 8`) | Fluted, buttressed reddish-brown swamp trunk surrounded by conical woody "knees" (or arched mangrove prop roots), draped in trailing pale-green Spanish moss. |
| **24** | `cladonia_rangiferina` | **Arctic Cushion Moss & Reindeer Lichen** | Tundra (`2`), Alpine Meadow (`3`) | `16×12` /<br>`24×16` (`col = 0`, walkable) | Low, wind-hugging mounds of pale creamy-grey reindeer lichen, bright tundra moss cushions, and tiny purple saxifrage blooms. |

---

### 7.4 Priority 3: Biome-Fitting Ground Decorations & Stones (`nd-stone` & Meadow Layer)
A biome's identity comes as much from its **non-tree ground props** as from its trees. Here is how ground decorations are organized across biomes so Deserts, Jungles, and Tundra never share out-of-place props:

| Biome Family | Stone / Boulder Props (`*` Pebble, `o` Rock, `O` Boulder) | Small Ground / Floor Decorations | What Must NEVER Appear Here |
| :--- | :--- | :--- | :--- |
| **Hot Desert (`14`)** | Rare (`~1.2%` of rooms): **Wind-carved red/ochre sandstone hoodoos**, flat desert pavement slabs, warm terracotta pebbles. | Sun-bleached desert deadwood/driftwood, dry thorn twigs, wind-rippled sand crests. | Green meadow grass tufts, daisies (`14`), flax (`3`), carrots (`8`), mossy grey boulders, artificial river trenches. |
| **Cold Desert (`6`) & Shrubland (`7`)** | Frequent (`~15%` of rooms): **Angular alkali-grey & buff frost-shattered rocks**, stony reg/scree scatter, dry wash boulders. | Sparse dry sagebrush clumps, pale alkali-steppe straw tussocks (0% in `6`, 20% clumped in `7`). | Lush green turf, tall canopy trees, tropical cacti (`6`), wetland reeds (`15`). |
| **Tropical Rainforest (`17`)** | Very rare on deep humus (`~1.2%`): **Moss-draped dark jungle boulders**, wet river-worn stones along basin banks. | **200% density jungle floor**: fern fronds, broad-leaf aroids/monstera, fallen mossy logs, buttress roots, crimson/orchid blooms. | Bare sun-bleached pasture grass, pine/birch/oak/maple trees, dry desert sand. |
| **Tropical Seasonal / Monsoon (`16`)** | Moderate (`~4.7%`): **Warm iron-rich red-ochre laterite boulders** and weathered temple-stone outcrops. | **Sun-dappled monsoon floor**: golden-olive bamboo leaf litter, tall monsoon grass clumps, amber/magenta seasonal blossoms. | Boreal conifers (`0, 1, 5`), European birch (`2, 4`), cold snow/tundra rocks. |
| **Tundra (`2`) & Alpine (`3`)** | Frequent (`~15%`): **Frost-heaved granite erratics (`felsenmeer`)**, lichen-crusted slate slabs, snow-dusted scree. | Low cushion moss, pale reindeer lichen, cotton-grass puffs, stunted wind-sheared krummholz (`3`). | Tall multi-tree canopies, warm-climate broadleaf trees, lush tall grass. |
| **Volcanic (`18`)** | High (`~18%`): **Jagged black basalt columns**, glossy obsidian shards, cooling sulfur/cinder vents. | 100% barren black ash and basaltic scoria (`0` plants, `0` grass tufts). | Any living vegetation or green grass tufts. |


