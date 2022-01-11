# Currently Unnamed Autoclicker

## Constraints

Is linux + X.org **only**.
you must use sudo or add user to `input` group (for `/dev/input/mice`)

## Why?

* sick [cps](https://cpstest.org/)
* minecraft (also btw if you get banned it's not my fault)
* it's cool (duh)

## How to run

Compile:
```bash
make
```

Usage:
```bash
bin/autoclicker [CPS]
```

## How it works

there are 5 threads running:

* main thread (initializes stuff, then waits to be interupted by `SIGINT` (`Ctrl`+`C`), and then cleans up)
* autoclicker for the left click   (implemented by `AsyncAutoClicker`)
* autoclicker for the right click  (implemented by `AsyncAutoClicker`)
* listens for real mouse clicks (needs access to `/dev/input/mice`) (worker thread in `MimicMouseButFaster`)
* listens for CapsLock to toggle autoclicking (listen thread in `MimicMouseButFaster`)

## TO DO

* Ctrl+C not working
* Crashes on window close
* make not important logs show only in verbose mode
* get rid of sudo requirement

