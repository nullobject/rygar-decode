# Rygar Decode

Dump ROMs:

```sh
make
```

Edit char tiles:

```sh
hexedit -l 8 cpu_8k.hex
```

Dump char tiles:

```sh
hexdump -v -e '4/1 "%02X" "\n"' cpu_8k.hex
```
