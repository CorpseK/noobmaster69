# GMSTTweaks

SKSE plugin đổi Game Setting (GMST) bằng file INI, không cần ESP. Tiện để chỉnh
những thứ thường phải làm ESP chỉ để sửa một con số (nhảy cao, số lần train/level, tỉ lệ nhặt mũi tên...).

## Tính năng
- Sau khi tất cả plugin load xong (`kDataLoaded`) thì đọc section `[GameSettings]` trong
  `Data/SKSE/Plugins/GMSTTweaks.ini` và ghi đè từng GMST. Vì chạy sau cùng nên **thắng cả ESP** khác sửa cùng GMST.
- Kiểu giá trị lấy theo setting gốc: float (`f...`), int (`i...`), unsigned (`u...`), bool (`b...`, `0/1/true/false`).
- Log ghi `tên: giá trị cũ -> giá trị mới`, nên muốn biết giá trị vanilla cứ thêm dòng rồi xem log.
- Tên GMST sai → log cảnh báo `no such game setting`, không crash.
- GMST không lưu trong save → gỡ DLL/xóa dòng là về như cũ.

File INI kèm theo có vài ví dụ (đều đang tắt bằng `;`).

## Cài
Copy `SKSE/Plugins/GMSTTweaks.dll` và `SKSE/Plugins/GMSTTweaks.ini` vào `Data/`.

## Cần test in-game
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/GMSTTweaks.log` có `Read N entries` và `Applied N/N`.
- [ ] Bật `fJumpHeightMin=110` → nhảy cao hơn rõ rệt. Log hiện giá trị cũ (~76).
- [ ] Bật `iTrainingNumAllowedPerLevel=10` → train được 10 lần/level.
- [ ] Thêm một dòng tên bậy (`fKhongCo=1`) → log có `no such game setting`, game vẫn chạy.
- [ ] Console `GetGameSetting fJumpHeightMin` trả về giá trị mới.

## Lịch sử
- 0.1.0: bản đầu (float/int/uint/bool).
