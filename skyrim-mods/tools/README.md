# Tools

Python thuần (3.9+), không cần cài thêm thư viện. Chạy trên Windows ở máy nhà.

| File | Việc |
|---|---|
| `fix_nif_paths.py` | Kiểm tra + sửa đường dẫn texture trong NIF của mod replacer, kiểm tra FaceGen với ESP |
| `niftex.py` | Đọc/ghi NIF Skyrim SE (20.2.0.7): texture set, shape, shader. Ghi lại giống hệt từng byte khi không sửa gì |
| `esp.py` | Đọc ESP/ESM: master, record (kể cả record nén) |
| `bsa.py` | Liệt kê / giải nén file trong BSA Skyrim SE (v105, cả file nén LZ4): `py -I bsa.py list <bsa> <regex>`, `py -I bsa.py extract <bsa> <regex> <thư mục ra>` |
| `nexus.py` | Tìm mod Skyrim SE trên Nexus qua API công khai (không cần đăng nhập, có cả mod adult): `py -I nexus.py search <từ khóa> --category Armour --adult any`, `py -I nexus.py info <id>`, `py -I nexus.py files <id>` |
| `strip_outfit_plugin.py` | Lọc plugin mod trang phục: giữ đồ + công thức temper, bỏ công thức chế, đồ đặt ngoài thế giới, quest phát đồ, mọi chỉnh sửa lên record game/mod khác (giữ FormID). `py -I strip_outfit_plugin.py <plugin> <file ra>`, `--dry-run` để xem trước |

## fix_nif_paths.py

```bash
python fix_nif_paths.py "D:\MO2\mods\Ten Mod" "D:\MO2\mods\Ten Mod - NIF fixes"
```

Có mod resource (SG Hairs, Eyes of Beauty…) thì thêm `--resource` cho từng thư mục, để file của chúng được tính là đã cài (chỉ loose file, file trong .bsa phải giải nén trước):

```bash
python fix_nif_paths.py "D:\MO2\mods\Ten Mod" "D:\MO2\mods\Ten Mod - NIF fixes" --resource "D:\MO2\mods\SG Hairs" --resource "D:\MO2\mods\The Eyes of Beauty"
```

- Thư mục mod gốc **chỉ được đọc**, không bị sửa.
- NIF đã sửa được ghi vào thư mục thứ hai, giữ nguyên đường dẫn → trong MO2 bật nó như một mod riêng, đặt **dưới** mod gốc.
  Muốn bỏ thì tắt mod đó đi.
- `report.txt` trong thư mục output liệt kê mọi thứ đã sửa và những gì phải sửa tay.

Tự sửa (chỉ khi file đích có thật trong mod):
1. Slot tint của FaceGen trỏ sang plugin khác / NPC khác / gõ sai → `facetint\<plugin>\<FormID>.dds` đúng của NPC đó.
2. Đường dẫn có `data\`, đường dẫn tuyệt đối, thiếu `textures\` → chuẩn hóa.
3. Texture không có trong mod nhưng có file cùng tên ở thư mục textures tương ứng với thư mục mesh
   (`meshes\A\B\x.nif` ↔ `textures\A\B\`), hoặc chỉ có đúng 1 file cùng tên trong mod.
   Đường dẫn vanilla thì không bao giờ đổi.

Các mục trong report:

| Mục | Ý nghĩa | Làm gì |
|---|---|---|
| `fixed` | Đã sửa | Không cần làm gì |
| `facegen-tint-needs-ck` | Không có file tint cho NPC | Xuất lại FaceGen trong CK (chọn NPC, Ctrl+F4) |
| `facegen-headpart-mismatch` | Head part trong ESP khác với FaceGen | Xuất lại FaceGen trong CK |
| `facegen-without-npc-record` | Có FaceGen nhưng ESP không có record NPC đó | Dark face, trừ khi plugin khác có record khớp. Xóa FaceGen hoặc thêm record |
| `missing-texture` | Texture không có ở đâu trong mod | Tìm file từ mod gốc, hoặc trỏ sang file khác |
| `esp-missing-mesh` / `esp-missing-texture` | ESP (head part, TXST, ARMA) trỏ tới file không có trong mod lẫn resource | Cài mod resource đó, hoặc gửi kèm `--resource` |
| `needs-<pack>` | Dùng texture của SG Hairs / KS Hairdo's… | Cài pack đó, nếu không sẽ tím |
| `vanilla-path-with-local-copy` | Trỏ vào texture vanilla trong khi mod có bản cùng tên cạnh mesh | Chỉ là gợi ý, có thể cố ý (hoặc ESP dùng TXST) |
