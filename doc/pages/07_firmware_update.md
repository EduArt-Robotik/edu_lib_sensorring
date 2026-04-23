# Firmware Update of the Sensor Ring Boards

Every Sensor Board ships with a small bootloader that takes over right after reset and decides whether to launch the application or remain in bootloader mode for a firmware update.
This page walks through the bootloader's behavior, the `firmware_updater` command line tool and the recovery procedure to force bootloader mode.

## 1. Bootloader Behavior

After every reset the bootloader starts first and then decides where to continue:

| Condition | Result |
|:----------|:-------|
| Valid application stored in flash | Jump to the application |
| No application present | Stay in bootloader mode |
| Application checksum invalid | Stay in bootloader mode |
| Software request from the main firmware to enter bootloader mode | Stay in bootloader mode |
| Reset recovery pattern triggered | Stay in bootloader mode (see [Section 4](#autotoc_md_4-recovery-force-bootloader-with-3-reset-pattern)) |

> ℹ️ A board in bootloader mode keeps its downstream power supply (`VEXT`) disabled to isolate the rest of the ring during an update.

> ℹ️ All boards with RGB Leds light up in a dim violet color to indicate that they are in bootloader mode.

## 2. The `firmware_updater` Tool

All update operations are handled by the command line tool `firmware_updater`.
It is built alongside the library (with `SENSORRING_BUILD_FIRMWARE_UPDATE=ON`) and lives in `apps/utils/firmware_updater`.

### 2.1 Command Syntax

```sh
firmware_updater -m <mode> -t <socketcan|usbtingo> -i <interface-name> [-n <board-index>] [-f <firmware.hex>]
```

| Argument | Description |
|:---------|:------------|
| `-m` | Update mode, see [Section 2.2](#autotoc_md_22-update-modes) |
| `-t` | Communication interface type (`socketcan` or `usbtingo`) |
| `-i` | Interface name (e.g. `can0` for SocketCAN or `0` for USBtingo) |
| `-n` | Zero-based index of the target board on the ring |
| `-f` | Path to the firmware image in Intel HEX format |

> ℹ️ Flashing requires a SocketCAN interface. The `usbtingo` transport can be used to send a board into bootloader mode but the actual flashing step is currently SocketCAN only.

### 2.2 Update Modes

The `-m` argument selects one of four modes. Each mode operates on a single, explicitly addressed board, or on the entire interface at once:

| Mode | Purpose | Required Arguments |
|:-----|:--------|:-------------------|
| `enter` | Send one specific board into bootloader mode | `-n` |
| `flash` | Flash a board that is already in bootloader mode | `-f` |
| `enter-flash` | Send one specific board into bootloader mode and flash it in one step | `-n` and `-f` |
| `auto-all` | Walk the full ring on one interface and flash every board sequentially | `-f` |

### 2.3 Usage Examples

**Send board 0 on `can0` into bootloader mode:**

```sh
firmware_updater -m enter -t socketcan -i can0 -n 0
```

**Flash a board that is already waiting in bootloader mode:**

```sh
firmware_updater -m flash -t socketcan -i can0 -f ./firmware.hex
```

**Send board 1 into bootloader mode and flash it right away:**

```sh
firmware_updater -m enter-flash -t socketcan -i can0 -n 1 -f ./firmware.hex
```

**Update every board on the interface automatically:**

```sh
firmware_updater -m auto-all -t socketcan -i can0 -f ./firmware.hex
```

## 3. Recommended Workflow

The typical update flow on a healthy ring looks like this:

1. Bring up the communication interface as described in [Hardware - Supported Communication Interfaces](01_hardware.md#autotoc_md_3-supported-communication-interfaces).
2. Run `firmware_updater -m auto-all ...` to update the whole ring in one go.
3. Use the single-board modes (`enter`, `flash`, `enter-flash`) when targeting an individual board or when recovering from a failed run.
4. After the tool reports success, power-cycle the ring and verify that every board comes back up in application mode.

## 4. Recovery: Force Bootloader with 3-Reset Pattern

If a board becomes unresponsive and the normal modes no longer reach it, the bootloader can be forced into bootloader mode entirely from the board side, without any bus communication:

1. Trigger a hardware reset on the affected board.
2. Trigger a second hardware reset **within 1 second** of the first.
3. Trigger a third hardware reset **within 1 second** of the second.

After the third reset in this timing window, the bootloader skips the application and stays in bootloader mode, even if a valid application image is installed.
From there, a fresh image can be written with `firmware_updater -m flash ...`.

> ℹ️ The reset pattern is consumed once. The next boot requires a fresh sequence of three resets to force bootloader mode again.

## 5. Troubleshooting

A few common situations and what to try first:

| Symptom | Likely Cause | Suggested Action |
|:--------|:-------------|:-----------------|
| `No board detected in bootloader mode` | Interface not up, or no board currently in bootloader | Verify the interface with `ip addr`, then retry `-m enter` before `-m flash` |
| Flashing aborts immediately | Wrong firmware layout or target board not in bootloader | Rebuild the firmware against the bootloader layout and re-check the target board |
| Only part of the ring gets updated | Downstream board failed mid-flash and blocks the chain | Apply the 3-reset recovery pattern to the failing board and re-run the update |

<div class="section_buttons"> 

| Read Previous | |
|:--|--:|
| [Wrappers](06_wrappers.md) | |

</div>
