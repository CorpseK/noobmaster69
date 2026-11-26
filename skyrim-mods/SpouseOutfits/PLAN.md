# SpouseOutfits: trang phục riêng cho vợ (nữ, không phải follower)

Mục tiêu: mỗi ứng viên kết hôn nữ **không phải follower** có một bộ đồ hợp phong cách/cốt truyện, chia bằng **SPID**
(file `_DISTR.ini`, lọc theo base FormID của từng NPC). Đây là bản kế hoạch, chưa có file SPID.

## Ràng buộc (đã chốt với người dùng)
- Body **CBBE 3BA**, **không SMP** (dễ crash khi đánh đông người). Loại mod bắt buộc FSMP.
- Đã có **Somewhere in Between** của Dayhawk (Armor Replacer 98945 + Clothes Replacer 167530): không đề xuất lại.
  Vì SIB là replacer mesh vanilla, các mod chỉ **thêm biến thể màu dùng mesh vanilla** (series *Styles of Skyrim*)
  sẽ tự ăn mesh 3BA của SIB. Đây là lựa chọn tốt nhất cho đồ thường.
- Chỉ chọn đồ **mới, standalone**, không chọn replacer (tránh đè SIB).
- **NSFW chỉ cho 3 nữ tu Dibella** (Anwen, Orla, Senna). Mọi người khác: SFW, lore-friendly.

## Danh sách hiện tại (sau vòng 3, ưu tiên Deze)
Deze Clothing Set CBBE 3BA (50446) và Deze Armor Set (49318) của ralfetas: đồ theo nghề (Blacksmith, Alchemist,
Crafter boots, Dealer cho Ysolda, nhiều bộ Mage; giáp Thieves Guild/Steel Plate/Whiterun/Dwemer).
Cài bản **"without NPCs"** để SPID quyết định. Nếu cài thêm Deze SPIDified (57293) thì nó cũng gán Ysolda: dòng của
mình phải thắng. Các bộ COCO ghi "HDT-SMP hoặc CBPC" nên chạy với CBPC của 3BA.

| NPC | Base ID | Bộ đồ (ID Nexus) | Ghi chú |
|---|---|---|---|
| Anwen | 00013386 Skyrim.esm | Priestess of First Light 3BA (158302) | NSFW. Bản gốc SMP, kiểm tra bản 3BA |
| Orla | 000133B0 Skyrim.esm | COCO Fairy Queen (82260) | NSFW |
| Senna | 000133B7 Skyrim.esm | **COCO Caress of Venus (68006)**, đã chốt | NSFW |
| Taarie | 000132AB Skyrim.esm | COCO Scarlet Rose (65109) | |
| Dravynea | 0001365F Skyrim.esm | **Deze Mage** (50446), chọn bộ court wizard/College | Thay Snow Queen |
| Muiri | 0001406B Skyrim.esm | **CHỐT: Skyforge Assassins (87465)** | Váy ren đen; SMP hoặc CBPC |
| Avrusa Sarethi | 00019BFE Skyrim.esm | **Deze Alchemist** + Crafter boots (50446) | Khác màu/biến thể với Muiri nếu có |
| Ghorza gra-Bagol | 0001339A Skyrim.esm | **Deze Blacksmith** + Crafter boots (50446) | |
| Morwen | 018FA9 Dragonborn.esm | **CHỐT: ELLE Tribal Scout 3BA (95333)** | Bộ lạc, hợp Skaal |
| Sylgja | 000C3A3F Skyrim.esm | **Deze Blacksmith/Alchemist apron** + Crafter boots (50446) | Thợ mỏ |
| Ysolda | 00013BAB Skyrim.esm | **Deze Dealer** (50446) | Tác giả làm riêng cho Ysolda |
| Hilund | 01A515 Dragonborn.esm | **Deze Armor – Steel Plate A (có váy)** (49318) | Đổi từ Bladedancer's Edge theo yêu cầu |
| Temba Wide-Arm | 000658D2 Skyrim.esm | Knight of The Village 3BA (62364) | |
| Camilla Valerius | 0001347B Skyrim.esm | Obi's Skirt and Shirt 3BA (128400) | |
| Viola Giordano | 00014129 Skyrim.esm | DD - Temio Cloth 3BA (125970) | |
| Aeri | 0001360B Skyrim.esm | **CHỐT: C5Kev's Flirty In Fur (64072)** | Áo lông + váy da, hở eo |
| Gilfre | 0001367A Skyrim.esm | Toughened Traveler Outfit (gốc 54471 + 3BA 54584) | Áo khoác da đi đường, kín. Phương án 2: Ark bản xanh (0x12C6 / 0x22F8) |
| Shahvee | 0001411A Skyrim.esm | Tạm bỏ qua | Argonian |

