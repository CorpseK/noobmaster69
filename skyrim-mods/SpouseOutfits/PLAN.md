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

## Việc tiếp theo
1. Người dùng duyệt danh sách (xem ảnh trên Nexus), chọn bộ cho từng NPC.
2. Tải mod, up plugin `.esp/.esl` lên Drive.
3. Đọc plugin lấy FormID Armor/Outfit. Mod nào không có Outfit thì tạo plugin ESL nhỏ chứa `OTFT`
   (mỗi NPC một Outfit; có thể trỏ vào leveled list để có biến thể).
4. Viết `SpouseOutfits_DISTR.ini`, lọc theo base ID ở bảng trên.
