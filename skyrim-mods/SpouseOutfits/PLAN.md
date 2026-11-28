# SpouseOutfits: trang phục + vũ khí cho NPC nữ (bản CHỐT)

Chia bằng **SPID** (`SpouseOutfits_DISTR.ini`, lọc theo base FormID của từng NPC). Bảng dưới là bản chốt cuối cùng;
lịch sử lựa chọn xem trong git log của file này.

## Ràng buộc
- Body **CBBE 3BA**, **không SMP**: FSMP tắt, món nào có SMP thì dính xuống sàn (đã gặp với Skyforge Assassins,
  Guardian Goddess, Yokuda, 2026-11-28). Kiểm tra bằng mesh: chuỗi `HDT Skinned Mesh Physics Object` trong nif (cả nif
  ShapeData của BodySlide), file `.xml` trong meshes. Tóm tắt Nexus của ELLE ghi "armour" vẫn có thể có mảnh SMP (mô tả chi
  tiết mới liệt kê). Mảnh SMP chỉ là phụ kiện thì bỏ mảnh đó, mảnh chính là SMP thì bỏ cả bộ. Goddess of War V2: CBPC, ổn.
- Đã có **Somewhere in Between** (Dayhawk) và **Guards Armor Replacer** (NordwarUA): không đè, không đề xuất lại.
- Chỉ đồ **mới, standalone**; không replacer. Không bikini, không đồ hiện đại, không đồ từ game/phim khác.
- **NSFW chỉ cho 3 nữ tu Dibella** (Anwen, Orla, Senna).
- Mỗi mod trang phục chạy qua `tools/strip_outfit_plugin.py`: chỉ giữ đồ + công thức temper. Ẩn trong MO2 các file chia
  đồ đi kèm mod (`*_DISTR.ini`, `*_KID.ini`, SkyPatcher, Container Distribution Framework, `*_SWAP.ini`).
  Deze: cài bản **without NPCs**.
- Temper: chỉ giữ công thức temper có sẵn trong mod; mod nào không có thì thôi, **không thêm** (người dùng quyết 2026-11-27).
- **Không đồ đội đầu che tóc** (người dùng dùng NPC replacer, muốn thấy tóc): không đưa vào Outfit món nào chiếm slot 30
  (head), 31 (hair) hoặc 41 (long hair): mũ trùm, mũ giáp, tóc giả. Vòng/vương miện chỉ dùng slot 42 (circlet) thì được.
  Đã quét plugin trên máy, các món phải bỏ:
  - Deze Clothing: mọi Hood (Restoration, Apprentice, Journey, Destruction, Court).
  - C5Kev's Flirty In Fur: 00FlirtySkirtHelm. Nord Steelheart: ArmorSteelHeartHelmet (Crown slot 42 thì được).
  - COCO Goddess of War V2: GWV2_hair*/hairsmp* (tóc giả), GWV2_helmet01/02 (slot 42 nhưng che tóc, người dùng đã xem).
  - Iron Battlemage: mọi Helmet/Hood. Wayward Ranger: Hood. Draugr Wanderer: Hood. Obi's Hood and Steel: 1_H&S_Hood.
  - Dragonbone Barbarian: Helmet. Orcish Marauder: Helmet. Obi's Common Outfit: 1_Obi_CommonHood.
  - Hunting Grounds: mọi HHHood có tóc/mũ; HHHoodOff (khăn hạ xuống, slot 44) thì được.
  - Wayward Knight: mọi Helmet, Helmet Crest. ELLE Warrior: Hair.
  Mặt nạ (không che tóc nhưng che mặt): Obi's Masquerade Mask (đã bỏ), West Wind AssaultMask: bỏ luôn.

