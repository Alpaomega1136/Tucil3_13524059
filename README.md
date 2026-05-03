# Tucil3_13524059

Tugas Kecil 3 IF2211 Strategi Algoritma 2025/2026.

Program untuk menyelesaikan permainan **Ice Sliding Puzzle Solver** dengan pendekatan graph search. Aktor bergerak dengan mekanisme sliding: ketika memilih arah, aktor terus berjalan sampai berhenti tepat sebelum rintangan `X`. Jalur yang melewati lava `L`, keluar papan, atau melewati angka tidak sesuai urutan dianggap tidak valid.

## Deskripsi Singkat

Papan permainan dibaca dari file `.txt`. Program membangun graph sederhana dari posisi-posisi tempat aktor dapat berhenti. Setiap edge menyimpan satu gerakan sliding lengkap, termasuk arah gerak, cost total tile yang dilewati, dan tile penting yang dilewati selama sliding.

Solusi dianggap valid jika:

- Aktor berhenti tepat di tile tujuan `O` (hutuf O).
- Semua angka pada papan sudah dilewati sesuai urutan dari `0` (angka 0) sampai angka terbesar yang ada (maksimal angka 9).
- Jalur tidak melewati lava `L`.
- Gerakan yang tidak memiliki rintangan sebagai titik berhenti dianggap game over dan tidak dimasukkan ke graph.

## Algoritma

Program menyediakan tiga algoritma pencarian:

- `UCS`: Uniform Cost Search, memakai total cost aktual dari start.
- `GBFS`: Greedy Best First Search, memakai nilai heuristik sebagai prioritas.
- `A*`: A Star, memakai `g(n) + h(n)` sebagai prioritas.

Untuk `GBFS` dan `A*`, tersedia tiga pilihan heuristik:

- `H1`: FinishOnly, jarak Manhattan dari node saat ini ke finish `O`.
- `H2`: ImportantSequence, jarak Manhattan ke angka penting berikutnya. Jika semua angka selesai, jarak dihitung ke finish `O`.
- `H3`: OrderedSequence, jumlah estimasi jarak Manhattan dari posisi saat ini ke semua target yang belum selesai secara berurutan, lalu ke finish `O`.

Semua algoritma tetap memvalidasi urutan angka. Perbedaan `H1`, `H2`, dan `H3` hanya memengaruhi prioritas pencarian pada algoritma yang memakai heuristik.

## Requirement

- C++17
- `g++`
- `make`
- `cmake` untuk build otomatis dengan dependency GUI
- `raylib` untuk build versi GUI. Jika memakai CMake dan raylib belum tersedia, CMake akan mencoba mengunduh raylib otomatis.

## Struktur Direktori

```text
.
├── bin/              # Executable dan object file hasil build
├── doc/              # Dokumen spesifikasi tugas
├── include/          # Header C++
├── src/              # Source C++
├── test/
│   ├── input/        # File input test case
│   └── output/       # File output solusi
├── Makefile
└── README.md
```

## Build

### Makefile

```bash
make
```

Secara default, `make` akan membangun versi CLI. Target build yang tersedia:

```bash
make CLI
make GUI
```

Hasil build:

- CLI: `bin/tucil3`
- GUI: `bin/tucil3-gui`

Untuk membersihkan hasil build:

```bash
make clean
```

Untuk build ulang dari awal:

```bash
make rebuild
```

### CMake

CMake disediakan agar dependency GUI `raylib` bisa dicari otomatis. Jika raylib belum terpasang, CMake akan mencoba mengunduh raylib melalui `FetchContent`.

```bash
cmake -S . -B cmake-build
cmake --build cmake-build --target tucil3
cmake --build cmake-build --target tucil3_gui
```

Hasil executable tetap dibuat di:

```text
bin/tucil3
bin/tucil3-gui
```

## Cara Menggunakan CLI

Build versi CLI:

```bash
make CLI
```

Jalankan dengan argumen file input:

```bash
./bin/tucil3 test.txt
```

Jika nama file tidak memuat folder, program akan mencari file tersebut di:

```text
test/input/test.txt
```

Program juga dapat dijalankan tanpa argumen:

```bash
./bin/tucil3
```


## Cara Menggunakan GUI

Build versi GUI:

```bash
make GUI
```

Jika raylib belum tersedia secara manual, gunakan CMake agar raylib diunduh otomatis:

```bash
cmake -S . -B cmake-build
cmake --build cmake-build --target tucil3_gui
```

Jalankan GUI:

```bash
./bin/tucil3-gui
```

## Format Input

Baris pertama berisi ukuran papan:

```text
N M
```

`N` baris berikutnya berisi peta permainan. Setelah itu, `N` baris berikutnya berisi cost traversal untuk setiap tile.

Format umum:

```text
<jumlah_baris> <jumlah_kolom>
<peta baris 1>
<peta baris 2>
...
<peta baris N>
<cost baris 1>
<cost baris 2>
...
<cost baris N>
```

Keterangan tile:

- `*`: path yang bisa dilewati.
- `X`: rintangan atau batu. Aktor berhenti tepat sebelum tile ini.
- `L`: lava. Jalur yang melewati tile ini tidak valid.
- `Z`: posisi awal aktor.
- `O`: titik tujuan.
- `0` sampai `9`: tile penting yang harus dilewati sesuai urutan.

Ketentuan input:

- Papan boleh berbentuk persegi atau persegi panjang.
- Harus ada tepat satu `Z`.
- Harus ada tepat satu `O`.
- Jika ada angka, angka harus berurutan mulai dari `0`.
- Cost harus tersedia untuk semua tile, termasuk `X` dan `L`, walaupun cost `X` dan `L` tidak dihitung dalam solusi valid.

Contoh input tersedia di:

```text
test/input/test.txt
```

