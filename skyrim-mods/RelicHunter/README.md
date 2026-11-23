# RelicHunter

SKSE plugin cho **Legacy of the Dragonborn** + **The Curator's Companion (TCC)**: loot random dễ ra relic
**còn thiếu** trong museum hơn. Không có ESP.

## Tính năng
- Hook `TESLeveledList::CalculateCurrentFormList` (Address Library ID SE 14579 / AE 14751), là hàm game dùng để
  bốc đồ từ leveled item list (LVLI). Rương, xác, đồ NPC mặc, hàng merchant đều đi qua đây.
- Sau khi game bốc một món: nếu món đó **không** phải relic còn thiếu, mà chính list đó (kể cả sublist) có relic còn
  thiếu ở level cho phép, thì có `fRerollChance`% đổi món đó thành một relic trong số ấy.
  - Chỉ ra những món **vốn có thể rơi từ list đó**. Artifact unique, đồ quest không nằm trong leveled list thì không
    bao giờ bị tạo ra.
  - `bSameType=1`: giáp đổi thành giáp, vũ khí đổi thành vũ khí, misc đổi thành misc. Bandit không mặc nhẫn thay mũ.
  - Không bao giờ ưu tiên món đã trưng bày.
- "Còn thiếu" lấy từ các FormList mà TCC giữ (cũng là list moreHUD đọc để hiện icon), trong
  `DBM_RelicNotifications.esp`:

  | List | FormID (local) | Nghĩa |
  |---|---|---|
  | `dbmMaster` | `0x609634` | mọi relic |
  | `dbmNew` | `0x558285` | chưa từng nhặt |
  | `dbmFound` | `0x558286` | đã nhặt, chưa trưng bày |
  | `dbmDisp` | `0x558287` | đã trưng bày |

  FormID lấy theo plugin mã nguồn mở grid-inventory, họ đã đo trong game thật. Nếu ID đổi ở bản TCC của bạn, mod
  thử tìm theo EditorID (cần **powerofthree's Tweaks**).
- Mặc định chỉ ưu tiên `dbmNew` (chưa từng nhặt). Món đã nhặt rồi thì thường đang nằm trong rương nhà bạn, ưu tiên
  thêm cũng phí. `bIncludeFound=1` để tính cả món đã nhặt mà chưa trưng bày.
- TCC cập nhật list bằng Papyrus, nên mod đọc lại list trên main thread (tối đa 3 giây/lần, chỉ khi kích thước list
  đổi).
- Không lưu gì vào save, thêm/gỡ lúc nào cũng được.

### Giới hạn / cần biết
- Chỉ tác động relic **nằm trong leveled list**: phần lớn Armory (bộ giáp/vũ khí các loại), đá quý, đồ từ các mod có
  patch thêm vào leveled list. Relic quest/unique vẫn phải đi lấy như thường.
- Rương/xác **đã sinh đồ rồi** thì không đổi. Hiệu quả thấy ở rương mới, NPC mới spawn, hàng merchant lần refresh sau.
- Dùng chung hook với GenderShift, Detours tự nối hai hook nên cài cả hai vẫn được.
- `fRerollChance` cao quá (vd. 100) sẽ làm loot gần như toàn relic, nhất là list hay rơi đồ Armory.

## Cài
Copy `SKSE/Plugins/RelicHunter.dll` và `SKSE/Plugins/RelicHunter.ini` vào `Data/`.

## Cần test in-game
Bật `bVerboseLog=1` khi test. Log nằm ở `Documents/My Games/Skyrim Special Edition/SKSE/RelicHunter.log`.

- [ ] Log có `Hooked CalculateCurrentFormList`, và 4 dòng `dbmMaster/dbmNew/dbmFound/dbmDisp found`.
      Có dòng `NOT FOUND` thì báo lại kèm bản TCC đang dùng.
- [ ] Sau khi load save có dòng `Relic lists: master=... new=... found=... disp=... -> N wanted`.
      `master` xấp xỉ `new + found + disp`, khớp với số trong MCM của TCC.
- [ ] Trưng bày một món trong museum, đi loot tiếp: dòng `Relic lists` mới có `disp` tăng 1.
- [ ] Đặt tạm `fRerollChance=100`, mở vài rương dungeon **chưa mở**. Log có
      `LVLI xxxxxxxx: ... -> relic ...` và trong rương có relic với icon moreHUD "new".
      Không thấy dòng nào thì báo lại (có thể rương sinh đồ qua đường khác).
- [ ] Giết bandit mới spawn: đồ trên xác có relic (vd. giáp Armory) hợp lý, đúng loại (`bSameType=1`).
- [ ] Merchant (vd. Belethor) sau khi refresh hàng có thêm relic.
- [ ] Không có relic quest/unique tự dưng xuất hiện.
- [ ] Gỡ TCC (hoặc tắt `DBM_RelicNotifications.esp`): log báo `not loaded -> RelicHunter does nothing`, game bình thường.
- [ ] Gỡ dll giữa save: load lại không lỗi.

## Lịch sử
- 0.1.0: bản đầu.
