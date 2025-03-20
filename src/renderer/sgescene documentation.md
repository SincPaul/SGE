**Little Endian**<br>
**Header 86 Byte excluding extension data**

| Offset | Size          | Name                   | Description      |
|:-------|:--------------|:-----------------------|:-----------------|
| 0x00   | 8 Byte        | Magic Number           | "SGESCNE\0"      |
| 0x08   | 2 Byte        | Major version          | 16 bit           |
| 0x0A   | 2 Byte        | Minor version          | 16 bit           |
| 0x0C   | 2 Byte        | Patch                  | 16 bit           |
| 0x0E   | 2 Byte        | Section Count          | 16 Bit           |
| 0x10   | 2 Byte        | Header Extension Count | Extension Count  |
| 0x12   | Variable Size | Header Extension Data  | Extension Data   |
| 0x..   | 64 Byte       | Renderable Name        | Name to identify |
| 0x...  | 4 Byte        | Header Checksum        | CRC32 Checksum   |

---------------------------------------------------------------------------------------------

**Section Header 88 Byte excluding Extension data**

| Offset | Size          | Name                    | Description                    |
|:-------|:--------------|:------------------------|:-------------------------------|
| 0x00   | 2 Byte        | Section Type            | Mesh, Material, etc            |
| 0x02   | 8 Byte        | Section Offset          | File offset of section         |
| 0x0A   | 8 Byte        | Data Size               | Size of Section without header |
| 0x12   | 2 Byte        | Section Extension Count | Extension Count                |
| 0x14   | Variable Size | Section Extension Data  | Extension Data                 |
| 0x..   | 64 Byte       | Section Name            | Name to identify               |
| 0x...  | 4 Byte        | Section Checksum        | CRC32 Checksum                 |

---------------------------------------------------------------------------------------------

**Section Types**

V0.1

| Value | Type | Description |
|:------|:-----|:------------|
|       |      |             |