## Nhóm vợ (ứng viên kết hôn nữ, không phải follower)
| NPC | Base ID | Trang phục (ID Nexus) | Ghi chú |
|---|---|---|---|
| Anwen | 00013386 | Hezimao 42 Dress (110988) | NSFW. Váy ren xuyên thấu, ngắn |
| Orla | 000133B0 | Korana Priestess Of Dibella (112973, file 3BA) | NSFW |
| Senna | 000133B7 | **CHỐT: Bisquits Pain Bringer** (111059, contentcat) | NSFW. Dây xích + trang sức vàng; cùng tác giả với Anwen/Orla |
| Taarie | 000132AB | Obi's Masquerade Outfit 3BA (116474) | **Không dùng mặt nạ** |
| Dravynea | 0001365F | Deze Clothing – Mage (50446), bộ court wizard/College | |
| Muiri | 0001406B | Obi's Abyss Armor (2026-11-28) | Skyforge Assassins: váy và bông tai SMP, đã bỏ |
| Avrusa Sarethi | 00019BFE | Deze Clothing – Alchemist + Crafter boots (50446) | |
| Ghorza gra-Bagol | 0001339A | Deze Clothing – Blacksmith + Crafter boots (50446) | |
| Morwen | 018FA9 (Dragonborn.esm) | ELLE Tribal Scout 3BA (95333) | |
| Sylgja | 000C3A3F | Deze Clothing – tạp dề (Blacksmith/Alchemist) + Crafter boots (50446) | |
| Ysolda | 00013BAB | ELLE Tavern Maid (108334) | Ysolda muốn mua lại Bannered Mare. Trước: Deze Dealer (vanilla quá) |
| Hilund | 01A515 (Dragonborn.esm) | Deze Armor – Steel Plate A, có váy (49318) | |
| Temba Wide-Arm | 000658D2 | Knight of The Village 3BA (62364) | |
| Camilla Valerius | 0001347B | Obi's Skirt and Shirt 3BA (128400) | |
| Viola Giordano | 00014129 | DD – Temio Cloth 3BA (125970) | |
| Aeri | 0001360B | C5Kev's Flirty In Fur (64072) | |
| Gilfre | 0001367A | Toughened Traveler Outfit (gốc 54471 + 3BA 54584) | |
| Shahvee | 0001411A | Bỏ qua | Argonian |

## Follower (housecarl, hireling, follower lẻ)
Housecarl: có mod AIO đổi mọi housecarl thành nữ, FormID giữ nguyên. Follower thuộc phe lớn (Companions, College,
Dawnguard...) giữ đồng phục phe.
| Follower | Base ID | Trang phục (ID Nexus) | Ghi chú |
|---|---|---|---|
| Lydia (Whiterun) | 000A2C8E | ELLE Warrior (129907), bỏ Hair | Áo/găng/giày đổi sang giáp nặng |
| Argis the Bulwark (Markarth) | 000A2C8C | Deze Armor – Dwemer (49318) | |
| Iona (Riften) | 000A2C91 | ELLE – Ginnungagap 3BA (97931) | |
| Jordis the Sword-Maiden (Solitude) | 000A2C8F | COCO Goddess Of War V2 (78335), bản bạc | CBPC |
| Calder (Windhelm) | 000A2C90 | Nord Steelheart (gốc 110922 + 3BA 111014) | |
| Gregor (Dawnstar) | 00521E (HearthFires.esm) | Sovngarde Steel Armor (76115, file 3BA) | |
| Valdimar (Morthal) | 00521B (HearthFires.esm) | Iron Battlemage Armor 3BA (73162) | |
| Rayya (Falkreath) | 005215 (HearthFires.esm) | Obi's Leather Outfit 3BA (94495), bản Bra & Jacket | Áo/găng/legs đổi sang giáp nặng. Yokuda: Belt SMP, đã bỏ |
| Jenassa | 000B9982 | ELLE Elven Assassin (128383) | Người dùng chọn 2026-11-28 |
| Mjoll the Lioness | 0001336B | Wayward Knight Set (112793), giáp nặng, bỏ mũ/crest, dùng Skirt không SMP | Guardian Goddess: Skirt SMP, đã bỏ |
| Uthgerd the Unbroken | 000918E2 | ELLE Knight Winds (89980), màu đen | Áo/găng/giày đổi sang giáp nặng |
| Annekke Crag-Jumper | 00013666 | Wayward Ranger Armor 3BA, đủ giáp (124246) | |
| Eola | 0001990F | ELLE Wicked Corruption (108088) | Người dùng chọn 2026-11-28 |
| Aranea Ienith | 00028AD0 | Obi's Hood and Steel Armor 3BA (136388) | Tím = Azura. Phương án 2: Ashtoreth Elven Sentinel (79296) bỏ áo choàng |
| Illia | 00048C2F | ELLE Sparrows Mage (96211) | Dremorn Witch: váy SMP, đã bỏ |
| Serana | 002B6C (Dawnguard.esm) | ELLE Dark Rebel 3BA (102957) | |
| Frea | 017934 (Dragonborn.esm) | Dragonbone Barbarian 3BA (47466, cần bản gốc) | |
| Borgakh the Steel Heart | 00019959 | Orcish Marauder Armor 3BA (64629) | |
| Ugor | 00019E1A | C5Kev's Lacey Ranger Armor 3BA (103426) | |
| Adelaisa Vendicci | 0001411D | ELLE Wayfarer Navi (170449) | Thuyền trưởng East Empire Company, không phải lính Imperial |
| Brelyna Maryon | 0001C196 | Wilderness Witch (gốc 40963 + 3BA 40845) | Học trò College, trong game mặc đồ mage thường |
Knight of The Village (Temba): plugin `Knight of The Village Set.esp` ở mod bản BHUNP (62371), mod 3BA (62364) chỉ có BodySlide.
Orcish Marauder (Borgakh): BodySlide 3BA tải về có tên "Ominous Shaman Armor 3BBB(3BA) bodyslides 2.0" trong MO2 (cùng trang 64629).
Base ID đọc trực tiếp từ Skyrim.esm/DLC trên máy (record NPC_). Housecarl Hearthfire nằm trong `HearthFires.esm`.

