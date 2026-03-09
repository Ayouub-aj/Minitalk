<div align="center">

```
███╗   ███╗██╗███╗   ██╗██╗████████╗ █████╗ ██╗     ██╗  ██╗
████╗ ████║██║████╗  ██║██║╚══██╔══╝██╔══██╗██║     ██║ ██╔╝
██╔████╔██║██║██╔██╗ ██║██║   ██║   ███████║██║     █████╔╝ 
██║╚██╔╝██║██║██║╚██╗██║██║   ██║   ██╔══██║██║     ██╔═██╗ 
██║ ╚═╝ ██║██║██║ ╚████║██║   ██║   ██║  ██║███████╗██║  ██╗
╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝   ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝
```

<img src="https://img.shields.io/badge/42-School-000000?style=for-the-badge&logo=42&logoColor=white"/>
<img src="https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white"/>
<img src="https://img.shields.io/badge/Signals-SIGUSR1%20%7C%20SIGUSR2-FF6B35?style=for-the-badge&logo=linux&logoColor=white"/>
<img src="https://img.shields.io/badge/IPC-Inter--Process%20Comm-6C3483?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Status-Finished-27AE60?style=for-the-badge"/>

<br/><br/>

> *"Two processes. Two signals. One bit at a time — crafting communication from almost nothing."*

**Minitalk** — a 42 School project that transmits strings between processes using **only UNIX signals**, bit by bit, with a clean MSB-first encoding approach.

<br/>

