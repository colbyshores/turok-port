# 3DS CIA packaging tools

`make -f Makefile.3ds cia` builds an installable **`build_3ds/turok.cia`** (HOME-menu app) in addition to the
`.3dsx`. It needs two third-party tools that are **not** in devkitPro and are **not committed** (binaries,
licensing, platform-specific) — drop the Linux x86_64 binaries into this directory:

| tool         | what it does                                  | where to get it |
|--------------|-----------------------------------------------|-----------------|
| `makerom`    | ELF + RSF + icon + banner → CIA               | https://github.com/3DSGuy/Project_CTR/releases (`makerom-*-ubuntu_x86_64.zip`) |
| `bannertool` | PNG → SMDH icon, PNG + WAV → banner (`.bnr`)   | https://github.com/carstene1ns/3ds-bannertool/releases (`bannertool-*-linux.tar.gz`) |

```sh
# from repo root:
mkdir -p tools/3ds-cia && cd tools/3ds-cia
curl -sL https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.19.0/makerom-v0.19.0-ubuntu_x86_64.zip -o m.zip && unzip -o m.zip && chmod +x makerom
curl -sL https://github.com/carstene1ns/3ds-bannertool/releases/download/1.2.3/bannertool-1.2.3-linux.tar.gz | tar xz --strip-components=1
chmod +x bannertool
```

## Inputs (also gitignored, user-supplied like the ROM)
- `turok.jpg` (repo root) — **banner** art. Resized by the build to **256×128**.
- `turok.ico` (repo root) — **HOME-menu icon** art. The build picks the largest frame of the multi-size `.ico`
  and downscales it to **48×48** (a plain `.png`/`.jpg` works too — override `CIA_ICON=`).
- `turok.wav` (repo root) — banner audio (any PCM WAV; bannertool converts it to CWAV).

## Build
```sh
make -f Makefile.3ds            # the .3dsx (and the .elf the CIA needs)
make -f Makefile.3ds cia        # -> build_3ds/turok.cia
```
Spec: [`port/3ds/turok.rsf`](../../port/3ds/turok.rsf) (committed). App metadata / unique-id are Makefile vars
(`APP_TITLE`, `APP_UNIQUE_ID` = `0xff3ff`, etc.) — override on the make command line if needed. `ImageMagick`
(`convert`) does the resize; override `CONVERT=` if it's elsewhere.
