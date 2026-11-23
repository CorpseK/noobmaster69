# GenderShift

SKSE plugin: đổi tỉ lệ nam/nữ của các NPC **spawn ngẫu nhiên từ leveled list** (bandit, Forsworn,
Vigilant, Thalmor patrol, mercenary...). Không có ESP.

## Tính năng
- Hook `TESLeveledList::CalculateCurrentFormList` (Address Library ID SE 14579 / AE 14751), là hàm game dùng để
  chọn một NPC từ leveled list (LVLN) khi một actor kiểu leveled được tạo ra.
- Sau khi game chọn xong, mod quyết định giới tính muốn có. Nếu NPC được chọn sai giới tính thì đổi sang một NPC
  **khác giới nằm trong cùng leveled list đó** (kể cả sublist):
  - cùng race (Breton vẫn là Breton);
  - cùng class (`bSameClass=1`: lính kiếm vẫn là lính kiếm, cung thủ vẫn là cung thủ);
  - level của entry ≤ level của NPC gốc → không bao giờ spawn ra địch mạnh hơn bản gốc.
- Không có bản khác giới thì giữ nguyên. Ví dụ **Briarheart** chỉ có bản nam → luôn là nam.
- NPC unique hoặc NPC đặt sẵn (không qua leveled list) **không bao giờ** bị đụng tới.
- Hai chế độ (`iMode`):
  - `0`: mỗi NPC tung xúc xắc riêng theo `fFemaleChance`.
  - `1` (mặc định): **theo nhóm**. Các NPC spawn cách nhau không quá `iGroupWindowMs` dùng chung một kết quả,
    nên cả trại load cùng lúc sẽ **toàn nam hoặc toàn nữ**.
- `fFemaleChance=100` thì mọi nơi đều nữ, `0` thì đều nam.
- Không lưu gì vào save. NPC đã spawn thì giữ nguyên tới khi cell reset. Gỡ mod thì những lần spawn sau trở lại như
  vanilla.

### Giới hạn / cần biết
- Chế độ nhóm dựa vào **thời điểm spawn**, không dựa vào vị trí. Khi fast travel, cả vùng 5×5 cell load một lượt,
  nên hai trại gần nhau có thể ra cùng giới tính. Trong interior (hang Forsworn) thì mỗi lần vào cell là một nhóm.
- Actor đã được resolve từ trước (trại bạn đã ghé, chưa reset) giữ giới tính cũ. Muốn thấy hiệu quả thì tới trại
  **chưa từng vào**, hoặc chờ cell reset (10 ngày trong game, hoặc 30 ngày nếu đã clear), hoặc dùng `resetinterior`
  cho interior.
- Mod chỉ đổi được khi leveled list có sẵn cả hai giới tính. Mod NPC overhaul có list riêng thì cũng áp dụng như vậy.
- NPC dùng template "Use Traits" trỏ tới một leveled list khác (giới tính chỉ biết lúc spawn) thì bị bỏ qua.

## Cài
Copy `SKSE/Plugins/GenderShift.dll` và `SKSE/Plugins/GenderShift.ini` vào `Data/`.

## Cần test in-game
Bật `bVerboseLog=1` khi test. Log nằm ở `Documents/My Games/Skyrim Special Edition/SKSE/GenderShift.log`.

- [ ] Log có `Hooked CalculateCurrentFormList at 0x...` và dòng `Config: ...` đúng với INI.
- [ ] **Quan trọng nhất:** vào một trại bandit/Forsworn **chưa từng ghé**. Log có dòng
      `xxxxxxxx 'Forsworn' -> yyyyyyyy 'Forsworn' (female)`. Nếu không có dòng nào thì có thể game không chọn
      leveled actor qua hàm này → báo lại kèm log.
- [ ] `iMode=1`, `fFemaleChance=50`: cả trại cùng một giới tính (trừ boss chỉ có một giới, vd. Briarheart).
      Log có `New group: female/male` mỗi lần vào trại.
- [ ] `fFemaleChance=100`: trại bandit toàn nữ. `fFemaleChance=0`: toàn nam.
- [ ] `iMode=0`: trong trại có lẫn cả nam lẫn nữ.
- [ ] Briarheart vẫn là nam. NPC có tên (Jarl, chủ quán, follower) không đổi.
- [ ] NPC bị đổi có mặt đúng, không bị **mặt đen/cổ lệch màu** (bản nữ dùng FaceGen vanilla của nó).
- [ ] Vũ khí/giáp/level của NPC bị đổi hợp lý (bản cung thủ vẫn cầm cung, không mạnh hơn bản gốc).
- [ ] Động vật (sói, gấu) không bị ảnh hưởng (`bOnlyHumanoid=1`).
- [ ] Đồ trong rương/loot leveled vẫn bình thường (hook chỉ xử lý leveled list nhân vật).
- [ ] Save, thoát game, load lại: NPC đã spawn giữ nguyên giới tính.
- [ ] Gỡ dll giữa chừng: load save không lỗi, các trại mới spawn lại như vanilla.
- [ ] Để ý FPS/khựng khi vào cell lớn: lần đầu gặp một list, mod duyệt cây list một lần rồi cache lại.

## Lịch sử
- 0.1.0: bản đầu.
