# LightPockets

SKSE plugin làm nhẹ (hoặc bỏ hẳn) cân nặng của đồ lặt vặt. Không có ESP.

## Tính năng
Sau khi game load xong dữ liệu (`kDataLoaded`), nhân cân nặng của từng loại đồ với hệ số trong
`Data/SKSE/Plugins/LightPockets.ini`:

| Key | Loại đồ | Mặc định |
|---|---|---|
| `fIngredients` | Nguyên liệu alchemy | 0.0 |
| `fPotions` | Thuốc (không phải poison, không phải đồ ăn) | 0.0 |
| `fPoisons` | Poison | 0.0 |
| `fFood` | Đồ ăn/uống | 1.0 (giữ nguyên) |
| `fSoulGems` | Soul gem | 0.0 |
| `fLockpicks` | Lockpick (Skyrim.esm `0000000A`) | 0.0 |
| `fKeys` | Chìa khóa | 0.0 |
| `fScrolls` | Scroll | 0.0 |
| `fBooks` | Sách, note, thư | 1.0 (giữ nguyên) |

`0.0` = không cân nặng, `1.0` = vanilla. Áp dụng cho mọi item của mọi plugin (kể cả đồ mod thêm).

Cân nặng chỉ đổi trong RAM, **không ghi vào save** → gỡ DLL là mọi thứ về như cũ.

## Cài
Copy `SKSE/Plugins/LightPockets.dll` và `SKSE/Plugins/LightPockets.ini` vào `Data/`.
Không có file INI thì dùng giá trị mặc định ở bảng trên.

## Cần test in-game
- [ ] Log `Documents/My Games/Skyrim Special Edition/SKSE/LightPockets.log` có dòng `Changed weights: ...` với số lượng > 0.
- [ ] Mở inventory: ingredient, potion, poison, soul gem, lockpick, key, scroll hiện cân nặng 0.
- [ ] Đồ ăn và sách vẫn có cân nặng như vanilla.
- [ ] Tổng carry weight giảm đúng sau khi load save cũ đang mang nhiều ingredient.
- [ ] Đổi `fIngredients=0.5`, khởi động lại → ingredient còn một nửa cân nặng.
- [ ] Gỡ DLL, load lại save → cân nặng về vanilla, save không lỗi.

## Lịch sử
- 0.1.0: bản đầu.
