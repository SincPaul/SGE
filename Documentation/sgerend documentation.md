**SGEREND**

SGEREND is a binary file format to structed store and organize 3D rendering Data. <br>
It is little Endian encoded and starts with a global header that leaves space for future extensions.<br>
After that an amount of sections follows where Aspects of a Renering object are defined, such as meshes, textures and index buffer.

sgerend is meant to be used for SGE (Stein Game Engine) 

**Header 86 Byte excluding extension data**

Each sgerend file starts with an 86 Byte header(excluding extension data), that includes metadata, such as the version, section count and a checksum for data integrity

| Offset | Size          | Name                   | Description      |
|:-------|:--------------|:-----------------------|:-----------------|
| 0x00   | 8 byte        | Magic number           | "SGEREND\0"      |
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

Each Section Starts with an 88 Byte Header(exluding extensions), that defines metadata <br>
such as the type (mesh, material, ...), offset in the file size and a checksum for data integrity

After the Header the Data starts where each type has a different Section, with different Metadata <br>
Each defined below(partially for now)

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

| Value | Type           | Description                |
|:------|:---------------|:---------------------------|
| 1     | Mesh           | SGE_SECTION_MESH           |
| 2     | Material       | SGE_SECTION_MATERIAL       |
| 3     | Texture        | SGE_SECTION_TEXTURE        |
| 4     | Shader Binding | SGE_SECTION_SHADER_BINDING |
| 5     | Index Buffer   | SGE_SECTION_INDEX_BUFFER   |
| 6     | Metadata       | SGE_SECTION_METADATA       |

---------------------------------------------------------------------------------------------

**Mesh Section**

**Header** <br>
Normal Header as defined above

**Data** <br>
Section Data: 

| Offset | Size   | Name            | Description                     |
|:-------|:-------|:----------------|:--------------------------------|
| +0x00  | 4 Byte | vertex_count    | Number of vertices (uint32_t)   | 
| 0x04   | 4 Byte | vertex_size     | Size of each vertex (uint32_t)  |
| 0x08   | 4 Byte | attribute_count | Number of attributes (uint32_t) |


Attribute Data<br>
foreach attribute:

| Offset | Size   | Name       | Description                          | 
|:-------|:-------|:-----------|:-------------------------------------|
| 0x00   | 2 Byte | type       | Position, Normal, etc (uint16_t)     | 
| 0x02   | 2 Byte | format     | float32, etc (uint16_t)              | 
| 0x04   | 2 Byte | components | Number of values (uint16_t)          | 
| 0x06   | 2 Byte | offset     | Byte offset within vertex (uint16_t) | 

Attribute Types

//V0.1

| Value | Type      | Description             | 
|:------|:----------|:------------------------|
| 1     | Position  | SGE_ATTRIBUTE_POSITION  |
| 2     | Normal    | SGE_ATTRIBUTE_NORMAL    | 
| 3     | Color     | SGE_ATTRIBUTE_COLOR     |
| 4     | Texcoords | SGE_ATTRIBUTE_TEXCOORDS |
| 5     | Tangent   | SGE_ATTRIBUTE_TANGENT   |
| 6     | Bitangent | SGE_ATTRIBUTE_BITANGENT |
| 7     | Weights   | SGE_ATTRIBUTE_WEIGHTS   |
| 8     | JointIDs  | SGE_ATTRIBUTE_JOINTIDS  |

Attribute Formats

| Value | Format  | Description        |
|:------|:--------|:-------------------|
| 1     | Float32 | SGE_FORMAT_FLOAT32 |
| 2     | Float16 | SGE_FORMAT_FLOAT16 |
| 3     | Int8    | SGE_FORMAT_INT8    |
| 4     | UInt8   | SGE_FORMAT_UINT8   | 
| 5     | Int16   | SGE_FORMAT_INT16   |
| 6     | UInt16  | SGE_FORMAT_UINT16  |
| 7     | Int32   | SGE_FORMAT_INT32   |
| 8     | UInt32  | SGE_FORMAT_UINT32  |


Data Size is defined as <br>
[data_size] = [vertex count] * [vertex size]

//todo manually atm not implemented <br>
**Example**

```hexdump
//Header ignored for simplicity

//Data start
//Section Data
20 00 00 00 //vertex_count 32
10 00 00 00 //vertex_size 16
02 00 00 00 //attribute_count 2
                  
//Attributes
//Atribute 1 - Size 12 (x,y,z (3) * float32 (4))
01 00 //Position
01 00 //Float 32
03 00 //XYZ
00 00 //Offset 0      

//Attribute 2 - Size 4 (RGBA (4) * uint8 (1))
03 00 //Color
04 00 //Uint8
04 00 //RGBA
0C 00 //Offset 12   

//foreach vertex(16 Byte)
//Position (12 Byte)
00 00 00 00         // X Position (Float32)
00 00 00 00         // Y Position (Float32)
00 00 00 00         // Z Position (Float32)
     
//Color (4 Byte)                                         
FF                  // Red    Color Value (uin8_t)
00                  // Green  Color Value (uin8_t)
00                  // Blue   Color Value (uin8_t)
FF                  // Alpha        Value (uin8_t)

//Repeat 31 times for each verticie

//Full Vertex Data Size = 512 = (vertex_count) 32 * (vertex_size) 16
```

