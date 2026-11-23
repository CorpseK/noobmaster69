# TNGPlayerOnly (NSFW)

SKSE plugin bổ sung cho **The New Gentleman (TNG)**: chỉ **player** có addon. Mọi NPC giữ skin gốc, giống nút
"không phân phát cho NPC" của SOS ngày trước. Không có ESP, không sửa file nào của TNG.

## TNG phân phát thế nào (đọc từ source MIT của TNG)
- Lúc load data, TNG nhân bản **skin của từng race**, gắn ARMA addon (slot 52) vào bản sao, đánh keyword
  `TNG_TngSkin` (`TheNewGentleman.esp` `0xFF6`) rồi gán `race->skin` = bản sao. Vì vậy mọi NPC dùng skin
  mặc định của race đều có addon. NPC có skin riêng thì bị vá theo cùng cách.
- Trong lúc chơi, TNG chạy lại cho actor (hook `Character::Load3D`, các event equip/load/đổi race). Nó có thể gắn
  lại addon cho NPC: lựa chọn đã lưu, tỉ lệ ngẫu nhiên "Gentlewoman" (mặc định 20%), hoặc khi bạn chỉnh MCM.
- MCM của TNG có thể chỉnh "None" cho từng nhóm race, nhưng phải chỉnh từng nhóm một, các nhóm ít NPC còn bị ẩn
  khỏi MCM, và player cũng dùng chung skin race → player cũng mất luôn.

## Mod này làm gì
1. Một frame sau `kDataLoaded` (lúc TNG đã xử lý xong):
   - Ghi nhớ skin TNG của từng race. Nếu player chưa có skin riêng thì gán cho player base skin TNG của race đó,
     để player **giữ** addon.
   - Trả **mọi race** và **mọi NPC khác player** về skin gốc. Skin gốc tìm bằng cách so: cùng race, cùng danh sách
     ARMA trừ ARMA slot 52. TNG chỉ copy skin gốc rồi thêm addon nên hai bên khớp nhau.
2. Bọc hook `Load3D` của TNG và nghe cùng các event. Mỗi khi TNG gắn lại addon cho NPC, mod gỡ ngay và refresh 3D
   của NPC đó.
- Không bao giờ sửa các skin TNG (player vẫn đang dùng), không đụng vào player. MCM, size, chọn addon cho player vẫn
  dùng bình thường.
- Không lưu gì vào save. Gỡ dll thì TNG trở lại như cũ ở lần load sau.

### Giới hạn / cần biết
- Mod dựa vào cách TNG làm ở bản source hiện tại (repo ModiLogist/TheNewGentleman, commit `db688f2`, 08/2026).
  TNG đổi cách phân phát hoặc đổi FormID keyword thì phải sửa lại. Log sẽ báo nếu không tìm thấy keyword.
- TNG vẫn **chạy** cho NPC (nó không biết mod này), chỉ là kết quả bị gỡ. Phần lag/crash do render addon trên NPC sẽ
  hết, nhưng phần TNG tự xử lý mỗi lần NPC load 3D vẫn còn.
- Ngay khi TNG gắn lại addon cho NPC (vd. lúc NPC thay đồ), có thể thấy addon nháy lên trong 1 frame rồi biến mất.
- Skin gốc nào không tìm được thì giữ nguyên skin TNG và ghi `No original skin found` vào log.
- Player nữ: được gán skin race giống hệt mặc định của TNG. Muốn TNG tự lo hoàn toàn cho player thì đặt
  `bKeepPlayer=0`.

## Cài
Copy `SKSE/Plugins/TNGPlayerOnly.dll` và `SKSE/Plugins/TNGPlayerOnly.ini` vào `Data/`. Vẫn giữ TNG như cũ.

## Cần test in-game
Bật `bVerboseLog=1` khi test. Log nằm ở `Documents/My Games/Skyrim Special Edition/SKSE/TNGPlayerOnly.log`.

- [ ] Log có `Indexed N candidate original skins`, `Initial pass: reverted X race skins and Y NPC skins`
      (X > 0), `Hooked Character::Load3D`, `TNGPlayerOnly ready`.
- [ ] Log **không** có (hoặc rất ít) dòng `No original skin found`. Có thì gửi log để xem skin nào.
- [ ] Player (nam) vẫn có addon. Chỉnh size/addon cho player trong MCM của TNG vẫn ăn.
- [ ] NPC nam (guard, bandit, NPC race mod như Bretons/Khajiit/Argonian) khi cởi đồ **không** có addon.
- [ ] NPC có skin riêng (follower mod, vd. có body riêng) vẫn giữ body/skin của họ, chỉ mất addon.
- [ ] NPC thay đồ / bị lột đồ → không có addon xuất hiện lại (có thể nháy 1 frame).
- [ ] NPC nữ không có addon "Gentlewoman" (kể cả khi TNG để tỉ lệ ngẫu nhiên 20%).
- [ ] Dùng hotkey NPC edit của TNG để gắn addon cho một NPC → bị gỡ lại (đúng thiết kế: chỉ player).
- [ ] Đổi race player bằng `showracemenu` → player vẫn có addon sau khi đóng menu.
- [ ] Save/load, fast travel, vào/ra cell: không crash, kết quả giữ nguyên.
- [ ] `bEnabled=0` → mọi thứ như TNG gốc.

## Lịch sử
- 0.1.0: bản đầu.
