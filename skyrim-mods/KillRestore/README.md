# KillRestore

SKSE plugin: mỗi lần **player** giết một actor thì hồi một phần Health / Magicka / Stamina. Không có ESP.

## Tính năng
- Nghe sự kiện `TESDeathEvent`. Nếu kẻ giết là player thì hồi `max × %` cho từng chỉ số
  (chỉ hồi phần bị mất, không vượt quá max).
- Mặc định hồi 10% mỗi chỉ số, chỉ tính khi giết kẻ đang thù địch (`bOnlyHostile=1`) để không farm gà.
- Chỉnh trong `Data/SKSE/Plugins/KillRestore.ini` (`fHealthPercent`, `fMagickaPercent`, `fStaminaPercent`,
  `bOnlyHostile`, `bVerboseLog`).
- Không lưu gì vào save → thêm/gỡ lúc nào cũng được.

**Giới hạn:** chỉ tính khi game ghi nhận player là killer. Follower hay summon giết thì không hồi.

## Cài
Copy `SKSE/Plugins/KillRestore.dll` và `SKSE/Plugins/KillRestore.ini` vào `Data/`.

## Cần test in-game
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/KillRestore.log` có `Registered TESDeathEvent sink`.
- [ ] Bật `bVerboseLog=1`. Giết bandit → log có `Player killed ...`, thanh máu/mana/stamina hồi lên.
- [ ] Mỗi lần giết chỉ hồi **một lần** (event có 2 pha dying/dead, mod chỉ xử lý pha dead).
- [ ] Giết bằng spell, cung, sneak attack, killmove đều hồi.
- [ ] Giết gà/NPC thường với `bOnlyHostile=1` → không hồi (log `ignored`).
      *Cần xem: lúc chết NPC có còn tính là hostile không, nếu không thì bandit cũng bị bỏ qua → báo lại để sửa.*
- [ ] Follower/summon giết → không hồi.

## Lịch sử
- 0.1.0: bản đầu.
