# BendWillRecruit

SKSE plugin, không có ESP. Shout **Bend Will** (Dragonborn DLC) biến mọi NPC humanoid trúng shout thành follower.
Vanilla shout này gần như chỉ để cưỡi rồng trong cốt truyện Dragonborn.

## Tính năng
- Lúc load game, mod tìm shout Bend Will (theo tên `sShoutName`, hoặc form `sShoutForm` trong INI) và lấy
  magic effect của cả 3 word. Log ghi rõ shout và effect tìm được.
- Khi effect của player trúng một NPC có keyword `ActorTypeNPC` (người, elf, orc, khajiit, argonian; không tính
  rồng, sinh vật), mod:
  1. Dừng combat nếu NPC đang đánh nhau.
  2. Nâng relationship với player lên **Ally** (Papyrus `SetRelationshipRank`).
  3. Thêm vào `PotentialFollowerFaction` để dismiss xong vẫn rủ lại được bằng thoại.
  4. Gọi `DialogueFollowerScript.SetFollower`, đúng hàm câu thoại "Follow me" vanilla gọi.
     **NFF bắt lời gọi này** nên follower vào NFF như rủ bằng thoại thường.
- Bỏ qua: NPC đã chết, trẻ em, NPC đã là follower, NPC trong `[Exclude]`.
- `bRecruitHostile=1` (mặc định): rủ được cả bandit, Forsworn đang đánh nhau với bạn.
- Hiệu ứng vanilla của Bend Will vẫn chạy như cũ (cưỡi rồng không đổi).

**Cần NFF** (hoặc framework nhiều follower khác). Vanilla chỉ cho 1 follower: rủ người thứ hai bằng shout có thể
làm follower cũ bị kẹt.

**Lưu ý save:** relationship, faction và trạng thái follower được game lưu vào save. Gỡ mod thì follower đã rủ vẫn
là follower, chỉ là shout không rủ thêm được nữa.

## Cài
Copy `SKSE/Plugins/BendWillRecruit.dll` và `SKSE/Plugins/BendWillRecruit.ini` vào `Data/`. Cần Dragonborn DLC.

## Cần test in-game
Bật `bVerboseLog=1` khi test. Log ở `Documents/My Games/Skyrim Special Edition/SKSE/BendWillRecruit.log`.

### Khởi động
- [ ] Log có `Using shout ... 'Bend Will'`, 3 dòng `word 1/2/3: spell ... effect ...`, và
      `Registered TESMagicEffectApplyEvent sink`.
- [ ] Nếu log báo `Shout 'Bend Will' not found`: console `help "bend will" 4`, lấy ID dòng `SHOU`, điền vào
      `sShoutForm` (vd. `Dragonborn.esm|0x0xxxxx`, bỏ 2 số đầu load order) rồi thử lại.

### Rủ
- [ ] Shout vào NPC thường (thợ rèn, nông dân) → thông báo `... bows to your Voice and follows you.` → NPC đi theo,
      hiện trong MCM NFF.
- [ ] Shout vào bandit đang đánh nhau → bandit dừng đánh, đi theo bạn. Xem đồng bọn của nó có đánh nó không,
      và nó có quay lại đánh bạn sau vài giây không (ghi lại nếu có).
- [ ] Shout trúng nhiều NPC một lúc → tất cả đều thành follower (NFF có giới hạn số follower, xem có báo đầy không).
- [ ] Shout vào follower đang đi theo → log `already a follower`, không đổi gì.
- [ ] Shout vào sinh vật (sói, gấu), rồng → log `not humanoid`, rồng vẫn cưỡi được như vanilla.
- [ ] Shout vào trẻ em → không rủ.
- [ ] `iMinWords=3` → shout 1-2 word không rủ, đủ 3 word mới rủ.

### Sau khi rủ
- [ ] Dismiss qua NFF → nói chuyện lại → có câu "Follow me".
- [ ] Save → thoát → load: follower vẫn đi theo.
- [ ] Rủ NPC đang giữ quest (vd. người giao quest, merchant) → xem quest/cửa hàng có hỏng không. Hỏng thì thêm NPC
      đó vào `[Exclude]` và ghi lại tên.
- [ ] Rủ guard, Jarl → có lỗi gì lạ không (guard bỏ trạm, Jarl rời ngai).

Gặp lỗi thì gửi lại file log + tên NPC.

## Lịch sử
- 0.1.0: bản đầu.
