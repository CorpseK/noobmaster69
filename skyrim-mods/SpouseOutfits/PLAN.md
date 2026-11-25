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

## Danh sách NPC (nguồn: UESP Skyrim:Marriage + trang từng NPC)

| NPC | Base ID | Plugin | Vai trò / tính cách | Phong cách | Mod đề xuất (ID Nexus) |
|---|---|---|---|---|---|
| Aeri | 0001360B | Skyrim.esm | Nord thợ đốn gỗ, Anga's Mill | Đồ lao động mộc mạc | Common Clothing Expansion 3BA (134131, cần bản gốc 134088), Tops and Bottoms (136007) |
| Gilfre | 0001367A | Skyrim.esm | Imperial chủ Mixwater Mill, làm việc một mình | Lao động, màu Imperial | Tops and Bottoms (136007), Colovian Leather Outfit 3BA (50683) |
| Temba Wide-Arm | 000658D2 | Skyrim.esm | Nord chủ xưởng cưa Ivarstead, ghét gấu | Thợ săn, áo lông | Fashions of the Huntsmen (75837, file CBBE 3BA), Pelts 'o' Plenty (120726, file 3BA) |
| Sylgja | 000C3A3F | Skyrim.esm | Nord thợ mỏ Shor's Stone | Đồ thợ mỏ | Common Clothes and Armors 3BA (83980, cần bản gốc 21305) |
| Shahvee | 0001411A | Skyrim.esm | Argonian thợ thuộc da, Windhelm | Đồ lao động (beast race) | Styles of Skyrim (mesh vanilla nên hỗ trợ beast race), cần chọn |
| Avrusa Sarethi | 00019BFE | Skyrim.esm | Dunmer thầy thuốc, từng mở tiệm ở Vivec | Đồ Dunmer Morrowind | Styles of Skyrim – Dunmer Outfit Variations (125719) |
| Dravynea | 0001365F | Skyrim.esm | Dunmer pháp sư nguyên tố, Kynesgrove | Áo pháp sư | Mage Clothing Expansion 3BA (128838) |
| Muiri | 0001406B | Skyrim.esm | Breton thầy thuốc, bị phản bội, thuê Dark Brotherhood | Thầy thuốc u tối | Styles of Skyrim – Alchemist (128147, cần CC Necromantic Grimoire) |
| Camilla Valerius | 0001347B | Skyrim.esm | Imperial, Riverwood Trader, giữa hai người theo đuổi | Cô bán hàng | Styles of Skyrim – Merchant Clothes (143310), Merchant Dress 3BA (110120) |
| Ysolda | 00013BAB | Skyrim.esm | Nord lái buôn chợ Whiterun | Lái buôn | Styles of Skyrim – Merchant Clothes (143310), Common Clothing Expansion 3BA |
| Taarie | 000132AB | Skyrim.esm | Altmer thợ may Radiant Raiment, kiêu | Thời trang cao cấp | Ashtoreth Noble Maiden Dress 3BA (101750) |
| Viola Giordano | 00014129 | Skyrim.esm | Imperial ở Windhelm, đi tuần tìm Butcher | Phu nhân trung lưu | Styles of Skyrim – Fine Clothes Variations (126451) |
| Ghorza gra-Bagol | 0001339A | Skyrim.esm | Orc thợ rèn Markarth | Tạp dề thợ rèn | Cần tìm thêm (các bản tạp dề tìm được là BHUNP hoặc adult) |
| Hilund | 01A515 | Dragonborn.esm | Nord chiến binh Thirsk, giáp vảy | Giáp Nord | Nord Steelheart 3BA (111014, cần bản gốc 110922) |
| Morwen | 018FA9 | Dragonborn.esm | Skaal, học việc thợ rèn | Đồ bộ lạc Skaal | Nord Tribal Armor 3BA (83361) |
| Anwen | 00013386 | Skyrim.esm | Redguard nữ tu Dibella, giọng kiêu | **NSFW** | xem dưới |
| Orla | 000133B0 | Skyrim.esm | Nord nữ tu Dibella | **NSFW** | xem dưới |
| Senna | 000133B7 | Skyrim.esm | Breton nữ tu Dibella, giọng "sultry" | **NSFW**, hở nhất | xem dưới |

### Nữ tu Dibella (NSFW, 3BA, không SMP)
- **Korana Priestess Of Dibella** (112973): có file CBBE 3BA, 2 màu vàng/bạc, ESL, làm riêng cho Dibella.
- **Priestess of First Light 3BA** (158302): bản 3BA của bộ gốc BHUNP **SMP**. Cần mở file xem còn SMP không.
- Loại vì bắt buộc SMP: Dibella's Grace (174196), COCO Caress of Venus (68006), Queen Marika's Dress (83261),
  Elegant Merchant Clothes (148167), Traveling Mage (69407).

## Việc tiếp theo
1. Người dùng duyệt danh sách (xem ảnh trên Nexus), chọn bộ cho từng NPC.
2. Tải mod, up plugin `.esp/.esl` lên Drive.
3. Đọc plugin lấy FormID Armor/Outfit. Mod nào không có Outfit thì tạo plugin ESL nhỏ chứa `OTFT`
   (mỗi NPC một Outfit; có thể trỏ vào leveled list để có biến thể).
4. Viết `SpouseOutfits_DISTR.ini`, lọc theo base ID ở bảng trên.
