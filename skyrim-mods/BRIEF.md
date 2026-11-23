# Brief cho chat mới: Skyrim SKSE mods

## Về mình
- Modding Skyrim mức cơ bản: tự sửa mod, sửa navmesh trong Creation Kit, hiểu khác biệt giữa ESP/ESL/ESM.
- Game: **Skyrim AE 1.6.1170** (cố ý giữ bản này vì ổn định), có SKSE, CPKE (Creation Kit Platform Extended).
- Mục tiêu: mod dạng **SKSE plugin thuần, không có ESP**, thêm/xóa giữa save lúc nào cũng được.
- Làm ở công ty (cloud, không có game), về nhà mới test. Cứ tạo nhiều mod rồi test dần.

## Repo
- GitHub: `CorpseK/noobmaster69`, làm trên nhánh `main`. Mod nằm trong `skyrim-mods/<TênMod>/`.
- Mỗi mod có `README.md` riêng (tính năng + checklist test in-game) và được liệt kê trong bảng ở `skyrim-mods/README.md`.
- Ý tưởng cũ (trade đồ default của follower) nằm trên máy ở nhà → **bỏ**, không làm lại.

## Cách build (đã thiết lập)
- C++23, **CommonLibSSE-NG** lấy bằng CMake FetchContent, ghim commit `b93280e832f263dbef44e44cbe2936622a02f91a`. Tắt VR và tests.
- vcpkg manifest (`vcpkg.json`), baseline `5d4a3dc90c0b3773f3bac81bf41297b54328f471`, triplet `x64-windows-static`. Thư viện: spdlog, rapidcsv, detours.
- Hook hàm bằng **Microsoft Detours**, địa chỉ lấy qua `RELOCATION_ID(SE, AE)` (Address Library).
- Plugin khai báo `SKSEPluginInfo(... .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)`.
- Máy cloud là Linux, không build được → **GitHub Actions** (`.github/workflows/build-skse.yml`, chạy windows-latest) build mỗi lần push vào `skyrim-mods/**`. DLL tải ở Actions → Artifacts. Mod mới thì thêm job vào workflow này.
- Ghi chú về CommonLibSSE-NG: không có `IsPlayerRef()`, dùng `== RE::PlayerCharacter::GetSingleton()`. Chuỗi `"..."sv` cần `using namespace std::literals;`.

## Mod đã có
### SpellBuffs v0.1.0 (`skyrim-mods/SpellBuffs/`): buff spell vanilla
- Paralyze: khi NPC có actor value `Paralysis > 0`, móc túi của player thành công **100%**.
- Cách làm: hook `AIFormulas::ComputePickpocketSuccess` (ID SE 25822 / AE 26379), trả về 100.
- **Chưa test in-game.** Lần build CI đầu tiên lúc tạo brief chưa xác nhận xanh, mở tab Actions xem.
- Việc còn lại: mở khóa đồ đang mặc/cầm (vanilla cần perk Perfect Touch / Misdirection, perk entry `kCanPickpocketEquippedItem`). Kiểm tra game có cho mở menu móc túi khi NPC đang ragdoll không.

## Yêu cầu cho chat mới
Làm thêm vài mod SKSE (không ESP) theo đúng cấu trúc và cách build ở trên, push lên `main`, xem CI build có xanh không. Mỗi mod có README kèm checklist test để về nhà test.
