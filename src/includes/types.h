#ifndef C_TYPES_LIB
#define C_TYPES_LIB

void print_hex4bit(uint8_t char4bit) {
    char4bit += 0x30;
    if(char4bit <= 0x39) {
        print_char(char4bit);
    } else {
        char4bit += 39;
        print_char(char4bit);
    }
}

void print_hexb(uint8_t byte) {
    print_hex4bit(byte >> 4);
    print_hex4bit(byte & 0x0F);
}

void print_hexw(uint16_t word) {
    print_hexb(word >> 8);
    print_hexb(word & 0x00FF);
}

void print_hexdw(uint32_t dword) {
    print_hexw(dword >> 16);
    print_hexw(dword & 0x0000FFFF);
}

#endif