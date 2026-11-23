# EasyRecruit

SKSE plugin: NPC nào có relationship với player từ **Friend** trở lên đều rủ làm follower được bằng câu
thoại vanilla, không cần mở MCM của NFF. Không có ESP.

## Tính năng
- Game chỉ hiện câu thoại rủ đi theo khi NPC nằm trong `PotentialFollowerFaction` (Skyrim.esm `0005C84D`)
  **và** relationship ≥ Friend. Nhiều NPC (vd. Aeri ở Anga's Mill) lên được Friend nhờ quest/bán đồ
  nhưng không có faction đó nên không rủ được.
- Mỗi lần mở và đóng menu thoại, mod kiểm tra NPC đang nói chuyện. Nếu relationship ≥ `iMinRank` (mặc định 1 = Friend)
  thì thêm NPC vào `PotentialFollowerFaction`. Có thông báo `<Tên> can now be recruited as a follower.`
- Làm quest/favor xong → đóng thoại → **nói chuyện lại** là thấy câu rủ đi theo. NFF bắt câu thoại vanilla này như bình thường.
- NPC đã Friend từ trước khi cài mod: nói chuyện một lần, đóng, nói lại.
- Bỏ qua: NPC đã chết, trẻ em, NPC đang thù địch, NPC trong `[Exclude]` (đều chỉnh trong INI).

**Lưu ý save:** faction được game lưu vào save. Gỡ mod thì NPC đã được thêm vẫn rủ được, NPC mới thì không.
Không gây hỏng save.

## Cách lấy FormID cho `[Exclude]`
Mở console, click vào NPC, gõ `GetBaseObject` hoặc xem bằng mod More Informative Console. Bỏ 2 chữ số đầu
(load order), vd. `0A012345` của `Skyrim.esm` → `Skyrim.esm|0x012345`. Với ESL (`FE xxx yyy`) lấy 3 chữ số cuối.

## Cài
Copy `SKSE/Plugins/EasyRecruit.dll` và `SKSE/Plugins/EasyRecruit.ini` vào `Data/`.

## Cần test in-game
Bật `bVerboseLog=1` khi test. Mỗi lần nói chuyện, log ghi tên NPC, base FormID và relationship rank.

### Chung (có hay không có RDO đều test)
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/EasyRecruit.log` có `Registered MenuOpenCloseEvent sink`.
- [ ] Nói chuyện với NPC bất kỳ → rank trong log phải giống `GetRelationshipRank player` trong console (click NPC trước).
- [ ] NPC mới gặp (Acquaintance, rank 0) → không được thêm.
- [ ] Follower vanilla (Lydia, Faendal...) → log `already a potential follower`, không đổi gì.
- [ ] Trẻ em có rank Friend → không được thêm. NPC trong `[Exclude]` → không được thêm.
- [ ] Follower được thêm qua mod → rủ bằng thoại → NFF nhận (hiện trong MCM NFF), dismiss rồi rủ lại được.
- [ ] Save → thoát game → load: NPC đã thêm vẫn rủ được.

### Không có RDO (vanilla thoại)
- [ ] **Aeri:** bán firewood hoặc làm quest mang thư → đóng thoại → thông báo `Aeri can now be recruited...`
      → nói lại → có câu "Follow me" → cô ấy đi theo.
- [ ] Vẫn cưới được Aeri sau khi đã làm follower (đeo Amulet of Mara → câu hỏi cưới vẫn hiện).
- [ ] Thêm 1-2 NPC lên Friend qua favor (vd. chủ quán, NPC nhờ việc) → rủ được.

### Có RDO
- [ ] **Aeri** như trên: câu rủ đi theo vẫn hiện (RDO sửa nhiều thoại, có thể chen hoặc che câu vanilla).
- [ ] **NPC RDO chỉ cho rủ sau questline** (Valerica, Knight-Paladin Gelebor...): trong lúc questline **còn đang làm**,
      nếu rank lên Friend thì mod có thể cho rủ **sớm hơn** RDO định → có thể hỏng quest
      (NPC bỏ vị trí quest để đi theo). Ghi lại NPC nào bị như vậy và rank lúc đó.
      Cách xử lý: thêm NPC đó vào `[Exclude]`, đợi RDO tự mở khóa sau quest như cũ.
- [ ] Làm xong questline → NPC rủ được như RDO vốn cho, không bị lặp 2 câu "Follow me" (một của RDO, một vanilla).
- [ ] NPC RDO vốn đã rủ được → log `already a potential follower`.

Gặp lỗi thì gửi lại file log + tên NPC + đang có RDO hay không.

## Lịch sử
- 0.1.0: bản đầu.
