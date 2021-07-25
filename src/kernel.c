void kmain(void) {
    char* ptr = (char*) 0xb8000;

    ptr[0] = 'X';
    ptr[1] = 0x2f;
}