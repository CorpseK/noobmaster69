# Brief cho chat Claude Code chạy trên máy nhà: SpouseOutfits (SPID trang phục + vũ khí)

Đọc file này trước, rồi đọc `skyrim-mods/SpouseOutfits/PLAN.md` (danh sách CHỐT: ai mặc gì, cầm gì, base ID).
Brief chung của repo: `skyrim-mods/BRIEF.md`.

## Bối cảnh
- Game: Skyrim AE (1.6.1170, sắp lên 1.7.x), MO2 ở `E:\Skyrim Modding\MO2`. Body **CBBE 3BA**, **không SMP**.
- Đã có: SPID, Somewhere in Between (Dayhawk, replacer áo giáp/quần áo vanilla), Guards Armor Replacer (NordwarUA),
  Heavy Armory, Immersive Weapons, Legacy of the Dragonborn, mod AIO đổi mọi housecarl thành nữ (FormID giữ nguyên).
- Mục tiêu: mỗi NPC nữ trong PLAN.md (nhóm vợ, follower/housecarl, NPC kết bạn qua favor) có trang phục riêng +
  vũ khí theo kỹ năng, chia bằng **một** file SPID. Không có ESP mới trừ một plugin ESL nhỏ chứa Outfit (nếu cần).
- Người dùng chỉ muốn đồ **tồn tại trong game và temper được**: bỏ công thức chế, đồ đặt ngoài thế giới, phần chia đồ
  cho NPC đi kèm mod, chèn vào leveled list.

## Việc cần làm (theo thứ tự)
1. **Kiểm kê mod đã cài** trong MO2: với mỗi mod trong PLAN.md, tìm thư mục mod trong `E:\Skyrim Modding\MO2\mods\`,
   ghi lại tên plugin (`.esp/.esl/.esm`) đang bật (xem `profiles\<profile>\plugins.txt`). Báo mod nào thiếu.
   - Deze Clothing/Armor phải là bản **without NPCs**.
   - Mod có nhiều plugin để chọn: dùng đúng plugin đang bật.
2. **Ẩn file chia đồ đi kèm mod** (đổi tên đuôi hoặc MO2 hide): `*_DISTR.ini`, `*_KID.ini`, `*_SWAP.ini`,
   `SKSE\Plugins\SkyPatcher\**`, file Container Distribution Framework. Liệt kê cho người dùng trước khi ẩn.
3. **Dọn plugin**: chạy `skyrim-mods/tools/strip_outfit_plugin.py` cho từng plugin trang phục (KHÔNG chạy cho Heavy
   Armory/Immersive Weapons/LotD/GAR):
   ```
   py -I skyrim-mods\tools\strip_outfit_plugin.py --dry-run "<mod>\Plugin.esp"
   py -I skyrim-mods\tools\strip_outfit_plugin.py "<mod>\Plugin.esp" "<MO2>\mods\SpouseOutfits - Clean Plugins\Plugin.esp"
   ```
   Giữ: record mới, công thức temper, override ARMO/ARMA/TXST/OTFT nhắm vào mod không phải vanilla (bản 3BA conversion).
   Bỏ: công thức chế, CELL/WRLD, QUST/DIAL/SCEN, mọi override record game. Xem `--dry-run` trước, có gì lạ thì hỏi.
   Mod "SpouseOutfits - Clean Plugins" đặt **dưới** các mod gốc trong MO2 (ghi đè).
4. **Đọc FormID** (dùng `skyrim-mods/tools/esp.py`): ARMO (tên, slot) và OTFT có sẵn của từng mod; WEAP/ARMO khiên của
   Heavy Armory, Immersive Weapons; vũ khí vanilla lấy từ UESP. Vũ khí/khiên chính xác ở bảng "Vũ khí" trong PLAN.md.
5. **Tạo Outfit** cho NPC nào mod không có OTFT phù hợp: viết plugin ESL nhỏ `SpouseOutfits.esp` (cờ ESL) chứa `OTFT`,
   mỗi NPC một Outfit, chỉ gồm các món PLAN.md chỉ định. Lưu ý:
   - Taarie (Obi's Masquerade): **không có mặt nạ**.
   - Voada: chỉ "bộ mặc trong" của Wayward Ranger, màu khác Annekke (Annekke mặc đủ giáp).
   - Jordis (Goddess of War V2): có thể bỏ món áo choàng nếu trông cứng.
   - Aranea (Obi's Hood and Steel): mặt nạ/thắt lưng bỏ bằng zap trong BodySlide.
   Tool ghi ESP chưa có: viết thêm vào `skyrim-mods/tools/` (định dạng record xem `esp.py`, `strip_outfit_plugin.py`).
6. **Viết `SpouseOutfits_DISTR.ini`** (SPID, đặt trong mod riêng `SpouseOutfits`):
   - `Outfit = <outfit FormID>~<plugin>|NONE|<base FormID NPC>~<plugin NPC>` cho từng NPC (base ID trong PLAN.md;
     follower lấy từ UESP). Hilund, Morwen thuộc `Dragonborn.esm`.
   - `Item = <weapon FormID>~<plugin>|NONE|<NPC>|NONE|NONE|1` cho vũ khí, khiên, cung/nỏ.
   - Không dùng đồ unique của museum (Grimsever, vũ khí Thane...).
7. Push các file text (ini, tool, README) lên `main` của `CorpseK/noobmaster69`, thư mục `skyrim-mods/SpouseOutfits/`.
   **Không** đưa plugin của tác giả mod (bản gốc hay bản đã lọc) lên GitHub.

## Ghi chú quyết định của người dùng (đừng đề xuất lại)
- Không: bikini, đồ hiện đại, đồ từ game/phim khác, replacer, mod bắt buộc FSMP, váy/tà dài mỏng (thiếu SMP thì cứng),
  Fashions of the Huntsmen (để dành dùng việc khác), Nordic Fur Armor, mod follower (vd. Lady Arkay).
- NSFW chỉ cho 3 nữ tu Dibella (Anwen, Orla, Senna – đều đồ của contentcat).
- Shahvee, Wujeeta (Argonian): bỏ qua.
- COCO/Daymarr chạy với CBPC (không cần SMP); Skyforge Assassins không SMP vẫn ổn (người dùng đã kiểm tra).

## Việc khác đang chờ (không thuộc SpouseOutfits)
- DOMTalkGate: test, gửi `DOMTalkGate.log` (`Gated 37 DOM opening line(s)`).
- Diary of Mine: đo delay capture có clone bằng `Papyrus.0.log` (bật verbose trong MCM DOM + Papyrus logging).
