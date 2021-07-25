
void _start (void)
{
    int i;

    while(i++ <= 100)
    {
        asm("nop");
    }
    asm("nop");
}