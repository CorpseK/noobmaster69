# DOMTalkGate (NSFW)

SKSE plugin bổ sung cho **Diary of Mine (DOM)**: thoại của DOM với NPC **chưa bị capture** chỉ hiện trong một khoảng
ngắn (mặc định 30 giây) sau khi bạn cast spell **Training Info** (hoặc power) lên NPC đó. Hết thời gian thì thoại tự
biến mất. Không có ESP, không sửa file nào của DOM.

Viết theo **DOM 8.5.6f SE** (`DiaryOfMine.esm`).

## DOM gốc làm thế nào
- Thoại cho NPC chưa capture nằm ở 4 branch top-level của `DiaryOfMine.esm`. Các dòng mở đầu của chúng có điều kiện
  `Subject.GetInFaction(DOMPlayerSlaveFaction) == 0`, còn `GetRelationshipRank` dùng để chọn câu nào hợp với NPC:

  | Branch | Dòng mở đầu cho NPC chưa capture |
  |---|---|
  | `DOM01DialogueInfoBranch` | 26 (mỗi mức relationship rank một câu) |
  | `DOM01SlaveOrderBranch` | 3 |
  | `DOM01DialogueSlaveOrder` | 4 |
  | `DOM01DialogueSlavePraiseBranch` | 4 |

- Vì mọi mức rank đều có câu, nên NPC nào cũng hiện thoại DOM khi bạn nói chuyện.
- Spell `DOMInfoSpell` và power `DOMInfoPower` gắn hiệu ứng `DOMTrainingInfoEffect` (`0x5BD55A`) trong **1 giây**.
  Script của hiệu ứng in thông tin training của NPC ra màn hình.

## Mod này làm gì (lúc load game, chỉ trong bộ nhớ)
1. Tăng thời gian của `DOMTrainingInfoEffect` trên các spell DOM lên `iWindowSeconds` (30 giây). Script vẫn chạy
   một lần và in thông tin như cũ, chỉ là hiệu ứng ở lại trên NPC lâu hơn.
2. Thêm vào đầu mỗi dòng mở đầu ở trên một điều kiện: `Subject.HasMagicEffect(DOMTrainingInfoEffect) == 1` (AND).
   - Chỉ chọn các dòng có `GetInFaction(DOMPlayerSlaveFaction) == 0` trên Subject và **không** có
     `GetIsID`/`GetStage`, nên thoại quest của DOM (Kimli, Agnis...) không bị đụng tới.
   - Điều kiện relationship rank vẫn giữ, nên DOM vẫn chọn câu hợp với quan hệ của NPC.
- Thoại với nô lệ đã capture **không đổi**.
- Không lưu gì vào save. Gỡ DLL thì DOM trở lại như cũ ở lần load sau.

### Cần biết
- Cast spell rồi mới mở hội thoại. Đang trong hội thoại mà hết 30 giây thì danh sách thoại chỉ cập nhật khi mở lại.
- Với nô lệ đã capture, hiệu ứng info cũng ở lại 30 giây, nhưng không ảnh hưởng gì (thoại của nô lệ không bị chặn).
- DOM có sẵn global `DOM_DeactivateDialogueForNPC` (MCM) để **tắt hẳn** thoại cho NPC chưa capture. Mod này là cách
  ở giữa: tắt mặc định, bật khi cần.
- Mod tìm form theo FormID của DOM 8.5.6f. Bản DOM khác đổi FormID thì log sẽ báo `not found` và mod không làm gì.

## Cài
Copy `SKSE/Plugins/DOMTalkGate.dll` và `SKSE/Plugins/DOMTalkGate.ini` vào `Data/`.

## Cần test in-game
Log nằm ở `Documents/My Games/Skyrim Special Edition/SKSE/DOMTalkGate.log`.

- [ ] Log có `Info effect now lasts 30 s on 2 spell effect(s)` (`DOMInfoSpell` + `DOMInfoPower`).
- [ ] Log có 4 dòng `Branch ...: gated N opening line(s)` và `Gated 37 DOM opening line(s)`.
      Số khác 37 thì gửi log (có thể khác bản DOM).
- [ ] Nói chuyện với NPC thường (chưa capture) **không** cast spell → không có thoại DOM.
- [ ] Cast Training Info lên NPC → vẫn hiện thông tin như cũ. Nói chuyện trong 30 giây → có thoại DOM.
- [ ] Chờ hơn 30 giây rồi nói chuyện lại → thoại DOM biến mất.
- [ ] Nô lệ đã capture: thoại DOM vẫn hiện bình thường, không cần cast.
- [ ] Thoại quest của DOM (Kimli, Agnis...) vẫn hiện đúng lúc.
- [ ] `bGateNPCDialogue=0` → thoại như DOM gốc.

## Lịch sử
- 0.1.0: bản đầu.
