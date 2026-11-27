# SpouseOutfits: trang phục + vũ khí cho NPC nữ (bản CHỐT)

Chia bằng **SPID** (`SpouseOutfits_DISTR.ini`, lọc theo base FormID của từng NPC). Bảng dưới là bản chốt cuối cùng;
lịch sử lựa chọn xem trong git log của file này.

## Ràng buộc
- Body **CBBE 3BA**, **không SMP**. Mod bắt buộc FSMP bị loại. Váy/tà dài mềm cũng loại (không SMP thì cứng đờ),
  trừ các bộ người dùng đã kiểm tra là ổn (Skyforge Assassins, Guardian Goddess, Goddess of War V2: có CBPC).
- Đã có **Somewhere in Between** (Dayhawk) và **Guards Armor Replacer** (NordwarUA): không đè, không đề xuất lại.
- Chỉ đồ **mới, standalone**; không replacer. Không bikini, không đồ hiện đại, không đồ từ game/phim khác.
- **NSFW chỉ cho 3 nữ tu Dibella** (Anwen, Orla, Senna).
- Mỗi mod trang phục chạy qua `tools/strip_outfit_plugin.py`: chỉ giữ đồ + công thức temper. Ẩn trong MO2 các file chia
  đồ đi kèm mod (`*_DISTR.ini`, `*_KID.ini`, SkyPatcher, Container Distribution Framework, `*_SWAP.ini`).
  Deze: cài bản **without NPCs**.

## Nhóm vợ (ứng viên kết hôn nữ, không phải follower)
| NPC | Base ID | Trang phục (ID Nexus) | Ghi chú |
|---|---|---|---|
| Anwen | 00013386 | Hezimao 42 Dress (110988) | NSFW. Váy ren xuyên thấu, ngắn |
| Orla | 000133B0 | Korana Priestess Of Dibella (112973, file 3BA) | NSFW |
| Senna | 000133B7 | Obi's Eclipse Seductress 3BA (111725) | NSFW |
| Taarie | 000132AB | Obi's Masquerade Outfit 3BA (116474) | **Không dùng mặt nạ** |
| Dravynea | 0001365F | Deze Clothing – Mage (50446), bộ court wizard/College | |
| Muiri | 0001406B | Skyforge Assassins (87465) | Váy dài chưa tới gối, không SMP vẫn ổn |
| Avrusa Sarethi | 00019BFE | Deze Clothing – Alchemist + Crafter boots (50446) | |
| Ghorza gra-Bagol | 0001339A | Deze Clothing – Blacksmith + Crafter boots (50446) | |
| Morwen | 018FA9 (Dragonborn.esm) | ELLE Tribal Scout 3BA (95333) | |
| Sylgja | 000C3A3F | Deze Clothing – tạp dề (Blacksmith/Alchemist) + Crafter boots (50446) | |
| Ysolda | 00013BAB | Deze Clothing – Dealer (50446) | Tác giả làm riêng cho Ysolda |
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
| Lydia (Whiterun) | | Deze Armor – Whiterun A (49318) | |
| Argis the Bulwark (Markarth) | | Deze Armor – Dwemer (49318) | |
| Iona (Riften) | | ELLE – Ginnungagap 3BA (97931) | |
| Jordis the Sword-Maiden (Solitude) | | COCO Goddess Of War V2 (78335), bản bạc | CBPC |
| Calder (Windhelm) | | Nord Steelheart (gốc 110922 + 3BA 111014) | |
| Gregor (Dawnstar) | | Sovngarde Steel Armor (76115, file 3BA) | |
| Valdimar (Morthal) | | Iron Battlemage Armor 3BA (73162) | |
| Rayya (Falkreath) | | Daymarr Yokuda (142696) | Hở; CBPC |
| Jenassa | | West Wind – Assault Armor 3BA (128266) | |
| Mjoll the Lioness | | Daymarr Guardian Goddess (137926) | CBPC |
| Uthgerd the Unbroken | | Deze Armor – Steel Plate B (49318) | |
| Annekke Crag-Jumper | | Wayward Ranger Armor 3BA, đủ giáp (124246) | |
| Eola | | Draugr Wanderer Outfit (41883, file 3BAv2) | |
| Aranea Ienith | | Obi's Hood and Steel Armor 3BA (136388) | Tím = Azura. Phương án 2: Ashtoreth Elven Sentinel (79296) bỏ áo choàng |
| Illia | | Wilderness Witch (gốc 40963 + 3BA 40845) | |
| Serana | | ELLE Dark Rebel 3BA (102957) | |
| Frea | | Dragonbone Barbarian 3BA (47466, cần bản gốc) | |
| Borgakh the Steel Heart | | Orcish Marauder Armor 3BA (64629) | |
| Ugor | | C5Kev's Lacey Ranger Armor 3BA (103426) | |
Base ID của follower sẽ lấy từ UESP khi viết SPID.

## NPC nữ kết bạn qua favor (rủ đi bằng Immersive Recruit)
Lọc từ UESP Skyrim:Favors: bỏ người có chồng/con, người già, người có việc quan trọng; bỏ Wujeeta (Argonian).
| NPC | Base ID | Trang phục (ID Nexus) | Vũ khí |
|---|---|---|---|
| Bolfrida Brandy-Mug | 00014126 | Nordic Fur Armor (gốc 26472 + 3BA 169079) | Iron Quarterstaff (HA) |
| Ganna Uriel | 0001365D | Fashions of the Huntsmen (75837, file CBBE 3BA) | Steel Hatchet (HA) |
| Voada | 000133C0 | Wayward Ranger – chỉ bộ mặc trong (124246), màu khác Annekke | Iron Dagger |
| Alessandra | 00013347 | Pandorable's Lady Arkay (28434, chỉ CBBE thường) | Silver Mace (HA) |
| Bagrak | 00019955 | Bladedancer's Edge (62751, file CBBE-3BA) | Orcish War Axe + Dwarven Bow |
| Yatul | 0001B077 | Hunting Grounds Outfit (bản gốc + 3BA 63986) | Orcish Shortsword (HA) + Supple Ancient Nord Bow |

