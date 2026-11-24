# Skyrim mods (SKSE plugin)

Nhắm tới **Skyrim AE 1.6.1170** + SKSE + Address Library. Mỗi mod chỉ là một file `.dll`
trong `Data/SKSE/Plugins/`, không có ESP/ESL/ESM → thêm/xóa giữa chừng save không sao.

| Mod | Mô tả | Trạng thái |
|---|---|---|
| [tools](tools/) | Script Python kiểm tra/sửa NIF, FaceGen của mod replacer | dùng được |
| [SpellBuffs](SpellBuffs/) | Buff các spell vanilla. Paralyze → móc túi 100% | v0.2.0, đã test in-game |
| [LightPockets](LightPockets/) | Ingredient, potion, poison, soul gem, lockpick, key, scroll không cân nặng (chỉnh bằng INI) | v0.1.0, chưa test in-game |
| [GMSTTweaks](GMSTTweaks/) | Đổi Game Setting (GMST) bằng file INI, thay cho ESP chỉ để sửa số | v0.1.0, chưa test in-game |
| [KillRestore](KillRestore/) | Player giết địch → hồi % Health/Magicka/Stamina | v0.1.0, chưa test in-game |
| [ImmersiveRecruit](ImmersiveRecruit/) | NPC relationship ≥ Friend (vd. Aeri) rủ làm follower bằng thoại vanilla, không cần MCM + follower theo level player, Protected | v1.0.0, đã test in-game (có RDO) |
| [GenderShift](GenderShift/) | NPC spawn từ leveled list (bandit, Forsworn...) đổi tỉ lệ nam/nữ; có chế độ cả trại toàn nam hoặc toàn nữ | v0.1.0, đã test in-game |
| [RelicHunter](RelicHunter/) | Loot random (rương, xác, đồ NPC, merchant) dễ ra relic LotD mà The Curator's Companion báo là còn thiếu | v0.1.0, chưa test in-game |
| [TNGPlayerOnly](TNGPlayerOnly/) | (NSFW) Add-on cho The New Gentleman: chỉ player có addon, NPC giữ skin gốc | v0.2.0, đã test in-game |

## Tải file DLL đã build sẵn

GitHub tự build mỗi lần có thay đổi trong `skyrim-mods/`:

1. Vào tab **Actions** của repo → workflow **Build SKSE plugins** → chọn lần chạy mới nhất (dấu tích xanh).
2. Kéo xuống phần **Artifacts**, tải file zip của mod cần (vd. `SpellBuffs.zip`). Mod nào có INI thì INI nằm sẵn trong zip.
3. Giải nén vào thư mục `Data` của game (hoặc cài như một mod trong MO2/Vortex).

## Tự build trên máy (Windows)

Cần: Visual Studio 2022 trở lên (workload *Desktop development with C++*), CMake ≥ 3.24, Git, vcpkg.

```bash
cd skyrim-mods/<TênMod>
```

```bash
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=<đường-dẫn-vcpkg>/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

```bash
cmake --build build --config Release
```

File ra ở `build/Release/<TênMod>.dll`.
