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

## Vòng 2: chọn từ mod adult trên Nexus (đã xem ảnh preview)
Người dùng thấy bản vòng 1 (Styles of Skyrim, Common Clothing...) quá giống đồ gốc, SIB đã đẹp hơn. Vòng 2 lấy từ
mod gắn cờ adult (thường đẹp hơn), loại bikini, đồ hiện đại, đồ từ game/phim khác, replacer, bản bắt buộc FSMP.
Các bộ **COCO** ghi "HDT-SMP hoặc CBPC" nên chạy được với CBPC của 3BA (không cần SMP).

| NPC | Base ID | Đề xuất (ID Nexus) | Lý do / mức hở |
|---|---|---|---|
| Anwen | 00013386 Skyrim.esm | Priestess of First Light 3BA (158302) | Vũ công xanh lá, hợp Redguard. **Hở**. Bản gốc là BHUNP SMP, cần xem file 3BA còn SMP không |
| Orla | 000133B0 Skyrim.esm | COCO Fairy Queen (82260) | Váy dài trắng/xanh kiểu elf. **Hở** |
| Senna | 000133B7 Skyrim.esm | COCO Caress of Venus (68006) hoặc Korana Priestess Of Dibella (112973) | Bộ Dibella kinh điển, xuyên thấu. **Hở nhất** |
| Taarie | 000132AB Skyrim.esm | COCO Scarlet Rose (65109, có "Clothing Version") | Đỏ-vàng kiểu elf quý tộc. Hở eo/đùi |
| Dravynea | 0001365F Skyrim.esm | COCO Snow Queen (94596) | Chủ đề băng, hợp việc giữ lạnh mỏ Steamscorch |
| Muiri | 0001406B Skyrim.esm | Skyforge Assassins (87465) | Váy ren đen "lore friendly", hợp quá khứ Dark Brotherhood |
| Avrusa Sarethi | 00019BFE Skyrim.esm | Hott Ildari 3BA (135244) | Đồ Dunmer (Ildari là nhân vật ESO, cùng vũ trụ TES) |
| Hilund | 01A515 Dragonborn.esm | Sovngarde Steel Armor (76115, file 3BA) / Bladedancer's Edge (62751) | Giáp thép Nord / giáp tối |
| Ghorza gra-Bagol | 0001339A Skyrim.esm | Obi's Wyrm Slayer Armor 3BA (103216) | Da + thép, kín, hợp thợ rèn Orc |
| Morwen | 018FA9 Dragonborn.esm | ELLE Tribal Scout 3BA (95333) | Bộ lạc, hợp Skaal |
| Temba Wide-Arm | 000658D2 Skyrim.esm | Knight of The Village 3BA (62364) | Áo blouse + corset da, mộc |
| Camilla Valerius | 0001347B Skyrim.esm | Obi's Skirt and Shirt 3BA (128400) | Blouse trễ vai + váy, cô gái làng |
| Ysolda | 00013BAB Skyrim.esm | Obi's Common Outfit 3BA (110678) | Áo choàng mũ trùm, lái buôn đi đường |
| Aeri, Gilfre, Sylgja | | Knight of The Village / Skirt and Shirt / Common Outfit | Chưa có bộ riêng, dùng chung nhóm "lao động" |
| Viola Giordano | 00014129 Skyrim.esm | (chưa có) | Cần tìm thêm |
| Shahvee | 0001411A Skyrim.esm | (chưa có) | Argonian: đồ 3BA hiếm khi hỗ trợ chân/đuôi beast race |

Loại sau khi xem ảnh: Swan, Elven Lake (kiểu bikini); Clothes Pack 4, Wayward Knight, Gothic Babe (hiện đại/anime);
Obi's Leather Outfit, Shino Tribal Huntress (quá hở). Loại vì bắt buộc SMP: Lady Ritual, Tribal Clothes, Silver Knight,
Priestess of Time, Witch Chains Dress, Robe of the Sorceress, Dibella's Grace, Queen Marika, Elegant Merchant Clothes.
Loại vì là replacer: Icy's 3D Blacksmith/Merchant/Wench.

## Việc tiếp theo
1. Người dùng duyệt danh sách (xem ảnh trên Nexus), chọn bộ cho từng NPC.
2. Tải mod, up plugin `.esp/.esl` lên Drive.
3. Đọc plugin lấy FormID Armor/Outfit. Mod nào không có Outfit thì tạo plugin ESL nhỏ chứa `OTFT`
   (mỗi NPC một Outfit; có thể trỏ vào leveled list để có biến thể).
4. Viết `SpouseOutfits_DISTR.ini`, lọc theo base ID ở bảng trên.