## NPC nữ kết bạn qua favor (rủ đi bằng Immersive Recruit)
Lọc từ UESP Skyrim:Favors: bỏ người có chồng/con, người già, người có việc quan trọng; bỏ Wujeeta (Argonian).
Không dùng: Nordic Fur Armor (xấu), Fashions of the Huntsmen (người dùng để dành chia đồ chung), Pandorable's Lady Arkay
(là mod follower, phải cài follower mới có đồ). Cleric Armours of the Nine Divines (5537, có bản Arkay) chưa rõ có bản nữ.
Alessandra: loại Journeyman Healer Robes (người dùng không ưng), Priestess of Mara (gần như khỏa thân + SMP), Ida Faber nun
(anime, có thánh giá), Dark Dreams (lưới, gothic), 3BA Inquisitor (áo choàng SMP), Mancika's Templar (giáp nặng tối).
| NPC | Base ID | Trang phục (ID Nexus) | Vũ khí |
|---|---|---|---|
| Bolfrida Brandy-Mug | 00014126 | Simple Ancient Nord Dress 3BA (153101) | Iron Quarterstaff (HA) |
| Ganna Uriel | 0001365D | **CHỐT: J3 Elegant Explorer 3BA** (126956) | Steel Hatchet (HA) |
| Voada | 000133C0 | Wayward Ranger – chỉ bộ mặc trong (124246), màu khác Annekke | Iron Dagger |
| Alessandra | 00013347 | **CHỐT: EGIL Demon Hunter II Templar 3BA** (81705) | Templar Sword (EGIL, cùng mod) |
| Bagrak | 00019955 | Bladedancer's Edge (62751, file CBBE-3BA) | Orcish War Axe + Dwarven Bow |
| Yatul | 0001B077 | Hunting Grounds Outfit (bản gốc + 3BA 63986) | Orcish Shortsword (HA) + Supple Ancient Nord Bow |

## NPC mới (thêm 2026-11-27): dễ thương, muốn đi đây đó; chị em của người đã có trong danh sách
Trang phục đã chốt (2026-11-27). Vũ khí: đề xuất theo kỹ năng, đồ vanilla (2 mod mới không kèm vũ khí theo mô tả Nexus). Vũ khí: chưa chốt, theo kỹ năng trong bảng (đọc từ `DNAM` của NPC_ trong Skyrim.esm).
| NPC | Base ID | Quan hệ / lý do | Kỹ năng nổi bật | Trang phục | Vũ khí |
|---|---|---|---|---|---|
| Erdi | 00013271 | Hầu gái Blue Palace (Solitude), muốn đi khỏi đó | Smithing, 2H, 1H, Light | **Girl's Travel Outfit** (125910) | Steel Dagger |
| Aduri Sarethi | 00019BFF | Em gái Avrusa Sarethi, muốn đi khám phá | Alchemy, Destruction, Light, Sneak | **Mage's Daily Outfit** (155656, bản không SMP) | Elven Dagger |
| Gemma Uriel | 00014040 | Chị em với Ganna Uriel | Restoration, Enchanting, 1H, Block, Heavy | **Obi's Traveler's Romper** (176951, đã cài) | Imperial Sword + Imperial Light Shield |
| Endarie | 0001326F | Chị em với Taarie (Radiant Raiment, Solitude) | Sneak, Speech, Illusion | **Obi's Common Outfit** (110678, đã cài), **bỏ mũ trùm** | Elven Dagger |
Bỏ qua (người dùng quyết): Bujold (chị em Hilund, có chồng), Bolar (chị em Yatul, wise woman).
Đã có sẵn trong danh sách: Sylgja là con gái Annekke; Borgakh là con gái Bagrak.

