#ifndef C_TYPES_LIB
#define C_TYPES_LIB


void printHex_ascii(unsigned char ascii) {
    ascii += 0x30;
    if(ascii <= 0x39) {
        print_char(ascii);
    } else {
        ascii += 39;
        print_char(ascii);
    }
}

void printHexByte(unsigned char byte) {
    printHex_ascii(byte >> 4);
    printHex_ascii(byte & 0x0F);
}

void printHexWord(unsigned short word) {
    printHexByte(word >> 8);
    printHexByte(word & 0x00FF);
}

void printHexDword(unsigned int dword) {
    printHexWord(dword >> 16);
    printHexWord(dword & 0x0000FFFF);
}

#endif