# SpellBuffs

SKSE plugin buff các spell có sẵn trong game. Không có ESP.

## Tính năng

### Paralyze → móc túi chắc chắn thành công
Khi NPC đang bị paralyze, mọi lần móc túi của người chơi lên NPC đó có tỉ lệ **100%**
(cả con số "Steal X%" trong menu lẫn lần roll thật).

Cách làm: hook hàm `AIFormulas::ComputePickpocketSuccess` của game
(Address Library ID SE 25822 / AE 26379). Kiểm tra paralyze bằng actor value `Paralysis > 0`,
nên áp dụng với mọi nguồn paralyze (spell Paralyze, poison, enchantment…).

Game gốc **không cho kích hoạt** NPC đang nằm ragdoll vì paralyze, nên menu móc túi không mở được.
Từ 0.2.0 mod hook thêm `TESNPC::Activate` và `GetActivateText` (vtable 0x37 / 0x4C): khi player **đang sneak**
và nhìn vào NPC còn sống đang bị paralyze, sẽ hiện chữ "Pickpocket" và bấm kích hoạt sẽ mở menu móc túi.
Áp dụng cả với NPC thù địch (vd. bandit bị paralyze).

**Giới hạn hiện tại:** đồ đang mặc/cầm vẫn theo luật vanilla (cần perk Misdirection /
Perfect Touch). Mở khóa cả đồ đang mặc là bước tiếp theo.

## Cần test in-game
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/SpellBuffs.log` có dòng `Hooked ComputePickpocketSuccess`.
- [x] ~~Có mở được menu móc túi trên NPC đang bị paralyze không~~ → 0.1.0: **không** (game chặn). Đã sửa ở 0.2.0.
- [ ] (0.2.0) Log có `Hooked TESNPC::Activate / GetActivateText`.
- [ ] (0.2.0) Sneak + nhìn vào NPC đang paralyze → hiện "Pickpocket"; bấm E → mở menu, log có
      `Pickpocket on paralyzed ...`.
- [ ] (0.2.0) Không sneak → như vanilla (không làm gì). NPC chết → loot bình thường.
- [ ] Menu hiện 100% và lấy đồ không bị phát hiện.
- [ ] NPC không bị paralyze → tỉ lệ bình thường như vanilla.

## Lịch sử
- 0.2.0: mở được menu móc túi trên NPC đang bị paralyze (khi sneak).
- 0.1.0: tính năng Paralyze → móc túi 100%.