## Giáo viên College (thêm 2026-11-28)
GenderShift bỏ qua NPC unique nên chỉ có 3 cô giáo. Người dùng muốn đồ riêng thay đồ College.
| NPC | Base ID | Trang phục | Ghi chú |
|---|---|---|---|
| Faralda (Destruction) | 0001C197 | ELLE Mageali (147010) | Bỏ Belt (SMP) |
| Colette Marence (Restoration) | 0001C19A | ELLE Lastriem (166180) | Bỏ Belt, Necklace (SMP) |
| Mirabelle Ervine (Master Wizard) | 0001C1A0 | Deze Clothing – Master Dress of Restoration + Mage Boots + Restoration Gloves | Dravynea giữ bộ Court |

## Giáp nặng cho người giỏi Heavy Armor (2026-11-28)
Lydia, Uthgerd, Rayya mặc đồ nhẹ: `SpouseOutfits.esp` ghi đè ARMO của áo, găng, giày (khóa `heavy` của NPC trong
`outfits.json`): loại giáp trong BOD2 thành Heavy, keyword ArmorLight thành ArmorHeavy. Không đổi chỉ số giáp, cân nặng
(người dùng quyết: temper và đồ phụ đã đủ mạnh). Đồ phụ vẫn là giáp nhẹ. Áp dụng cho mọi bản của món đó trong game.