# Vũ khí
Nguồn: trang Armory (SSE) của wiki LotD (legacy-of-the-dragonborn.fandom.com), kiểm tra chéo: món vanilla có trang
trên UESP, món Heavy Armory (HA) / Immersive Weapons (IW) có trang trên wiki LotD. Kỹ năng follower lấy từ UESP
(Skyrim:Followers). Quy tắc: có One-handed + Block → 1 tay + khiên; Two-handed → 2 tay; có Archery → thêm cung/nỏ;
pháp sư không có kỹ năng vũ khí → không cầm vũ khí cận chiến. Housecarl cầm khiên lính gác của Hold mình (vanilla).
Người dùng có **Guards Armor Replacer** (NordwarUA): nó thay hình đồ lính gác theo từng Hold, nên các khiên lính gác này
sẽ mang kiểu GAR của Hold đó, không cần thêm record. Markarth của GAR làm từ quặng Dwemer, hợp giáp Deze Dwemer của Argis.
Không dùng đồ unique trưng bày ở museum (Grimsever, vũ khí Thane...).
Lưu ý: SPID chỉ thêm vào túi; NPC tự chọn theo kỹ năng/AI. "Steel Crossbow" của LotD = "Crossbow" (Dawnguard) trên UESP.

## Nhóm vợ (không phải follower, chỉ vũ khí theo nghề; cung chỉ cho người đi săn)
| Nhân vật | Vũ khí | Nguồn |
|---|---|---|
| Aeri | Iron Hatchet | HA |
| Gilfre | Iron Double Axe | IW |
| Temba Wide-Arm | Iron Hunter's Spear + Hunting Bow | IW + vanilla |
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
| Dravynea, Anwen, Orla, Senna | Không | |

## Follower
| Follower | Kỹ năng chính (UESP) | Vũ khí chính | Khiên | Cung/nỏ |
|---|---|---|---|---|
| Lydia (Whiterun) | Heavy, 1H, Archery, Block | Steel Longsword (IW) | Whiterun Guard's Shield | Nord Hero Bow |
| Argis (Markarth) | Heavy, 1H, Archery, Block | Eastern Dwemer Sword (IW) | Markarth Guard's Shield | Dwarven Crossbow |
| Iona (Riften) | Heavy, 1H, Archery, Block | Nord Hero Shortsword (HA) | Riften Guard's Shield | Nordic Scrimshaw Bow (IW) |
| Jordis (Solitude) | Heavy, 1H, Archery, Block | Silver Longsword (IW) | Solitude Guard's Shield | Imperial Bow |
| Calder (Windhelm) | Heavy, 1H, Archery, Block | Nordic War Axe (vanilla) | Windhelm Guard's Shield | Nordic Bow |
| Gregor (Dawnstar) | Heavy, 1H, Archery, Block | Steel Long Mace (HA) | Pale Guard's Shield | Steel Crossbow |
| Rayya (Falkreath) | Heavy, 1H, Archery, Block | Glass Scimitar (IW) | Falkreath Guard's Shield | Glass Bow |
| Valdimar (Morthal) | Heavy, 1H, Alteration, Destruction, Restoration | Honed Ancient Nord War Axe | Không (không có Block) | Không (không có Archery) |
| Jenassa | Archery, Light, Block, 1H | Steel Tanto (IW) | Elven Shield | Elven Hawk Bow (IW) |
| Mjoll the Lioness | Heavy, **2H**, Archery, Block | Elven Great Blade (IW) | Không (2 tay) | Elven Bow |
| Uthgerd the Unbroken | Heavy, 1H, Archery, Block | Steel Spiked Club (HA) | Steel Shield | Ancient Nord Bow |
| Annekke Crag-Jumper | Archery, Light, Block, 1H | Steel War Pick (IW) | Hide Shield | Hunting Bow |
| Borgakh the Steel Heart | Heavy, 1H, Archery, Sneak | Orcish Saber (IW) | Không (không có Block) | Ebony Bow |
| Ugor | Archery, Light, Block, 1H | Orcish Hatchet (HA) | Orcish Shield | Orcish Bow |
| Eola | Destruction, Sneak, Alteration, 1H | Honed Ancient Nord Dagger (HA) | Không | Không |
| Serana | Sneak, Conjuration, Light, 1H | Daedric Tanto (IW) | Không | Không |
| Frea | 1H, Alteration, Light, Restoration | Stalhrim Shortspear (HA, 1 tay) | Không | Không |
| Aranea Ienith | Destruction, Restoration, Alteration, Conjuration | Không (phép) | Không | Không |
| Illia | Destruction, Restoration, Alteration, Conjuration | Forsworn Staff (gậy phép vanilla) | Không | Không |

## Việc tiếp theo
1. Người dùng tải đủ mod, build BodySlide, up plugin `.esp/.esl/.esm` (đúng bản đang bật) lên Drive, kèm Heavy Armory
   và Immersive Weapons.
2. Chạy `strip_outfit_plugin.py` cho từng mod trang phục, trả bản sạch lên Drive.
3. Đọc FormID; mod nào không có Outfit thì tạo plugin ESL nhỏ chứa `OTFT`.
4. Viết `SpouseOutfits_DISTR.ini` cho cả 3 nhóm (trang phục + vũ khí).