[⚙️ Algorithm Deep Dive](#%EF%B8%8F-algorithm-deep-dive) · [📡 Signals Explained](#-unix-signals-explained) · [🚀 Usage](#-usage) · [📚 Resources](#-resources)

</div>

---

## 📖 Table of Contents

<details>
<summary>Click to expand</summary>

- [📌 About](#-about)
- [📡 UNIX Signals Explained](#-unix-signals-explained)
- [⚙️ Algorithm Deep Dive](#%EF%B8%8F-algorithm-deep-dive)
  - [Client — string\_format()](#-client--string_format)
  - [Client — kill\_sig()](#-client--kill_sig)
  - [Server — signalhandle()](#-server--signalhandle)
  - [Why MSB-first?](#-why-msb-first-vs-lsb-first)
  - [Full Flow Diagram](#-full-transmission-flow)
- [🗂️ Project Structure](#%EF%B8%8F-project-structure)
- [🛠️ Installation & Compilation](#%EF%B8%8F-installation--compilation)
- [🚀 Usage](#-usage)
- [🧪 Testing](#-testing)
- [❓ Defense FAQ](#-defense-faq)
- [📚 Resources](#-resources)

</details>

---

## 📌 About

**Minitalk** solves a deceptively simple problem: *how do you send a string from one process to another using only two signals?*

Since `SIGUSR1` and `SIGUSR2` can only convey **one bit of information each**, every character must be **decomposed into its 8 binary bits** and transmitted as a sequence of signals. The server reassembles these bits back into characters — one signal at a time.

### Rules

| Constraint | Detail |
|---|---|
| 🚫 No pipes, sockets, or shared memory | Only `kill()` and signal handlers |
| ✅ Allowed signals | `SIGUSR1` and `SIGUSR2` only |
| ✅ Allowed functions | `signal`, `kill`, `getpid`, `pause`, `usleep`, `write`, `malloc`, `free`, `exit` |
| 🔄 Persistent server | Server must handle multiple clients without restarting |

---

## 📡 UNIX Signals Explained

### What is a Signal?

A **signal** is an asynchronous software interrupt delivered to a process by the OS kernel. When a process receives a signal, it pauses execution, runs the registered **signal handler**, then resumes.

```
  ┌─────────────┐         kill(pid, SIG)        ┌─────────────┐
  │   CLIENT    │ ─────────────────────────────► │   KERNEL    │
  │  (sender)   │                                │             │
  └─────────────┘                                │  routes to  │
                                                 │  target PID │
  ┌─────────────┐        deliver signal          └──────┬──────┘
  │   SERVER    │ ◄──────────────────────────────────── ┘
  │ (receiver)  │
  │             │   pause() unblocks
  │  handler()  │   handler runs
  └─────────────┘
```

### SIGUSR1 & SIGUSR2 — Our Binary Alphabet

The OS defines these as **user-defined signals** with no built-in meaning. We give them meaning:

| Signal | Number | Encoding | Meaning |
|--------|--------|----------|---------|
| `SIGUSR1` | 10 | **Bit = 0** | This bit is zero |
| `SIGUSR2` | 12 | **Bit = 1** | This bit is one |

```c
// From kill_sig() — the rule is beautifully simple:
if (sig)              // result of (word & mask) is non-zero → bit is 1
    kill(pid, SIGUSR2);
else                  // result is zero → bit is 0
    kill(pid, SIGUSR1);
```

### `signal()` — Registering the Handler

The server registers **one function** for both signals. The same `signalhandle` fires on either — the `sig` parameter tells it which arrived:

```c
signal(SIGUSR1, signalhandle);
signal(SIGUSR2, signalhandle);
```

> 💡 `signal()` is clean and sufficient for this project. For the bonus (needing the sender's PID for acknowledgments), `sigaction()` with `SA_SIGINFO` provides a `siginfo_t` struct containing `si_pid` — the PID of whoever sent the signal.

---

## ⚙️ Algorithm Deep Dive

this implementation has a distinct, elegant characteristic: it sends bits **MSB-first** using a **sliding bitmask** that shifts right, rather than the common LSB-first approach. Here's how every piece fits together.

---

### 📤 Client — `string_format()`

```c
void    string_format(char word, pid_t pid)
{
    unsigned char   num;
    int             idx;

    idx = 0;
    num = 0x80;              // 0x80 = 10000000  ← start at MSB (bit 7)
    while (idx < 8)
    {
        kill_sig(word & num, pid);   // isolate the current bit with mask
        num = num >> 1;              // slide mask one position right
        idx++;
    }
    return ;
}
```

**The sliding bitmask — step by step:**

`num` starts at `0x80` (`10000000`) and walks **right** through all 8 positions, exposing each bit one at a time from **most significant to least significant**:

```
Character: 'A' = ASCII 65 = 01000001

  Iteration │  num (mask)  │  word & num   │ Non-zero? │ Signal
  ──────────┼──────────────┼───────────────┼───────────┼─────────
     0      │  1000 0000   │  0000 0000    │    No     │ SIGUSR1
     1      │  0100 0000   │  0100 0000    │   Yes     │ SIGUSR2
     2      │  0010 0000   │  0000 0000    │    No     │ SIGUSR1
     3      │  0001 0000   │  0000 0000    │    No     │ SIGUSR1
     4      │  0000 1000   │  0000 0000    │    No     │ SIGUSR1
     5      │  0000 0100   │  0000 0000    │    No     │ SIGUSR1
     6      │  0000 0010   │  0000 0000    │    No     │ SIGUSR1
     7      │  0000 0001   │  0000 0001    │   Yes     │ SIGUSR2

  Signals sent: USR1 USR2 USR1 USR1 USR1 USR1 USR1 USR2
  Bits:           0    1    0    0    0    0    0    1   = 01000001 = 65 = 'A' ✅
```

> 🔑 **Key insight:** `word & num` doesn't give `0` or `1` — it gives either `0` (the masked bit is 0) or some **non-zero value** (the masked bit is 1). `kill_sig` uses a simple `if (sig)` truthiness check, which handles both cases perfectly without needing to normalize to exactly 1.

---

### ⚡ Client — `kill_sig()`

```c
void    kill_sig(int sig, pid_t pid)
{
    if (sig)
        kill(pid, SIGUSR2);   // non-zero → bit is 1 → send SIGUSR2
    else
        kill(pid, SIGUSR1);   // zero     → bit is 0 → send SIGUSR1
    usleep(400);              // wait 400 microseconds before next signal
    return ;
}
```

**Why `usleep(400)`?**

Signals are asynchronous — the OS delivers them when it schedules it. Without a delay, the client can fire signals faster than the server can handle them:

```
  Without delay:                        With usleep(400):

  Client: ▓▓▓▓▓▓▓▓  (burst)            Client: ▓ · · ▓ · · ▓ · · ▓
  Server: ▓▓▓▓ ✗✗✗✗ (loses signals!)   Server: ▓ · · ▓ · · ▓ · · ▓  ✓
```

`400 µs` gives the kernel time to deliver the signal and the server's handler time to execute and return before the next `kill()` fires.

---

### 📥 Server — `signalhandle()`

```c
void    signalhandle(int sig)
{
    static char receive = 0;    // accumulates bits across calls
    static int  idx = 0;        // tracks how many bits collected

    receive = receive << 1;     // shift left: make room for incoming bit
    if (sig == SIGUSR1)
        receive = receive + 0;  // append 0
    else if (sig == SIGUSR2)
        receive = receive + 1;  // append 1
    idx++;
    if (idx == 8)               // collected all 8 bits → full character
    {
        ft_putchar_fd(receive, 1);
        receive = 0;            // reset for next character
        idx = 0;
    }
    return ;
}
```

**The reconstruction — left-shift serial accumulation:**

On every call, `receive` shifts left (making room on the right), and the new bit gets appended. After 8 signals, the original byte has been rebuilt serially:

```
  Signals for 'A' (01000001), received MSB-first:

  Call │  sig   │ receive before <<1 │ receive after <<1 │ + bit │ receive
  ─────┼────────┼────────────────────┼───────────────────┼───────┼──────────
   1   │ USR1=0 │    0000 0000       │    0000 0000      │  +0   │ 0000 0000
   2   │ USR2=1 │    0000 0000       │    0000 0000      │  +1   │ 0000 0001
   3   │ USR1=0 │    0000 0001       │    0000 0010      │  +0   │ 0000 0010
   4   │ USR1=0 │    0000 0010       │    0000 0100      │  +0   │ 0000 0100
   5   │ USR1=0 │    0000 0100       │    0000 1000      │  +0   │ 0000 1000
   6   │ USR1=0 │    0000 1000       │    0001 0000      │  +0   │ 0001 0000
   7   │ USR1=0 │    0001 0000       │    0010 0000      │  +0   │ 0010 0000
   8   │ USR2=1 │    0010 0000       │    0100 0000      │  +1   │ 0100 0001
                                                                   = 65 = 'A' ✅
                                                              → ft_putchar_fd('A', 1)
```

> 💡 **Why `static`?** `signalhandle()` is called 8 separate times per character — once per signal. Local variables would reset to zero on every call. `static` keeps `receive` and `idx` alive in memory between calls, preserving the partial character until all 8 bits arrive.

---

### 🔀 Why MSB-first vs LSB-first?

this implementation sends the **Most Significant Bit first** — a natural, readable choice:

| Approach | Mask start | Bit order | Server reconstruction |
|----------|-----------|-----------|-----------------------|
| **mine — MSB-first** | `0x80` → `>>=1` | 7 → 6 → 5 → ... → 0 | Left-shift accumulate |
| Common — LSB-first | `1` → `<<=1` | 0 → 1 → 2 → ... → 7 | Set bit at position `i` |

MSB-first mirrors then read binary numbers (left to right), and the reconstruction is elegantly serial — each new bit simply slots in on the right as `receive` grows naturally from left to right:

```
  MSB-first build-up for 'H' (01001000):

  After bit 7:  [0]
  After bit 6:  [0 1]
  After bit 5:  [0 1 0]
  After bit 4:  [0 1 0 0]
  After bit 3:  [0 1 0 0 1]
  After bit 2:  [0 1 0 0 1 0]
  After bit 1:  [0 1 0 0 1 0 0]
  After bit 0:  [0 1 0 0 1 0 0 0] = 72 = 'H' ✅
```

---

### 🔄 Full Transmission Flow

```
  CLIENT                                                  SERVER
  ══════                                                  ══════

  main()                                                  main()
   ├─ Check argc == 3                                      ├─ ft_putnbr_fd(getpid(), 1)
   ├─ Validate PID: 100 < pid < 99998                      ├─ signal(SIGUSR1, signalhandle)
   ├─ Loop: i = 0 → ft_strlen(argv[2])                     ├─ signal(SIGUSR2, signalhandle)
   └─ string_format(argv[2][i], pid)                       └─ while(1) pause()
         │                                                        │
         ├─ num = 0x80                                            │ (sleeping in pause)
         │                                                        │
         ├─ iter 0: kill_sig('A' & 0x80, pid)                    │
         │    └─ kill(pid, SIGUSR1)  ──── signal ───────────────► signalhandle(SIGUSR1)
         │       usleep(400)                                      │  receive <<= 1  → 0
         │                                                        │  receive += 0   → 0
         │                                                        │  idx = 1
         ├─ iter 1: kill_sig('A' & 0x40, pid)                    │
         │    └─ kill(pid, SIGUSR2)  ──── signal ───────────────► signalhandle(SIGUSR2)
         │       usleep(400)                                      │  receive <<= 1  → 0
         │                                                        │  receive += 1   → 1
         │                                                        │  idx = 2
         ├─ iter 2–6: (all zeros for 'A') ...                    │  idx = 3 → 7
         │                                                        │
         └─ iter 7: kill_sig('A' & 0x01, pid)                    │
              └─ kill(pid, SIGUSR2)  ──── signal ───────────────► signalhandle(SIGUSR2)
                 usleep(400)                                      │  receive <<= 1  → 64
                                                                  │  receive += 1   → 65
                                                                  │  idx = 8 ✅
                                                                  │  ft_putchar_fd(65) → 'A'
                                                                  │  receive = 0, idx = 0

   Next character → repeat
```

---

## 🗂️ Project Structure

```
Minitalk/
├── Makefile
├── minitalk.h              ← shared header: includes, prototypes, ANSI color macros
├── client.c                ← string_format(), kill_sig(), main()
├── server.c                ← signalhandle(), main()
└── libft/                  ← ft_atoi, ft_strlen, ft_putchar_fd, ft_putstr_fd, ft_putnbr_fd
```

---

## 🛠️ Installation & Compilation

```bash
# Clone the repository
git clone https://github.com/Ayouub-aj/Minitalk.git
cd Minitalk

# Compile both binaries
make
```

### Makefile Targets

```bash
make          # compile server and client
make clean    # remove object files (.o)
make fclean   # remove objects + binaries
make re       # fclean then make
```

---

## 🚀 Usage

### Step 1 — Start the Server

```bash
./server
```

Output:
```
PID: 12345
```

### Step 2 — Send a Message

```bash
./client <server_PID> "<message>"
```

**Examples:**

```bash
./client 12345 "Hello, World!"
./client 12345 "42 Network"
./client 12345 "Signals are just bits in disguise"
```

**Server output:**
```
PID: 12345
Hello, World!
42 Network
Signals are just bits in disguise
```

### PID Validation

Your client guards against invalid PIDs:

```c
if (!(pid > 100 && pid < 99998))
{
    ft_putstr_fd(ANSI_COLOR_RED"Process id error !!\n", 2);
    return (0);
}
```

This catches accidental typos (`./client 0 "hello"`) before any `kill()` call is made.

---

## 🧪 Testing

### Basic

```bash
./server &
PID=$!

./client $PID "Hello"
./client $PID "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
./client $PID "0123456789"
```

### Stress Tests

```bash
# Long string
./client $PID "The quick brown fox jumps over the lazy dog"

# Special characters
./client $PID "!@#\$%^&*()_+-=[]{}|;:',.<>?"

# Repeated rapid sends
for i in {1..10}; do ./client $PID "Message $i"; done
```

### Verify Correctness

```bash
# These should print character-perfect
./client $PID "aA"     # 01100001 01000001
./client $PID "zZ"     # 01111010 01011010
./client $PID " "      # 00100000 (space)
```

### Debug — Watch Signals Live

```bash
# Trace every signal sent by client
strace -e trace=kill ./client $PID "Hi"

# Find server PID if you forgot
pgrep -a server
ps aux | grep server
```

---

## ❓ Defense FAQ

<details>
<summary>📡 <strong>What does kill() actually do?</strong></summary>

Despite the name, `kill(pid, sig)` is simply a **signal-sending syscall**. It sends signal `sig` to process `pid`. The OS kernel delivers it asynchronously. The name is historical — the default action for many signals is termination, hence "kill".

```c
kill(12345, SIGUSR1);
// → asks the kernel to deliver SIGUSR1 to PID 12345
// → the target process's registered handler fires
```

</details>

<details>
<summary>🧱 <strong>Why start the bitmask at 0x80 and not at 1?</strong></summary>

`0x80` = `10000000` is the **most significant bit** of a byte. Starting there and shifting right (`>>=1`) each round walks through bits 7→6→5→4→3→2→1→0, sending them MSB-first.

Starting at `1` and shifting left would send LSB-first (bit 0 first), which also works but requires a different reconstruction on the server side. MSB-first matches how we naturally read binary and makes the serial build-up on the server side very intuitive.

</details>

<details>
<summary>🔁 <strong>Why are receive and idx declared static?</strong></summary>

`signalhandle()` is invoked **8 separate times** to form one character — once per signal. If `receive` and `idx` were regular local variables, they'd reset to zero every time the function was called. `static` makes them persist between calls, so `receive` keeps accumulating bits and `idx` keeps counting until 8 is reached.

</details>

<details>
<summary>⏱️ <strong>Why usleep(400) and not a bigger or smaller value?</strong></summary>

`400 µs` is a balance. Too small and the server drops signals (signal handler hasn't returned before the next arrives). Too large and transmission is unnecessarily slow. `400 µs` is enough for the kernel to deliver the signal and the handler to run and return, while keeping the communication reasonably fast for normal string lengths.

</details>

<details>
<summary>🔄 <strong>How exactly does the server rebuild the character?</strong></summary>

Each call does `receive <<= 1` (push existing bits left, new slot opens on the right), then adds `0` or `1` based on the signal. After 8 calls this serial stream has reconstructed the original byte exactly as sent. It's the mirror image of the client's encoding — the client exposes bits left-to-right using a mask, the server absorbs them left-to-right using a shift.

</details>

<details>
<summary>♻️ <strong>Can the server handle multiple clients?</strong></summary>

The server loops with `while(1) pause()` and never exits — it can receive from many clients sequentially. However, if two clients transmit simultaneously, their signals could interleave, corrupting both messages. The `usleep(400)` on the client side mitigates this in practice. True concurrent support would require tracking sender PIDs via `sigaction` with `SA_SIGINFO`.

</details>

<details>
<summary>🔢 <strong>What's the difference between signal() and sigaction()?</strong></summary>

| Feature | `signal()` | `sigaction()` |
|---------|-----------|---------------|
| Simplicity | ✅ One-liner | More verbose |
| Portability | Varies by OS | POSIX standard ✅ |
| Get sender's PID | ✗ | ✅ via `siginfo_t.si_pid` |
| Block signals during handler | ✗ | ✅ |
| Stays registered after call | System-dependent | Always ✅ |

`signal()` is perfect for this mandatory part. For a bonus acknowledgment system where the server needs to reply to the correct client, `sigaction` with `SA_SIGINFO` is required.

</details>

---

## 📚 Resources

### 📖 Man Pages

| Function | Link |
|----------|------|
| `signal()` | [man7.org/signal.2](https://man7.org/linux/man-pages/man2/signal.2.html) |
| `sigaction()` | [man7.org/sigaction.2](https://man7.org/linux/man-pages/man2/sigaction.2.html) |
| `kill()` | [man7.org/kill.2](https://man7.org/linux/man-pages/man2/kill.2.html) |
| `pause()` | [man7.org/pause.2](https://man7.org/linux/man-pages/man2/pause.2.html) |
| `usleep()` | [man7.org/usleep.3](https://man7.org/linux/man-pages/man3/usleep.3.html) |
| `getpid()` | [man7.org/getpid.2](https://man7.org/linux/man-pages/man2/getpid.2.html) |

### 🎓 Learning

| Resource | Description |
|----------|-------------|
| [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/) | The best intro to inter-process communication |
| [GNU C — Signal Handling](https://www.gnu.org/software/libc/manual/html_node/Signal-Handling.html) | Deep reference on signals |
| [Bitwise Operations in C](https://en.wikipedia.org/wiki/Bitwise_operations_in_C) | Masking, shifting, all operators explained |
| [ASCII Table](https://www.asciitable.com/) | Char → decimal → binary reference |
| [POSIX Signals](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html) | Full POSIX signal specification |

### 🔢 Quick Binary Reference

```
Char │ Dec │  Hex  │  Binary
─────┼─────┼───────┼──────────
 'A' │  65 │ 0x41  │ 01000001
 'Z' │  90 │ 0x5A  │ 01011010
 'a' │  97 │ 0x61  │ 01100001
 'z' │ 122 │ 0x7A  │ 01111010
 '0' │  48 │ 0x30  │ 00110000
 '9' │  57 │ 0x39  │ 00111001
 ' ' │  32 │ 0x20  │ 00100000
'\n' │  10 │ 0x0A  │ 00001010
'\0' │   0 │ 0x00  │ 00000000
```

---

<div align="center">

### 📊 Implementation Summary

| Component | Your Technique |
|-----------|----------------|
| Bit extraction | Bitmask `0x80` sliding right with `>>=` |
| Bit order | **MSB-first** (bit 7 → bit 0) |
| Encoding `0` | `SIGUSR1` |
| Encoding `1` | `SIGUSR2` |
| Signal pacing | `usleep(400)` between every bit |
| Reconstruction | `receive <<= 1` then `+= 0/1` per signal |
| State persistence | `static char receive` + `static int idx` |
| Server loop | `while(1) pause()` — zero CPU until signal |

<br/>

**Made with ⚡ by [Ayouub-aj](https://github.com/Ayouub-aj)**

*Found this helpful? Drop a ⭐ on the repo!*

<img src="https://img.shields.io/github/stars/Ayouub-aj/Minitalk?style=social"/>

</div>
