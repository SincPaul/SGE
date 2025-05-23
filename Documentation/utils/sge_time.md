# SGE Time Utils

## Overview

## Table of Contents

1. [API Reference](#api-reference)
    1.1 [current_time_formatted](#char-current_time_formatted) <br>
    1.2 [get_current_ms_time](#uint64_t-get_current_ms_time) <br>
    1.3 [get_current_year](#int-get_current_year) <br>
    1.4 [get_current_month](#int-get_current_month) <br>
    1.5 [get_current_day](#int-get_current_day) <br>

## API Reference

### `char* current_time_formatted()`

```c
char* current_time_formatted();
```

#### Returns:
- `Pointer`: formatted timestamp `[04.04.2025 13:23:00]`

---

### `uint64_t get_current_ms_time(...)`

```c
uint64_t get_current_ms_time();
```

#### Returns:
- `uint64_t`: current unix timestamp

---

### `int get_current_year()`

```c
int get_current_year();
```

#### Returns:
- `int`: current year 

---

### `int get_current_month()`

```c
int get_current_month();
```

#### Returns:
- `int`: current month

---

### `int get_current_day()`

```c
int get_current_day();
```

#### Returns:
- `int`: current day

---