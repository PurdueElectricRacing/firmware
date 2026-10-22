# F4 CAN PHAL Redesign: G4-style "strong IRQ + weak callback" API

## 1. Goal

Make the F4 bxCAN PHAL expose the same public API shape and semantics as the G4 FDCAN PHAL, so F4 and G4 targets drive the CAN HAL identically from application and `can_library` code:

| | G4 (reference, unchanged) | F4 (new) |
|---|---|---|
| Init | `void PHAL_FDCAN_init(FDCAN_GlobalTypeDef*, PHAL_FDCAN_BaudRate_t)` | `bool PHAL_CAN_init(CAN_TypeDef*, PHAL_FDCAN_BaudRate_t)` |
| Filters | `bool PHAL_FDCAN_setFilters(FDCAN_GlobalTypeDef*, const uint32_t*, uint32_t, const uint32_t*, uint32_t)` | `bool PHAL_CAN_setFilters(CAN_TypeDef*, const uint32_t*, uint32_t, const uint32_t*, uint32_t)` |
| Send | `bool PHAL_FDCAN_send(CanMsgTypeDef_t*)` | `bool PHAL_CAN_send(CanMsgTypeDef_t*)` |
| TX free | `bool PHAL_FDCAN_txFifoFree(const FDCAN_GlobalTypeDef*)` | `bool PHAL_CAN_txFifoFree(const CAN_TypeDef*)` |
| RX path | Strong `FDCANx_IT0_IRQHandler` drains FIFO0 → weak `PHAL_FDCAN_rxCallback(CanMsgTypeDef_t*)` | Strong `CAN1/2_RX0_IRQHandler` drains FIFO0 → weak `PHAL_CAN_rxCallback(CanMsgTypeDef_t*)` |
| TX path | Strong `FDCANx_IT1_IRQHandler` → weak `PHAL_FDCAN_txCallback` | Strong `CAN1/2_TX_IRQHandler` → weak `PHAL_CAN_txCallback(CAN_TypeDef*)` |

