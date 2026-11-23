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
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/EasyRecruit.log` có `Registered MenuOpenCloseEvent sink`.
- [ ] Bật `bVerboseLog=1`. Nói chuyện với NPC bất kỳ → log ghi relationship rank của NPC đó (kiểm tra số có đúng không:
      console `GetRelationshipRank player` trên NPC phải ra cùng số).
- [ ] **Aeri:** bán firewood / làm quest mang thư → đóng thoại → log `Aeri ... added to PotentialFollowerFaction`,
      có thông báo góc màn hình → nói lại → có câu rủ đi theo, NFF nhận làm follower.
- [ ] NPC mới gặp (Acquaintance) → không được thêm.
- [ ] Follower vanilla (Lydia, Faendal...) → log `already a potential follower`, không đổi gì.
- [ ] Thử với RDO bật: câu thoại rủ vẫn hiện.
- [ ] Trẻ em có relationship Friend → không được thêm.
- [ ] NPC trong `[Exclude]` → không được thêm.

## Lịch sử
- 0.1.0: bản đầu.
