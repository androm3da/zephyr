
zephyr_library()

zephyr_library_sources(
  prep_c.c
  swap.c
  fatal.c
  isr.S
  cpu_idle.c
  semihost.c
  swap_helper.S
  thread.c
)
