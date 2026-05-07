# Tucil3_13524059

Tugas Kecil 3 IF2211 Strategi Algoritma 2025/2026.

## Penjelasan Singkat Program

Program ini merupakan penyelesai permainan **Ice Sliding Puzzle** menggunakan pendekatan pencarian jalur pada graph. Papan permainan dibaca dari file input, lalu direpresentasikan sebagai graph sederhana yang berisi titik-titik tempat pemain dapat berhenti setelah melakukan sliding.

Pada permainan ini, pemain bergerak ke satu arah dan akan terus meluncur sampai berhenti tepat sebelum rintangan. Program mencari jalur valid dari titik awal `Z` menuju finish `O` dengan tetap memperhatikan tile penting berupa angka `0` sampai `9` yang harus dilewati sesuai urutan.

Algoritma pathfinding yang tersedia:

- `UCS`: Uniform Cost Search, memakai total cost aktual `g(n)`.
- `GBFS`: Greedy Best First Search, memakai nilai heuristik `h(n)`.
- `A*`: A Star, memakai `g(n) + h(n)`.

Heuristik yang tersedia untuk `GBFS` dan `A*`:

- `H1`: FinishOnly, menghitung jarak ke finish.
- `H2`: ImportantSequence, menghitung jarak ke target penting berikutnya.
- `H3`: OrderedSequence, menghitung estimasi jarak ke seluruh target penting yang belum selesai secara berurutan sampai finish.

## Requirement dan Instalasi

Requirement utama:

- C++17
- `g++`
- `make`

Requirement tambahan untuk GUI:

- `cmake` minimal versi 3.16
- `raylib`

Jika `raylib` belum terpasang, build GUI melalui CMake akan mencoba mengunduh `raylib` secara otomatis menggunakan `FetchContent`.

## Cara Mengkompilasi Program

### CLI

Kompilasi program CLI dengan Makefile:

```bash
make
```

Hasil executable CLI berada di:

```text
bin/tucil3
```

Untuk membersihkan hasil build CLI:

```bash
make clean
```

Untuk build ulang:

```bash
make rebuild
```

### GUI

Kompilasi program GUI dengan CMake:

```bash
cmake -S . -B bin/cmake-build
cmake --build bin/cmake-build --target tucil3_gui
```

Hasil executable GUI berada di:

```text
bin/tucil3-gui
```

## Cara Menjalankan dan Menggunakan Program

### Menjalankan CLI

Jalankan program CLI dengan Makefile:

```bash
make run
```

Saat program berjalan, pengguna memasukkan:

- nama file input,
- algoritma pathfinding (`UCS`, `GBFS`, atau `A*`),
- jenis heuristik (`H1`, `H2`, atau `H3`) untuk `GBFS` dan `A*`,
- pilihan untuk menyimpan hasil solusi.

Jika nama file input tidak memuat folder, program akan mencari input dari folder:

```text
test/input/
```

Contoh input `test1.txt` akan dibaca sebagai:

```text
test/input/test1.txt
```

Jika hasil disimpan, file output akan berada di:

```text
test/output/
```

### Menjalankan GUI

Jalankan GUI melalui target CMake:

```bash
cmake --build bin/cmake-build --target run
```

Atau jalankan executable langsung:

```bash
./bin/tucil3-gui
```

Pada GUI, pengguna dapat memasukkan nama file input, memilih algoritma, memilih heuristik, menjalankan pencarian, melihat visualisasi solusi, dan menyimpan hasil solusi.

## Format Input

Baris pertama berisi ukuran papan:

```text
N M
```

`N` baris berikutnya berisi peta permainan. Setelah itu, `N` baris berikutnya berisi cost traversal setiap tile.

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

- `*`: tile kosong yang dapat dilewati.
- `X`: rintangan atau batu.
- `L`: lava.
- `Z`: posisi awal.
- `O`: finish.
- `0` sampai `9`: tile penting yang harus dilewati berurutan.

Contoh file input dapat diletakkan di:

```text
test/input/test.txt
```

## Author

Raymond Jonathan Dwi Putra Julianto  
NIM: 13524059  
IF2211 Strategi Algoritma
