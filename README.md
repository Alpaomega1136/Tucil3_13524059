# Tucil3_13524059

Implementasi C++ untuk pencarian jalur pada graf berbobot menggunakan Dijkstra
dan A*.

## Struktur

```text
.
├── include/        # Header C++
├── src/            # Source C++
├── bin/            # Output executable
├── build/          # Object file hasil kompilasi
├── doc/            # Dokumen tugas
├── test/           # Contoh input
└── Makefile
```

## Build

```bash
make
```

Executable akan dibuat di `bin/tucil3`.

## Run

```bash
./bin/tucil3 test/sample.txt astar
./bin/tucil3 test/sample.txt dijkstra
```

Atau:

```bash
make run
```

`make run` hanya menampilkan bantuan karena target ini tidak mengirim argumen
file input.

## Format Input

```text
<jumlah_simpul> <jumlah_sisi>
<id_simpul> <x> <y>
...
<asal> <tujuan> <bobot>
...
<simpul_awal> <simpul_tujuan>
```

Catatan:

- Nomor simpul dimulai dari `0`.
- Sisi pada input dianggap tidak berarah.
- Koordinat dipakai sebagai heuristik Euclidean untuk A*.

Contoh tersedia di `test/sample.txt`.
