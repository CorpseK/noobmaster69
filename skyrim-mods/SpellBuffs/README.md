# SpellBuffs

SKSE plugin buff các spell có sẵn trong game. Không có ESP.

## Tính năng

### Paralyze → móc túi chắc chắn thành công
Khi NPC đang bị paralyze, mọi lần móc túi của người chơi lên NPC đó có tỉ lệ **100%**
(cả con số "Steal X%" trong menu lẫn lần roll thật).

Cách làm: hook hàm `AIFormulas::ComputePickpocketSuccess` của game
(Address Library ID SE 25822 / AE 26379). Kiểm tra paralyze bằng actor value `Paralysis > 0`,
nên áp dụng với mọi nguồn paralyze (spell Paralyze, poison, enchantment…).

**Giới hạn hiện tại:** đồ đang mặc/cầm vẫn theo luật vanilla (cần perk Misdirection /
Perfect Touch). Mở khóa cả đồ đang mặc là bước tiếp theo.

## Cần test in-game
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/SpellBuffs.log` có dòng `Hooked ComputePickpocketSuccess`.
- [ ] Có mở được menu móc túi trên NPC đang bị paralyze không (NPC ragdoll).
- [ ] Menu hiện 100% và lấy đồ không bị phát hiện.
- [ ] NPC không bị paralyze → tỉ lệ bình thường như vanilla.

## Lịch sử
- 0.1.0: tính năng Paralyze → móc túi 100%.
