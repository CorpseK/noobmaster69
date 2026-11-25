# Brief cho chat mới: Skyrim SKSE mods

## Về mình
- Modding Skyrim mức cơ bản: tự sửa mod, sửa navmesh trong Creation Kit, hiểu khác biệt giữa ESP/ESL/ESM.
- Game: **Skyrim AE 1.6.1170** (cố ý giữ bản này vì ổn định), có SKSE, CPKE (Creation Kit Platform Extended).
  Quản lý mod bằng **MO2** (`E:\Skyrim Modding\MO2` ở máy nhà).
- Mục tiêu: mod dạng **SKSE plugin thuần, không có ESP**, thêm/xóa giữa save lúc nào cũng được.
- Làm ở công ty (chat cloud, không có game), về nhà test (có thể mở chat Claude Code chạy ngay trên máy nhà).

## Repo
- GitHub: `CorpseK/noobmaster69`. Mod nằm trong `skyrim-mods/<TênMod>/`.
- **Push lên `main`** được, miễn không ảnh hưởng mod khác (mỗi mod một thư mục riêng). Trước khi push thì
  `git fetch` + merge `origin/main`, vì máy nhà và các chat khác cũng push lên `main`.
- Mỗi mod có `README.md` riêng và một dòng trong bảng ở `skyrim-mods/README.md` (cột Trạng thái: phiên bản +
  đã/chưa test in-game).
- README của mod **chưa test** có mục "Cần test in-game" (checklist). Test xong thì thay bằng mục ngắn "Gỡ lỗi"
  (cách bật log + các dòng log cần xem) và sửa bảng thành "đã test in-game".
- Mod nào có INI thì INI tên `<TênMod>.ini`, nằm cạnh `CMakeLists.txt`. Mod nên có `bVerboseLog` để debug.
  Log ở `Documents/My Games/Skyrim Special Edition/SKSE/<TênMod>.log`.

## Cách build (đã thiết lập)
- C++23, **CommonLibSSE-NG** lấy bằng CMake FetchContent, ghim commit `b93280e832f263dbef44e44cbe2936622a02f91a`.
  Tắt VR và tests.
- vcpkg manifest (`vcpkg.json`), baseline `5d4a3dc90c0b3773f3bac81bf41297b54328f471`, triplet `x64-windows-static`.
  Thư viện: spdlog, rapidcsv, detours (detours chỉ thêm cho mod nào hook hàm).
- Hook hàm bằng **Microsoft Detours**, địa chỉ lấy qua `RELOCATION_ID(SE, AE)` (Address Library). Hook vtable thì
  dùng `REL::Relocation<std::uintptr_t>{ X::VTABLE[0] }.write_vfunc(idx, thunk)`.
- Plugin khai báo `SKSEPluginInfo(... .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)`.
- Máy cloud là Linux, không build được → **GitHub Actions** (`.github/workflows/build-skse.yml`, windows-latest)
  build mỗi lần push vào `skyrim-mods/**`. Mod mới thì thêm tên vào `matrix.mod`. DLL (+ INI) tải ở
  Actions → Artifacts. Một lần build mất khoảng 10–15 phút.
- Actions đã lên bản Node 24: `checkout@v5`, `cache@v5`, `upload-artifact@v6`.
- Ghi chú CommonLibSSE-NG:
  - Include `Windows.h`/detours **sau** `RE/Skyrim.h` + `SKSE/SKSE.h` (nếu không: lỗi "Windows API detected").
  - Runner GitHub không còn VS 2022 nên không ghi cứng `-G`.
  - Không có `IsPlayerRef()` trên mọi kiểu, dùng `== RE::PlayerCharacter::GetSingleton()`.
  - Chuỗi `"..."sv` cần `using namespace std::literals;`.
  - Source CommonLib ở commit ghim có thể clone vào scratchpad để tra API trước khi viết code.

## Mod đã có

| Mod | Bản | Trạng thái | Tóm tắt |
|---|---|---|---|
| SpellBuffs | 0.2.0 | đã test | Paralyze → móc túi 100% (hook `AIFormulas::ComputePickpocketSuccess` 25822/26379) + mở được menu móc túi NPC đang ragdoll khi sneak (hook `TESNPC::Activate`/`GetActivateText`, vtable 0x37/0x4C) |
| ImmersiveRecruit (trước là EasyRecruit) | 1.0.0 | đã test (có RDO) | NPC relationship ≥ Friend được thêm vào `PotentialFollowerFaction`; follower lên level theo player (cap 80), Protected. README/INI tiếng Anh |
| GenderShift | 0.1.0 | đã test | Hook `TESLeveledList::CalculateCurrentFormList` (14579/14751), đổi NPC bốc từ LVLN sang bản khác giới cùng list (cùng race/class, level ≤). Chế độ theo nhóm: cả trại cùng giới |
| TNGPlayerOnly (NSFW) | 0.2.0 | đã test | Add-on cho The New Gentleman: trả race/NPC về skin gốc, chỉ player giữ addon; NPC nam dùng thân vanilla có quần lót (mesh vanilla phải tự giải nén bằng `tools/bsa.py`) |
| RelicHunter | 0.1.0 | chưa test | LotD + The Curator's Companion: cùng hook leveled list, đổi loot thành relic còn thiếu (list `dbmNew`/`dbmFound`/`dbmDisp` trong `DBM_RelicNotifications.esp`) |
| LightPockets | 0.1.0 | chưa test | Ingredient, potion, poison, soul gem, lockpick, key, scroll không cân nặng (INI) |
| GMSTTweaks | 0.1.0 | chưa test | Đổi GMST bằng INI, thay ESP chỉ để sửa số |
| KillRestore | 0.1.0 | chưa test | Player giết địch → hồi % Health/Magicka/Stamina |
| DOMTalkGate (NSFW) | 0.1.0 | chưa test | Add-on cho Diary of Mine 8.5.6f: thoại DOM với NPC chưa capture chỉ hiện `iWindowSeconds` (30s) sau khi cast Training Info (kéo dài `DOMTrainingInfoEffect` 0x5BD55A + thêm điều kiện `HasMagicEffect` vào 37 dòng mở đầu) |

Ngoài ra `skyrim-mods/tools/` có script Python (NIF, FaceGen, ESP, BSA) chạy trên máy nhà.

Lưu ý: GenderShift và RelicHunter cùng hook `CalculateCurrentFormList`. Detours tự nối hai hook nên dùng chung được.

## Việc còn lại / ý tưởng
- Diary of Mine: delay lúc capture có clone (`DOM_SlaveManager.Capture` → `DOM_Core.CloneAndCopyActor`). Cần đo bằng Papyrus log trước khi sửa.
- Test RelicHunter, LightPockets, GMSTTweaks, KillRestore.
- SpellBuffs: mở khóa móc đồ đang mặc/cầm khi NPC bị paralyze (vanilla cần perk Misdirection / Perfect Touch,
  perk entry `kCanPickpocketEquippedItem`).
- Ý tưởng cũ (trade đồ default của follower) nằm trên máy ở nhà → **đã xong và chạy tốt**, không làm lại.

## Yêu cầu cho chat mới
Làm thêm hoặc sửa mod SKSE (không ESP) theo đúng cấu trúc và cách build ở trên, push lên `main`, xem CI build có
xanh không. Mod mới có README kèm checklist test để về nhà test.