Accepted divergences (call sites are per-app, not shared):
- F4 keeps a `bool` return on init (bxCAN init can time out in init mode; G4's is `void`).
- F4 keeps the `CAN_TypeDef` peripheral pointer; G4 keeps `FDCAN_GlobalTypeDef`. `CanMsgTypeDef_t.Bus` stays arch-typed; `can_tx.c`'s `CAN_BUS_TYPE` typedef bridges this.
- The `PHAL_FDCAN_BaudRate_t` / `FDCAN_BAUD_*` enum names are kept as-is on F4 (already the shared naming in `can.h` and every generated bus header).

## 2. F4 PHAL changes (`firmware/common/phal_F4/can/`)

### 2.1 `can.h` — new public API (hard cut, no compat shims)

Keep: the baud-rates enum, `CanMsgTypeDef_t`, existing bit-timing macros, the two weak-callback externs.

Remove: `PHAL_initCAN(bus, test_mode, uint32_t)`, `PHAL_deinitCAN`, `PHAL_txCANMessage(mbx)`, `PHAL_txMailboxFree(mbx)`, `PHAL_txCANAbort`, `PHAL_rxCANMessage(bus, fifo, msg)`, `PHAL_anyTxMailboxFree`, `PHAL_getFreeTxMailbox`.

Add:
- `bool PHAL_CAN_init(CAN_TypeDef *bus, PHAL_FDCAN_BaudRate_t bit_rate);`
- `bool PHAL_CAN_setFilters(CAN_TypeDef *bus, const uint32_t *sid_list, uint32_t num_sid, const uint32_t *xid_list, uint32_t num_xid);`
- `bool PHAL_CAN_send(CanMsgTypeDef_t *msg);` — mailbox selection/arming internal; non-blocking, false if no free mailbox.
- `bool PHAL_CAN_txFifoFree(const CAN_TypeDef *bus);` — true if any mailbox is free.
- `PHAL_CAN_MAX_NUM_SID_FILTER = 28`, `PHAL_CAN_MAX_NUM_XID_FILTER = 8` (signature parity with G4; runtime validates the bxCAN 28-combined limit, §2.3).

### 2.2 `can.c` — init

- Drop `test_mode` entirely (all call sites pass `false`); take the `PHAL_FDCAN_BaudRate_t` enum instead of `uint32_t bit_rate`.
- Per-bus self-sufficiency (so `init(CAN1)` + `init(CAN2)` each work standalone):
  - Every `PHAL_CAN_init` call first programs the CAN1→CAN2 bank split `CAN2SB = 14` (in CAN1's FMR), then programs the accept-all bank in the calling peripheral's own half and activates it. Idempotent.
  - CAN1: accept-all at bank 0. CAN2: accept-all at bank 14 (its first bank).
  - Accept-all bank: id 0, mask 0, 32-bit scale (FS1R set), filter mode = accept-all (FM1R cleared), routed to FIFO0. 32-bit scale both routes the bank to FIFO0 and legally binds it to one peripheral.
- RX collapses to FIFO0 only. Remove all RX1 handling: `RFLR`/`RF2R` code, RX1 SRR bit clears, `CAN1/2_RX1_IRQHandler` weak stubs.
- TX: `TMEIE` enabled in `IER` during init. bxCAN's TME is level-triggered, so the TX IRQ handler masks TMEIE on entry and re-enables it only if a frame is still in flight (TSR shows a mailbox in arbitration/transmit); `PHAL_CAN_send` re-enables TMEIE when it arms a frame. Mirrors G4's txCallback-driven re-arm semantics.

### 2.3 `can.c` — filters move from generated code into the PHAL

`PHAL_CAN_setFilters` implements the bxCAN bank packing that today lives in canpiler-generated `bxcan_set_filters()`:

- bxCAN has 28 filter banks: banks 0–13 owned by CAN1, banks 14–27 by CAN2 (CAN2SB=14).
- In 32-bit scale each bank holds TWO 32-bit filter words (FR1 and FR2), so 14 banks = 28 IDs per peripheral.
- Layout for a peripheral: base = (bus == CAN2) ? 14 : 0. std IDs fill banks first (FR1 = even index, FR2 = odd index), ext IDs continue in the next free word after the std IDs, sharing the last std bank when the std count is odd. Duplicate the final ID for an odd combined count.
- Std ID value: `id << 21`. Ext ID value: `(id << 3) | 4`.
- Used ID-list banks: FM1R set (ID-list mode), FS1R set (32-bit scale), FA1R set (active). Accept-all banks use mask mode (FM1R cleared).
- Validation: `num_sid <= 28`, `num_xid <= 8`, and `num_sid + num_xid <= 28` combined (the bxCAN per-peripheral limit); return false otherwise.
- Re-entry safety: enter init mode on CAN1 (INRQ, wait INAK); de-activate only the banks the target peripheral owns (FA1R bits for that half), write the new config, re-activate, exit init mode. INRQ/FINT enter/exit lives inside the PHAL — `can_init.c` no longer does it.

### 2.4 `can.c` — strong IRQ handlers + weak callbacks

- Strong `CAN1_RX0_IRQHandler` / `CAN2_RX0_IRQHandler`: drain FIFO0 (read RIR/RDTR/RDLR/RDHR while FMP0 set, release via RFOM0 after each frame) and for each frame fill `CanMsgTypeDef_t` (`Bus = CAN1/2`, `IDE`, `StdId`/`ExtId`, `DLC`, `Data[8]`) and call `PHAL_CAN_rxCallback(&msg)`. Mirrors G4's `FDCANx_IT0_IRQHandler` body.
- Strong `CAN1_TX_IRQHandler` / `CAN2_TX_IRQHandler`: read TSR; set RQCP0/1/2 for completed mailboxes; TME mask policy (§2.2); then call `PHAL_CAN_txCallback(CANx)` — mirroring G4's IT1 handler.
- Weak `PHAL_CAN_rxCallback(CanMsgTypeDef_t*)` — no-op default. Weak `PHAL_CAN_txCallback(CAN_TypeDef*)` — no-op default (already exists).
- Remove the old weak `CANx_RX0/RX1_IRQHandler` stubs and the old mailbox-based TX handler logic.

## 3. `can_library` changes (`firmware/can_library/`)

- `can_rx.c`: the 2-branch `#define PHAL_RX_CALLBACK` bridge (F4 → `PHAL_CAN_rxCallback`, G4 → `PHAL_FDCAN_rxCallback`) stays — signatures now match exactly. Delete the `// TODO: fix PHALs to expose the same function` comment.
- `can_tx.c`: replace the two-branch F4-mailbox-drain / G4-FIFO-drain code with one path per arch using `PHAL_*_txFifoFree` + `PHAL_*_send`, mirroring the G4 structure with the same `#define PHAL_TX_CALLBACK`-style bridge as `can_rx.c`. Keep the `CAN_BUS_TYPE` typedef; delete the mailbox-specific F4 branch and all `TMEIE` fiddling (re-arm now happens in the PHAL handler).
- `can_init.c`:
  - F4 filter path: stop calling the generated monolithic `bxcan_set_filters()` around INRQ/FINT; call the generated per-peripheral `CAN1_set_filters()` / `CAN2_set_filters()` instead (INRQ/FINT enter/exit now lives inside `PHAL_CAN_setFilters`).
  - F4 NVIC: enable `CAN1_RX0_IRQn`, `CAN2_RX0_IRQn`, `CAN1_TX_IRQn`, `CAN2_TX_IRQn` only — drop RX1.
  - G4 path untouched.

## 4. daq migration (`firmware/source/daq/`)

- `main.c:125,128`: `PHAL_initCAN(CAN1, false, VCAN_BAUD_RATE)` → `PHAL_CAN_init(CAN1, VCAN_BAUD_RATE)` (and CAN2).
- `can_irq.c`: delete the strong `CAN1_RX0_IRQHandler` / `CAN2_RX0_IRQHandler` that hand-poll the FIFO registers. Replace with a strong `PHAL_CAN_rxCallback(CanMsgTypeDef_t *msg)` that preserves current behavior:
  - `ticks_ms = xTaskGetTickCountFromISR()`, `last_can_rx_time_ms = ticks_ms`
  - `set_bus_id(&rx, (msg->Bus == CAN1) ? 0 : 1)`, `set_xid(&rx, msg->IDE)`, `set_can_id(&rx, msg->IDE ? msg->ExtId : msg->StdId)`
  - payload: zero-init, copy `msg->Data[0..DLC-1]` little-endian
  - `SPMC_enqueue_from_ISR(&g_spmc, &rx)`
  - `if (msg->Bus == CAN1 && !msg->IDE && msg->StdId == GPS_TIME_MSG_ID)` → `xQueueOverwriteFromISR(gps_time_queue, ...)`
  - `portYIELD_FROM_ISR(xHigherPriorityTaskWoken)`
- `can_irq.h`: drop the now-dead strong-handler declarations.
- `spmc.c`: no functional change — its RX0-only NVIC setup at equal priority is exactly what the PHAL's strong handlers need.

## 5. Other F4 call sites

No migration needed after rebasing onto `origin/master`: PDU now uses G4, and upstream deleted `firmware/source/f4_testing/canpiler.c`.

## 6. canpiler changes (`generators/canpiler/`)

- `codegen/hardware.py`: unify the filter model. Delete `BxcanFilters` (2-IDs-per-bank packing, CAN2 offset-14) and `FilterBank` bookkeeping. Both arches produce per-peripheral `{accept_all, std_ids: [msg...], ext_ids: [msg...]}`; bxCAN validation errors at codegen time when a peripheral exceeds 28 combined IDs.
- `codegen/templates/filter/bxcan.c.jinja` and `fdcan.c.jinja`: converge into one shared per-peripheral template emitting `CANn_set_filters()` / `FDCANn_set_filters()` that call `PHAL_CAN_setFilters` / `PHAL_FDCAN_setFilters` with the ID arrays (same structure as today's fdcan template, parameterized by PHAL function name).
- `node_header.h.jinja`: F4 node headers lose `bxcan_set_filters()`; gain `CAN1_set_filters()`/`CAN2_set_filters()`.
- Regenerate with `generators/generate.py`.

## 7. Known gaps / risks

1. F4 CAN only needs 500 kbit/s; no additional baud-rate timing macros are needed.
2. TMEIE re-arm policy: bxCAN TME is level-triggered; handler masks on entry, re-enables only if a frame is still in flight. Verify on bench with loopback.
3. Filter split idempotency: `PHAL_CAN_init` on either bus rewrites CAN1 FMR CAN2SB and its own half's bank state — keep FMR writes canonical so interleaved init/setFilters on the other bus cannot clobber active-bank bits.
4. bxCAN 28-combined-ID cap is tighter than G4's 28 std + 8 ext when mixing; validate at codegen time.

## 8. Order of implementation

1. F4 PHAL: new `can.h`/`can.c`.
2. daq migration (`main.c`, `can_irq.c`, `can_irq.h`).
3. Other F4 init call-site updates (obsolete after the upstream G4 migration).
4. `can_library`: unify `can_tx.c`, simplify `can_init.c` (NVIC RX1 drop, per-peripheral filter calls), drop `can_rx.c` TODO.
5. canpiler: unify `hardware.py`, converge filter templates, regenerate.
6. Full build of all targets; fix residuals.

## 9. Completion and validation

Implementation is complete. `python firmware/build_firmware.py` passed a clean configure,
generation, compilation, link, and packaging run for all firmware targets, including DAQ,
PDU, standalone images, resident bootloaders, and bootloadable applications. The packaging
workflow also passed vector-address and application-image validation. No residual build
fixes were required.

Hardware validation remains outstanding: RX FIFO draining, TX interrupt re-arming, and
CAN1/CAN2 filter isolation still need bench verification.
