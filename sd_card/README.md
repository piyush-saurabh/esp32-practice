# SD Card interfacing

### Pin Connection

SPI Reader|ESP 32 Pin|
----------|----------|
3v3       | 3v3      |
CS        | GPIO 5   |
MOSI      | GPIO 23  |
CLK       | GPIO 18  |
MOSI      | GPIO 19  |
GND       | GND      |

### fopen modes
Value|Description|
----------|----------|
r       | Open for **reading only**      |
w       | Create for **writing only**. If a file by that name already exists, it will be **overwritten**.      |
a       | Append. New content will be written at **end-of-file** or **create** for writing if the file does not exist.      |
r+      | Open an **existing** file for **update** (reading and writing).      |
w+      | Create a new file for **update** (reading and writing). If a file by that name already exists, it will be **overwritten**.      |
a+       | Append. Open (or create if the file does not exist) for update at the **end of the file**.      |