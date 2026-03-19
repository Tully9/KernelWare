# CS4422-KernelWare
---

<img width="384" height="65" alt="text-1772727326144" src="https://github.com/user-attachments/assets/29498554-e57c-4a1e-a320-3e2682fd2162" />

---

A WarioWare-style micro-game engine where mini-games are driven by a Linux kernel module. Dr. Mark Burkley course project.

---

## How to Use

### Quick Start

```bash
cd KernelWare/scripts/
sudo bash QuickStart.sh   # builds driver + userspace, loads the module
cd ../userspace/
./kernelware              # launch the game
```

### Manual Load / Unload

```bash
sudo bash scripts/load.sh     # insmod the driver
sudo bash scripts/unload.sh   # rmmod the driver
```

### Requirements

- Linux kernel headers (matching your running kernel)
- `ncurses` development library
- `gcc`, `make`

The driver creates `/dev/kernelware` (mode `0666`) — no `sudo` needed to run the userspace binary once the module is loaded.

### Controls

| Key | Action |
|-----|--------|
| Any key | Advance title/game-over screen |
| `A` `S` `D` `F` | Button-input games (Pipe Dream, Memory Leak, Type Faster) |
| `0-9`, letters, `-` | Text-input games (Kill It, Rot-Brain, Load Balancer, Hack the Host) |
| `Enter` | Submit typed answer |
| `Backspace` | Delete last character |

---

## Project Topology

```
KernelWare/
|
|-- shared/
|   `-- kw_ioctl.h          # ioctl command defs + kw_state/kw_config structs
|                             # (included by both kernel and userspace)
|
|-- driver/                 # Linux Kernel Module (.ko)
|   |-- kernelware_main.c   # char device init, file_operations (open/read/write/ioctl)
|   |-- kw_games.c          # game start/stop/input dispatch, per-game kernel logic
|   |-- kw_timer.c          # hrtimer: arms per-game countdown, fires timeout event
|   |-- kw_state.c          # spinlock-protected game state (round/next/timeout/reset)
|   |-- kernelware_proc.c   # /proc/kernelware_proc read handler
|   |-- kernelware.h        # shared kernel globals (kernel_buf, my_wq, current_state)
|   |-- kw_driver.h         # drv_state struct
|   |-- kw_games.h
|   |-- kw_state.h
|   `-- kw_timer.h
|
|-- userspace/              # ./kernelware binary
|   |-- main.c              # entry point; spawns 3 pthreads + render loop
|   |-- input.c             # hardware kbd event device + SSH stdin fallback
|   |-- input.h
|   `-- games/
|       |-- games.h         # game_def_t interface, game_shared_t, GAME_SET_MSG macro
|       |-- games.c         # games[] registry (one entry per mini-game)
|       |-- game_pipedream.c
|       |-- game_rotbrain.c
|       |-- game_killit.c
|       |-- game_memleak.c
|       |-- game_typefaster.c
|       |-- game_loadbalancer.c
|       `-- game_hackthehost.c
|
`-- scripts/
    |-- QuickStart.sh       # build driver, load module, build userspace
    |-- load.sh             # insmod + device setup
    `-- unload.sh           # rmmod
```

### High-Level Architecture

```
  Keyboard / SSH stdin
         |
         v
   [ input_thread ]          userspace (./kernelware)
         | write(driverFD, button event)
         v
  /dev/kernelware  ------>  kw_game_handle_input()  [kernel]
         |                          |
         | wake waitqueue           | update kernel_buf
         v                          |
   [ game_thread ]                  |
     game_N.run() <-- read() <------+
         |
         | ioctl(START / STOP / GET_STATE / SET_DIFF)
         v
     kw_ioctl()  [kernel]
         |
         +-- kw_game_start() --> kw_timer_start()
                                       |
                               [hrtimer fires]
                                kw_timer_callback()
                                writes KW_EVENT_TIMEOUT
                                wakes waitqueue
                                       |
                                       v
                           game_thread receives timeout --> lose

   [ render_thread ]  (every 50 ms)
     ioctl(KW_IOCTL_GET_STATE) --> compute time bar %
     games[id].draw()           --> ncurses overlay
```

The userspace binary runs **three threads**:

| Thread | File | Role |
|--------|------|------|
| `input_thread` | `input.c` | Reads raw key events; forwards button presses to the driver via `write()` |
| `render_thread` | `main.c` | Redraws the ncurses HUD + active game overlay at ~20 fps |
| `game_thread` | `main.c` | Orchestrates game sessions; calls each mini-game's blocking `run()` |

The **kernel module** exposes `/dev/kernelware` as a character device. It arms an `hrtimer` for each round, validates game-specific input, and wakes the userspace `read()` with either a button event or a timeout event.

### Difficulty Ramp

The session starts with a 10-second timer. Every 3 wins the timer shrinks by 10% (floor: 2 seconds), making each subsequent game faster.

---

## Mini-Games Overview

Seven mini-games are picked at random (never the same twice in a row). Each lasts one timer tick — win or lose, the next game starts immediately.

---

### 1 — Pipe Dream
**Controls:** `A` `S` `D` `F`

A kernel kthread fills a pipe buffer at a fixed rate. Press any of the four keys to drain it. Reduce the fill level from 100% to 0% before the buffer overflows.

---

### 2 — Rot-Brain
**Controls:** letter keys + `Enter`

The kernel applies a random ROT-N cipher (N = 1–12) to a common CS word (`kernel`, `module`, `driver`, `process`, or `thread`) and shows you the scrambled result. Type the original plaintext word and press `Enter` before time runs out.

---

### 3 — Kill It
**Controls:** digit keys + `Enter`

A real child process is forked and its PID is displayed on screen. The kernel validates your typed input. Type the exact PID and press `Enter` to `SIGKILL` it before the timer expires.

---

### 4 — Memory Leak
**Controls:** `A` (allocate) / `F` (free)

The kernel tracks a live `kmalloc` pointer. Alternate between pressing `A` to allocate and `F` to free it — 5 successful alloc/free cycles wins. Pressing `A` when memory is already allocated (leak) or `F` when nothing is allocated (double-free) costs a life.

---

### 5 — Type Faster
**Controls:** any key, as fast as possible

Mash any mapped key 20 times before the timer fires. A progress bar tracks how many presses have been registered in the kernel. Hit the target count to win.

---

### 6 — Load Balancer
**Controls:** `1` `2` `3` + `Enter`

The kernel spawns three `kw_lb_N` kthreads. Terminate all of them by typing their IDs (1, 2, or 3) one at a time and pressing `Enter`. Clear all three before the clock runs out.

---

### 7 — Hack the Host
**Controls:** alphanumeric / `-` keys + `Enter`

The kernel exposes the current UTS namespace hostname. Type any new hostname string and press `Enter` to overwrite it via `init_uts_ns`. The original hostname is restored automatically when the game ends (win or lose).

---

## Adding a Mini-Game

See [ADDING_A_MINIGAME.md](ADDING_A_MINIGAME.md) for a step-by-step guide covering the kernel-side setup, userspace `run`/`draw` implementation, and game registration.