---------------------------------------------------------------------------------------------

**Material Section**

**Header**<br>
Normal Header as defined above

**Data**<br>
Section Data: 

| Offset | Size   | Name                 | Description                            |
|:-------|:-------|:---------------------|:---------------------------------------|
| 0x00   | 4 Byte | parameter_count      | Number of parameters (uint32_t)        |
| 0x04   | 4 Byte | shader_binding_index | Reference to shader binding (uint32_t) |

Parameter Data <br>
foreach parameter:

| Offset | Size          | Name        | Description                           |
|:-------|:--------------|:------------|:--------------------------------------|
| 0x00   | 2 Byte        | type        | Parameter Type (uint16_t)             |
| 0x02   | 2 Byte        | datatype    | Data storage format (uint16_t)        |
| 0x04   | 32 Byte       | name        | Parameter Name (char[32])             |
| 0x24   | Variable Size | value       | Parameter value (union based type)    |

Parameter Types

| Value | Type      | Description                 | 
|:------|:----------|:----------------------------|
| 1     | Color     | RGB/RGBA values             |
| 2     | Roughness | Surface Roughness           |
| 3     | Direction | Normalized Vector Direction |

Parameter Data Types

| Value | Type              | Size | Description                | Use Cases                               |
|:------|:------------------|:-----|:---------------------------|:----------------------------------------|
| 1     | Float32           | 4    | SGE_MATERIAL_PARAM_FLOAT   | Single float                            |
| 2     | Vec2 (Float32[2]) | 8    | SGE_MATERIAL_PARAM_VEC2    | 2D Vector (XY)                          |
| 3     | Vec3 (Float32[3]) | 12   | SGE_MATERIAL_PARAM_VEC3    | 3D Vector (XYZ)                         |
| 4     | Vec4 (Float32[4]) | 16   | SGE_MATERIAL_PARAM_VEC4    | 4D vector (XYZA)                        |
| 5     | Texture           | 4    | SGE_MATERIAL_PARAM_TEXTURE | Reference to texture section (uint32_t) |


**Example**

```hexdump
//Header ignored for simplicity

//Material Section Data
02 00 00 00         //parameter_count 2
01 00 00 00         //shader_binding_index 1

//Parameter 1
01 00               //type 1 = color
03 00               //datatype 3 = Vec3
74 65 73 74         //name 32 byte fully null termianted
20 70 61 72         //name = "test parameter name"
61 6D 65 74 
65 72 20 6E 
61 6D 65 00
00 00 00 00
00 00 00 00
00 00 00 00

3F 80 00 00         // R = 1.0f (float32)
00 00 00 00         // G = 0.0f (float32)
00 00 00 00         // B = 0.0f (float32)

02 00               //type 2 = Roughness
01 00               //datatype 1 = float32
72 6F 75 67         //name 32 byte full null terminated
68 6E 65 73         //name = "roughness"
73 00 00 00
00 00 00 00
00 00 00 00
00 00 00 00
00 00 00 00
00 00 00 00

3E 99 99 99         //float 0.3 in hex
```

---------------------------------------------------------------------------------------------

**Index Buffer Section**


**Header**<br>
Normal Header as defined above

**Data**<br>
Section Data: 
 
| Offset | Size   | Name           | Description                             |
|:-------|:-------|:---------------|:----------------------------------------|
| 0x00   | 4 Byte | index_count    | Number of indicies (uint32_t)           |
| 0x04   | 2 Byte | index_size     | 2 or 4 Byte                             | 
| 0x06   | 4 Byte | Primitive Type | Type of "SGE_PRIMITIVE_TYPE" (uint32_t) | 

primitive type: 

| Value | Type           | Description                  |
|:------|:---------------|:-----------------------------|
| 1     | points         | SGE_PRIMITIVE_POINTS         |
| 2     | lines          | SGE_PRIMITIVE_LINES          |
| 3     | triangle       | SGE_PRIMITIVE_TRIANGLE       |
| 4     | triangle_strip | SGE_PRIMITIVE_TRIANGLE_STRIP |

**Examples**

```hexdump
//Header ignored for simplicity

//Index Buffer Section
06 00 00 00         //Index Count = 6 (2 triangles)
02 00               //index size = 2 (16 bit bc less that 65535 indicies) otherwise 4
03 00 00 00         //primative type = 2 (triangles)

//Index Data
00 00               //index[0] = 0 
01 00               //index[1] = 1
02 00               //index[2] = 2
01 00               //index[3] = 1
03 00               //index[4] = 3
02 00               //index[5] = 2

//Total Data Size = 12 + 12 = 24
```

---------------------------------------------------------------------------------------------

**Extensions**

| Offset | Size          | Name                | Description         |
|:-------|:--------------|:--------------------|:--------------------|
| 0x00   | 2 Byte        | Extension Type      | LOD, etc            |
| 0x02   | 4 Byte        | Extension Data Size | Extension Data Size |
| 0x06   | Variable Size | Extension Data      | Extension Data      |

---------------------------------------------------------------------------------------------

MA = Major Version <br>
MI = Minor Version <br>
PA = Patch Version <br>
SEC = Section Count <br>
EXC = Extension Count <br>
CRC32 = Crc32 checksum <br>
TYPE = TYPE <br>
OFF = OFFSET <br>
DASI = DATA SIZE <br>
