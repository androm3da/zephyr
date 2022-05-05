#define HDR_ATTR __attribute__((section(".start"))) __attribute__((used))

void __start(void);

static HDR_ATTR void (*_entry_point)(void) = &__start;


int started = 0;
void __start(void)
{
    started = 1;
    main();
    while (started);
    return;
}