Loại sau khi xem ảnh: Swan, Elven Lake (kiểu bikini); Clothes Pack 4, Wayward Knight, Gothic Babe (hiện đại/anime);
Obi's Leather Outfit, Shino Tribal Huntress (quá hở). Loại vì bắt buộc SMP: Lady Ritual, Tribal Clothes, Silver Knight,
Priestess of Time, Witch Chains Dress, Robe of the Sorceress, Dibella's Grace, Queen Marika, Elegant Merchant Clothes.
Loại vì là replacer: Icy's 3D Blacksmith/Merchant/Wench. Vòng 3 loại: Goma Fine Clothes (bắt buộc SMP),
DD Exquisite Cloth (hiện đại), Vintage Roses, COCO Luscious Lady (anime/hầu gái).

## Dọn mod trang phục (chỉ giữ đồ + temper)
Người dùng chỉ muốn đồ tồn tại trong dữ liệu game và temper được. Mỗi plugin chạy qua
`tools/strip_outfit_plugin.py` (giữ FormID), đặt bản sạch trong một mod MO2 riêng, ưu tiên cao hơn mod gốc.
Ngoài ra xóa/ẩn trong MO2 các file chia đồ đi kèm mod: `*_DISTR.ini` (SPID), `*_KID.ini`, `SKSE/Plugins/SkyPatcher/**`,
file Container Distribution Framework, `*_SWAP.ini`. Deze: chọn bản **without NPCs** khi cài.

## Việc tiếp theo
1. Người dùng duyệt danh sách (xem ảnh trên Nexus), chọn bộ cho từng NPC.
2. Tải mod, up plugin `.esp/.esl` lên Drive.
3. Đọc plugin lấy FormID Armor/Outfit. Mod nào không có Outfit thì tạo plugin ESL nhỏ chứa `OTFT`
   (mỗi NPC một Outfit; có thể trỏ vào leveled list để có biến thể).
4. Viết `SpouseOutfits_DISTR.ini`, lọc theo base ID ở bảng trên.

# Follower nữ (housecarl, hireling, follower lẻ)
Phạm vi (người dùng chốt): housecarl (có mod AIO đổi **mọi** housecarl thành nữ), hireling nữ, follower lẻ.
Follower thuộc phe lớn (Companions, College, Dawnguard...) giữ đồng phục phe trong game, không làm.
Quy tắc như nhóm vợ: 3BA, không SMP (COCO/Daymarr chạy với CBPC), không bikini/hiện đại/crossover, không lặp mod của vợ.

| Follower | Nơi | Đề xuất (ID Nexus) | Ghi chú |
|---|---|---|---|
| Lydia | Whiterun | Deze Armor – **Whiterun** bản A (49318) | Tác giả làm cho Irileth (Whiterun) |
| Argis the Bulwark | Markarth | Deze Armor – **Dwemer** (49318) | Tác giả làm cho Faleen (Markarth) |
| Iona | Riften | ELLE – Ginnungagap 3BA (97931) | Da tối, mặt nạ, hợp Riften |
| Jordis the Sword-Maiden | Solitude | COCO Goddess Of War V2 (78335), bản bạc | Imperial Centurion Battle Dress bị loại (SMP skirt) |
| Calder | Windhelm | Nord Steelheart (110922 + 3BA 111014) | Lông + thép, hợp Eastmarch lạnh |
| Gregor | Dawnstar | Sovngarde Steel Armor (76115, file 3BA) | Thép Nord |
| Valdimar | Morthal | Iron Battlemage Armor 3BA (73162) | Valdimar là spellsword (Alteration/Destruction/Restoration) |
| Rayya | Falkreath (Redguard) | **CHỐT: Daymarr Yokuda (142696)** | Chủ đề Redguard/Hammerfell; hở; SMP hoặc CBPC |
| Jenassa | Hireling, Whiterun | West Wind – Assault Armor 3BA (128266) | Lính đánh thuê kiểu sát thủ |
| Mjoll the Lioness | Riften | Daymarr Guardian Goddess (137926) | Paladin vàng-trắng |
| Uthgerd the Unbroken | Whiterun | Deze Armor – **Steel Plate B** (49318) | Tác giả làm cho Uthgerd |
| Annekke Crag-Jumper | Darkwater Crossing | Wayward Ranger Armor 3BA, đủ giáp (124246) | Ranger |
| Eola | Markarth (Namira) | **Draugr Halloween / "Draugr Wanderer" Outfit** (41883, file 3BAv2) | Vải mục, xương kiểu draugr: hợp Namira (mục rữa, sâu bọ). Phương án 2: Daymarr Shadow Knight (119080) |
| Aranea Ienith | Shrine of Azura | **CHỐT: COCO Twilight Sorceress (102730)** | Chủ đề twilight (Azura = hoàng hôn/bình minh). Hở; SMP hoặc CBPC |
| Illia | Darklight Tower | **CHỐT: Wilderness Witch** (gốc 40963 + 3BA 40845) | Phù thủy rừng |
| Serana | Volkihar | **CHỐT: ELLE Dark Rebel 3BA (102957)** | Giáp nhẹ + áo khoác tối |
| Frea | Skaal | Dragonbone Barbarian 3BA (47466, cần bản gốc của Shocky) | Lông + xương rồng, chiến binh phương Bắc thay vì đồ giống cướp |
| Borgakh the Steel Heart | Mor Khazgur | Orcish Marauder Armor 3BA (64629) | Warrior giáp nặng; vẫn chất Orc nhưng là bộ mới, không phải giáp Orcish vanilla |
| Ugor | Largashbur | C5Kev's Lacey Ranger Armor 3BA (103426) | Ranger (bắn cung, giáp nhẹ) |

