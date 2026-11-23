# Skyrim mods (SKSE plugin)

Nhắm tới **Skyrim AE 1.6.1170** + SKSE + Address Library. Mỗi mod chỉ là một file `.dll`
trong `Data/SKSE/Plugins/`, không có ESP/ESL/ESM → thêm/xóa giữa chừng save không sao.

| Mod | Mô tả | Trạng thái |
|---|---|---|
| [SpellBuffs](SpellBuffs/) | Buff các spell vanilla. Paralyze → móc túi 100% | v0.1.0, chưa test in-game |

## Tải file DLL đã build sẵn

GitHub tự build mỗi lần có thay đổi trong `skyrim-mods/`:

1. Vào tab **Actions** của repo → workflow **Build SKSE plugins** → chọn lần chạy mới nhất (dấu tích xanh).
2. Kéo xuống phần **Artifacts**, tải `SpellBuffs.zip`.
3. Giải nén vào thư mục `Data` của game (hoặc cài như một mod trong MO2/Vortex).

## Tự build trên máy (Windows)

Cần: Visual Studio 2022 trở lên (workload *Desktop development with C++*), CMake ≥ 3.24, Git, vcpkg.

```bash
cd skyrim-mods/SpellBuffs
```

```bash
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=<đường-dẫn-vcpkg>/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

```bash
cmake --build build --config Release
```

File ra ở `build/Release/SpellBuffs.dll`.
