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

## Gỡ lỗi
Đã test in-game (11/2026, AE 1.6.1170). Khi có vấn đề, bật `bVerboseLog=1` rồi xem
`Documents/My Games/Skyrim Special Edition/SKSE/GenderShift.log`:
- `Hooked CalculateCurrentFormList at 0x...` → hook đã cài.
- `New group: female/male` → mỗi lần một nhóm (trại) được tung xúc xắc.
- `xxxxxxxx '...' -> yyyyyyyy '...' (female)` → một NPC đã được đổi.
- `kept: no ... counterpart` → list không có bản khác giới, giữ nguyên (vd. Briarheart).

## Lịch sử
- 0.1.0: bản đầu.