# Vũ khí
Nguồn: trang Armory (SSE) của wiki LotD (legacy-of-the-dragonborn.fandom.com), kiểm tra chéo: món vanilla có trang
trên UESP, món Heavy Armory (HA) / Immersive Weapons (IW) có trang trên wiki LotD. Kỹ năng follower lấy từ UESP
(Skyrim:Followers). Quy tắc: có One-handed + Block → 1 tay + khiên; Two-handed → 2 tay; có Archery → thêm cung/nỏ;
pháp sư không có kỹ năng vũ khí → không cầm vũ khí cận chiến. Housecarl cầm khiên lính gác của Hold mình (vanilla).
Người dùng có **Guards Armor Replacer** (NordwarUA): nó thay hình đồ lính gác theo từng Hold, nên các khiên lính gác này
sẽ mang kiểu GAR của Hold đó, không cần thêm record. Markarth của GAR làm từ quặng Dwemer, hợp giáp Deze Dwemer của Argis.
Không dùng đồ unique trưng bày ở museum (Grimsever, vũ khí Thane...).
**Ưu tiên đồ cùng mod (2026-11-27):** nếu mod trang phục có kèm vũ khí/khiên thì NPC mặc bộ đó dùng vũ khí/khiên của mod
(vẫn theo quy tắc kỹ năng ở trên); không có thì mới dùng bảng dưới. **Ngoại lệ: housecarl luôn cầm khiên lính gác
Hold của mình**, không dùng khiên của mod. Đã quét plugin trên máy, các mod đang dùng có kèm đồ:
| Mod | Vũ khí/khiên trong plugin (EDID, FormID) | Dùng cho |
|---|---|---|
| C5Kev's Flirty In Fur | 0FlirtySkirtSword 000823, 00FlirtySkirtShield 000817 | Aeri: kiếm (thay Iron Hatchet) |
| [COCO] Goddess of War V2 | GWV2_shield 000862, GWV2_shield2 000864 (chỉ khiên) | Không dùng (housecarl giữ khiên Hold) |
| Sovngarde Steel Armor | SovngardeSteelSword 000808, ArmorSovngardeSteelShield 000809 | Gregor: kiếm (khiên giữ của Hold) |
| Knight of The Village Set (plugin ở bản BHUNP 62371) | AAKoTVSword 000806, AAKoTVSwordb 000824, AAKoTVShield 000803 | Temba: kiếm (thay Iron Hunter's Spear); không khiên (không có Block) |
| C5Kev's Lacey Ranger Armor 3BA | LaceyRangerBow 000804 | Ugor: cung (thay Orcish Bow) |
| EGIL Demon Hunter II Templar | EG_DHIITemplarSword 000812 (+ bản Gold/Wallnut), GreatSword, 2 khiên | Alessandra: Templar Sword (thay Silver Mace); không khiên (không có Block) |
Aeri không có Block nổi bật nên không cho khiên. Plugin EGIL nằm ở mod bản BHUNP (`EGIL DHII Templar.esp`), mod 3BA chỉ có BodySlide.
Mod **Immersive Weapons Distributed with SPID**: giữ nguyên, người dùng đã chỉnh để chỉ chia cho địch, phe lớn và NPC
quan trọng, ngoài phạm vi file này.
Lưu ý: SPID chỉ thêm vào túi; NPC tự chọn theo kỹ năng/AI. "Steel Crossbow" của LotD = "Crossbow" (Dawnguard) trên UESP.

## Nhóm vợ (không phải follower, chỉ vũ khí theo nghề; cung chỉ cho người đi săn)
| Nhân vật | Vũ khí | Nguồn |
|---|---|---|
| Aeri | Flirty Skirt Sword | C5Kev's Flirty In Fur (cùng mod) |
| Gilfre | Iron Double Axe | IW |
| Temba Wide-Arm | KoTV Sword (Knight of The Village, cùng mod) + Hunting Bow | cùng mod + vanilla |
| Sylgja | Nordic War Pick | IW |
| Avrusa Sarethi | Glass Dagger | vanilla |
| Muiri | Forsworn Dagger | HA |
| Camilla Valerius | Imperial Dagger | HA |
| Ysolda | Iron Shortsword | HA |
| Taarie | Elven Shortsword | HA |
| Viola Giordano | Imperial Shortsword | HA |
| Ghorza gra-Bagol | Orcish Maul | HA |
| Morwen | Stalhrim Hatchet | HA |
| Hilund | Nordic Long Mace | HA |
| Dravynea | Ebony Dagger (Dunmer) | vanilla |
| Anwen | Polished Silver Dagger | HA |
| Orla | Silver Dagger | HA |
| Senna | Nord Hero Dagger | HA |

## Follower
| Follower | Kỹ năng chính (UESP) | Vũ khí chính | Khiên | Cung/nỏ |
|---|---|---|---|---|
| Lydia (Whiterun) | Heavy, 1H, Archery, Block | Steel Longsword (IW) | Whiterun Guard's Shield | Nord Hero Bow |
| Argis (Markarth) | Heavy, 1H, Archery, Block | Eastern Dwemer Sword (IW) | Markarth Guard's Shield | Dwarven Crossbow |
| Iona (Riften) | Heavy, 1H, Archery, Block | Nord Hero Shortsword (HA) | Riften Guard's Shield | Nordic Scrimshaw Bow (IW) |
| Jordis (Solitude) | Heavy, 1H, Archery, Block | Silver Longsword (IW) | Solitude Guard's Shield | Imperial Bow |
| Calder (Windhelm) | Heavy, 1H, Archery, Block | Nordic War Axe (vanilla) | Windhelm Guard's Shield | Nordic Bow |
| Gregor (Dawnstar) | Heavy, 1H, Archery, Block | Sovngarde Steel Sword (cùng mod) | Pale Guard's Shield | Steel Crossbow |
| Rayya (Falkreath) | Heavy, 1H, Archery, Block | Steel Scimitar (IW, Redguard) | Falkreath Guard's Shield | Glass Bow |
| Valdimar (Morthal) | Heavy, 1H, Alteration, Destruction, Restoration | Honed Ancient Nord War Axe | Không (không có Block) | Không (không có Archery) |
| Jenassa | Archery, Light, Block, 1H | Steel Tanto (IW) | Elven Shield | Elven Hawk Bow (IW) |
| Mjoll the Lioness | Heavy, **2H**, Archery, Block | Viking Double Axe (IW, người dùng chọn 2026-11-28) | Không (2 tay) | Elven Bow |
| Uthgerd the Unbroken | Heavy, 1H, Archery, Block | Steel Spiked Club (HA) | Steel Shield | Ancient Nord Bow |
| Annekke Crag-Jumper | Archery, Light, Block, 1H | Steel War Pick (IW) | Hide Shield | Hunting Bow |
| Borgakh the Steel Heart | Heavy, 1H, Archery, Sneak | Orcish Saber (IW) | Không (không có Block) | Ebony Bow |
| Ugor | Archery, Light, Block, 1H | Orcish Hatchet (HA) | Orcish Shield | Lacey Ranger Bow (cùng mod) |
| Eola | Destruction, Sneak, Alteration, 1H | Honed Ancient Nord Dagger (HA) | Không | Không |
| Serana | Sneak, Conjuration, Light, 1H | Daedric Tanto (IW) | Không | Không |
| Frea | 1H, Alteration, Light, Restoration | Stalhrim Shortspear (HA, 1 tay) | Không | Không |
| Aranea Ienith | Destruction, Restoration, Alteration, Conjuration | Không (phép) | Không | Không |
| Illia | Destruction, Restoration, Alteration, Conjuration | Không (bỏ Forsworn Staff 2026-11-28: không cho NPC gậy phép) | Không | Không |
| Adelaisa Vendicci | Enchanting, Restoration, 1H | Steel Cutlass (IW, thuyền trưởng) | Không | Không |
| Brelyna, Faralda, Colette, Mirabelle | Phép | Không (người dùng: không cho NPC gậy phép, level cao tự cast mạnh hơn) | Không | Không |

## Trạng thái (2026-11-28)
- Đợt đổi đồ SMP + ELLE + College (2026-11-28): chưa test trong game. Mod ELLE không kèm mesh cơ thể: phải build trong
  BodySlide (Build Morphs). Đã tắt trong MO2 các mod không còn ai dùng (14 mod ELLE, Skyforge Assassins, Guardian Goddess,
  West Wind, Draugr Halloween, Yokuda) và ẩn bản lọc của chúng.
- Đợt trước: đã test trong game (~20 NPC ổn).
- Mọi mod đã cài, bật trong MO2. File chia đồ đi kèm mod đã ẩn (`.mohidden`).
- 42 plugin trang phục đã lọc bằng `strip_outfit_plugin.py`, nằm ở mod MO2 `SpouseOutfits - Clean Plugins` (không đưa lên GitHub).
- Chọn món cụ thể cho từng NPC: `outfits.json` (FormID nội bộ + tên). Build:
  `py -I skyrim-mods\tools\build_spouse_outfits.py --mo2 "E:\Skyrim Modding\MO2" --data "<game>\Data" --out "<MO2>\mods\SpouseOutfits"`
  ra `SpouseOutfits.esp` (ESL, 49 OTFT + 9 ARMO ghi đè giáp nặng, không đưa lên GitHub) + `SpouseOutfits_DISTR.ini` (bản copy ở thư mục này).
  Tool kiểm mọi FormID với plugin thật, chặn món chiếm slot 30/31/41.
- OBody NG: build tool ghi `SKSE\Plugins\OBody\obody_includes\SpouseOutfits_OBody.json` (bản copy ở thư mục này),
  gán theo npcFormID (ưu tiên cao nhất). Từ 2026-11-28 chỉ còn **Aduri** (CBBE Slim (Outfit), CBBE Athletic, khóa
  `obody_presets` của NPC) vì Mage's Daily Outfit clip/kéo giãn với ngực to. Mọi NPC khác dùng config OBody của người dùng
  (người dùng test thấy ổn). Muốn gán nhóm chung lại: thêm khóa `presets` vào `obody` trong `outfits.json`.
  NPC đã có body thì giữ body cũ tới khi Re-roll trong menu OBody.
- Tự quyết khi build: Jordis bộ COCO 02 (bạc); Hilund thêm giày+găng Steel Plate vanilla, Argis giày+găng Dwarven;
  Camilla/Bolfrida thêm Deze Crafter Boots; bỏ áo choàng SMP và mặt nạ; người dùng cung/nỏ có thêm 30 tên/bolt.