Loại vòng follower: Daring Tribunal Robes (ngực trần), Brazen Assassin, Dangerous Gladiator, Black Widow,
Daggerfall Archmage (kiểu bikini/dây), Dovakini (anime), Ebony Armor Kai (SMP), Champion of Azura (latex),
Moon Feather Shaman (quá hở), Volkihar Assassin (FSMP, lại là vampire), Sorceress Armor (SMP), Daring Stormhold Warrior (hở ngực).

# Vũ khí
Nguồn: trang Armory (SSE) của wiki LotD (legacy-of-the-dragonborn.fandom.com): vũ khí vanilla, **Heavy Armory (HA)**
và **Immersive Weapons (IW)** theo chất liệu. Chọn theo nghề, quê quán, chất liệu bộ giáp.
Lưu ý: SPID chỉ thêm vũ khí vào túi; NPC tự chọn vũ khí theo kỹ năng/AI, có thể vẫn dùng vũ khí cũ nếu nó mạnh hơn.
Đưa vũ khí cho follower thì người chơi lấy lại được, và bản đó tính cho phòng Armory của museum.

| Nhân vật | Vũ khí | Nguồn | Lý do |
|---|---|---|---|
| Aeri | Iron Hatchet | HA | Rìu nhỏ của thợ đốn gỗ |
| Gilfre | Iron Double Axe | IW | Rìu hai lưỡi to, chủ xưởng gỗ |
| Temba Wide-Arm | Iron Hunter's Spear + Hunting Bow | IW + vanilla | Săn gấu |
| Sylgja | Nordic War Pick | IW | Cuốc chiến cho thợ mỏ |
| Avrusa Sarethi | Glass Dagger | vanilla | Dunmer, chất liệu Morrowind |
| Muiri | Forsworn Dagger | HA | Sống ở Markarth (xứ Reach) |
| Camilla Valerius | Imperial Dagger | HA | Imperial |
| Ysolda | Iron Shortsword | HA | Lái buôn đi đường |
| Taarie | Elven Shortsword | HA | Altmer, thanh lịch |
| Viola Giordano | Imperial Shortsword | HA | Imperial, đi tuần |
| Ghorza gra-Bagol | Orcish Maul | HA | Thợ rèn Orc |
| Morwen | Stalhrim Hatchet | HA | Thợ rèn Skaal, chất liệu Solstheim |
| Hilund | Nordic Long Mace | HA | Gốc dùng chùy Nordic; Nordic là chất liệu Solstheim |
| Dravynea, Anwen, Orla, Senna | Không | | Pháp sư / nữ tu |
| Lydia (Whiterun) | Steel Longsword + khiên | IW | Phương án: The Sword of Whiterun (vũ khí Thane của LotD) |
| Argis (Markarth) | Eastern Dwemer Sword | IW | Hợp giáp Deze Dwemer |
| Iona (Riften) | Nord Hero Shortsword | HA | Hợp Ginnungagap kiểu Norse |
| Jordis (Solitude) | Silver Longsword | IW | Hợp giáp bạc |
| Calder (Windhelm) | Nord Hero Double Axe | IW | Giáp nặng Nord |
| Gregor (Dawnstar) | Steel Long Mace | HA | Vũ khí Thane của Dawnstar cũng là chùy |
| Valdimar (Morthal) | Steel Battle Staff | IW | Spellsword / battlemage |
| Rayya (Falkreath) | Glass Scimitar | IW | Redguard + Yokuda |
| Jenassa | Elven Hawk Bow + Steel Tanto | IW | Cung thủ, sát thủ |
| Mjoll | Grimsever (giữ nguyên) | vanilla | Kiếm riêng của cô |
| Uthgerd | Steel Spiked Club | HA | Dân đấu tay đôi |
| Annekke Crag-Jumper | Steel War Pick + Hunting Bow | IW + vanilla | Mẹ của Sylgja, dân mỏ + ranger |
| Eola | Honed Ancient Nord Dagger | HA | Chủ đề draugr/mục rữa |
| Aranea Ienith | Glass Quarterstaff | IW | Ánh sao của Azura |
| Illia | Forsworn Staff | vanilla | Hagraven/Forsworn |
| Serana | Daedric Tanto | IW | Tối màu; tránh Silver (khắc chế vampire) |
| Frea | Stalhrim Spear | HA | Skaal, Solstheim |
| Borgakh | Orcish Saber | IW | Kiếm Orc gọn, hợp người rời stronghold |
| Ugor | Orcish Bow + Orcish Hatchet | vanilla + HA | Ranger |
