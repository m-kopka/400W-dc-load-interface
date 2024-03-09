#include "display_driver/fonts.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

const uint8_t font_6x8[] = {

    6, 	 	// font width
	8, 	 	// font height
	' ', 	// first char
	'z',	// last char

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // 032 - ' '
	0x00, 0x5F, 0x5F, 0x00, 0x00, 0x00,     // 033 - '!'
	0x00, 0x03, 0x00, 0x03, 0x00, 0x00,     // 034 - '"'
	0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00,     // 035 - '#'
	0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00,     // 036 - '$'
	0x23, 0x13, 0x08, 0x64, 0x62, 0x00,     // 037 - '%'
	0x36, 0x49, 0x56, 0x20, 0x50, 0x00,     // 038 - '&'
	0x00, 0x00, 0x03, 0x00, 0x00, 0x00,     // 039 - '''
	0x00, 0x1C, 0x22, 0x41, 0x00, 0x00,     // 040 - '('
	0x00, 0x41, 0x22, 0x1C, 0x00, 0x00,     // 041 - ')'
	0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00,     // 042 - '*'
	0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,     // 043 - '+'
	0x50, 0x30, 0x00, 0x00, 0x00, 0x00,     // 044 - ','
	0x08, 0x08, 0x08, 0x08, 0x08, 0x00,     // 045 - '-'
	0x60, 0x60, 0x00, 0x00, 0x00, 0x00,     // 046 - '.'
	0x00, 0x60, 0x1C, 0x03, 0x00, 0x00,     // 047 - '/'
	0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,     // 048 - '0'
	0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,     // 049 - '1'	
	0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 	// 050 - '2'		
	0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 	// 051 - '3'
	0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 	// 052 - '4'	
	0x27, 0x45, 0x45, 0x45, 0x39, 0x00,     // 053 - '5'
	0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 	// 054 - '6'	
	0x01, 0x71, 0x09, 0x05, 0x03, 0x00,     // 055 - '7'	
	0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 	// 056 - '8'		
	0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 	// 057 - '9'		
	0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 	// 058 - ':'		
	0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 	// 059 - ';'		
	0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 	// 060 - '<'		
	0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 	// 061 - '='		
	0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 	// 062 - '>'		
	0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 	// 063 - '?'		
	0x32, 0x49, 0x79, 0x41, 0x3E, 0x00, 	// 064 - '@'		
	0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00, 	// 065 - 'A'		
	0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 	// 066 - 'B'		 
	0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 	// 067 - 'C'		 
	0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 	// 068 - 'D'		 
	0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 	// 069 - 'E'		 
	0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, 	// 070 - 'F'		 
	0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00, 	// 071 - 'G'		 
	0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 	// 072 - 'H'		 
	0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 	// 073 - 'I'		 
	0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 	// 074 - 'J'		 
	0x7F, 0x08, 0x14, 0x26, 0x41, 0x00, 	// 075 - 'K'		 
	0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 	// 076 - 'L'		 
	0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00, 	// 077 - 'M'		 
	0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 	// 078 - 'N'		 
	0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 	// 079 - 'O'		 
	0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 	// 080 - 'P'		 
	0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 	// 081 - 'Q'		 
	0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 	// 082 - 'R'		 
	0x26, 0x49, 0x49, 0x49, 0x32, 0x00, 	// 083 - 'S'		 
	0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 	// 084 - 'T'		 
	0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 	// 085 - 'U'		 
	0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 	// 086 - 'V'		 
	0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, 	// 087 - 'W'		 
	0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 	// 088 - 'X'		 
	0x07, 0x08, 0x70, 0x08, 0x07, 0x00, 	// 089 - 'Y'		 
	0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 	// 090 - 'Z'		 
	0x00, 0x7F, 0x41, 0x41, 0x00, 0x00, 	// 091 - '['		 
	0x01, 0x02, 0x1C, 0x20, 0x40, 0x00, 	// 092 - '\'		 
	0x00, 0x41, 0x41, 0x7F, 0x00, 0x00, 	// 093 - ']'		 
	0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 	// 094 - '^'		 
	0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 	// 095 - '_'		 
	0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 	// 096 - '`'		 
	0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 	// 097 - 'a'		 
	0x7F, 0x48, 0x44, 0x44, 0x38, 0x00, 	// 098 - 'b'		 
	0x38, 0x44, 0x44, 0x44, 0x20, 0x00, 	// 099 - 'c'		 
	0x38, 0x44, 0x44, 0x48, 0x7F, 0x00, 	// 100 - 'd'		 
	0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 	// 101 - 'e'		 
	0x08, 0x7E, 0x09, 0x01, 0x02, 0x00, 	// 102 - 'f'		 
	0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00, 	// 103 - 'g'		 
	0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 	// 104 - 'h'		 
	0x00, 0x44, 0x7D, 0x40, 0x00, 0x00, 	// 105 - 'i'		 
	0x20, 0x40, 0x44, 0x3D, 0x00, 0x00, 	// 106 - 'j'		 
	0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 	// 107 - 'k'		 
	0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 	// 108 - 'l'		 
	0x7C, 0x04, 0x18, 0x04, 0x78, 0x00, 	// 109 - 'm'		 
	0x7C, 0x08, 0x04, 0x04, 0x78, 0x00, 	// 110 - 'n'		 
	0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 	// 111 - 'o'		 
	0x7C, 0x14, 0x14, 0x14, 0x08, 0x00, 	// 112 - 'p'		 
	0x08, 0x14, 0x14, 0x18, 0x7C, 0x00, 	// 113 - 'q'		 
	0x7C, 0x08, 0x04, 0x04, 0x08, 0x00, 	// 114 - 'r'		 
	0x48, 0x54, 0x54, 0x54, 0x20, 0x00, 	// 115 - 's'		 
	0x04, 0x3F, 0x44, 0x40, 0x20, 0x00, 	// 116 - 't'		 
	0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00, 	// 117 - 'u'		
	0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, 	// 118 - 'v'		
	0x3C, 0x40, 0x20, 0x40, 0x3C, 0x00, 	// 119 - 'w'		
	0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 	// 120 - 'x'		
	0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00, 	// 121 - 'y'		
	0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 	// 122 - 'z'			
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const uint8_t font_16x24[] = {

    16, 	// font width
	24, 	// font height
	'0', 	// first char
	'9',	// last char

    0x00, 0xE0, 0xF8, 0xFC, 0x3C, 0x1E, 0x0E, 0x0E, 0x0E, 0x1E, 0x1C, 0xFC, 0xF8, 0xF0, 0x80, 0x00, // 48 - '0'
	0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
	0x03, 0x1F, 0x3F, 0x7E, 0x70, 0xF0, 0xE0, 0xE0, 0xE0, 0xF0, 0x70, 0x7E, 0x3F, 0x0F, 0x01, 0x00, 
    0x00, 0x00, 0x00, 0xE0, 0x70, 0x70, 0x38, 0x3C, 0xFC, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, // 049 - '1'
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1C, 0x1C, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x1E, 0x3E, 0xFC, 0xF8, 0xF0, 0x00, 0x00, // 050 - '2'
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0x78, 0x3C, 0x1F, 0x0F, 0x03, 0x00, 0x00,
	0x00, 0xF0, 0xFC, 0xFE, 0xE7, 0xE3, 0xE1, 0xE1, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00,
	0x00, 0x00, 0x38, 0x1C, 0x0C, 0x0E, 0x0E, 0x0E, 0x0E, 0x1E, 0x1C, 0xFC, 0xF8, 0xF0, 0x00, 0x00, // 051 - '3'
	0x00, 0x00, 0x00, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x7C, 0x7E, 0xEF, 0xE7, 0xC1, 0x80, 0x00,
	0x00, 0x38, 0x70, 0x70, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x70, 0x78, 0x3F, 0x1F, 0x0F, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0x3C, 0xFE, 0xFE, 0xFE, 0x00, 0x00, 0x00, // 052 - '4'
	0x80, 0xC0, 0xF0, 0x78, 0x3C, 0x0F, 0x07, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF, 0x07, 0x07, 0x07,
	0x00, 0x00, 0x00, 0xF0, 0xFE, 0xFE, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00, // 053 - '5'
	0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1C, 0x1C, 0x1C, 0x3C, 0x7C, 0xF8, 0xF0, 0xE0, 0x00, 0x00,
	0x00, 0x00, 0x70, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0x78, 0x3F, 0x1F, 0x07, 0x00, 0x00,
	0x00, 0x00, 0xC0, 0xF0, 0xF8, 0xFC, 0x3C, 0x1E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x1E, 0x00, 0x00, // 054 - '6'
	0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0x38, 0x1C, 0x0E, 0x0E, 0x0E, 0x1E, 0x3E, 0xFC, 0xF8, 0xE0, 0x00,
	0x00, 0x03, 0x1F, 0x3F, 0x7F, 0xF8, 0xF0, 0xE0, 0xE0, 0xE0, 0xF0, 0x78, 0x3F, 0x1F, 0x07, 0x00,
	0x00, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x8E, 0xEE, 0xFE, 0x3E, 0x0E, 0x00, // 055 - '7'
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0x7E, 0x1F, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xE0, 0xFC, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xF0, 0xF8, 0xFC, 0x3E, 0x0E, 0x0E, 0x0E, 0x0E, 0x3E, 0xFC, 0xF8, 0xF0, 0x00, 0x00, // 056 - '8'
	0x00, 0x80, 0xC3, 0xE7, 0xEF, 0x7C, 0x38, 0x38, 0x38, 0x38, 0x7C, 0xEF, 0xE7, 0xC3, 0x80, 0x00,
	0x00, 0x0F, 0x3F, 0x7F, 0x78, 0xF0, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0x78, 0x7F, 0x3F, 0x0F, 0x00,
	0xC0, 0xF0, 0xF8, 0x3C, 0x1C, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x1E, 0x7C, 0xFC, 0xF8, 0xE0, 0x80, // 057 - '9'
	0x0F, 0x1F, 0x3F, 0x78, 0xF0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x70, 0x38, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x70, 0x70, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0x70, 0x78, 0x3E, 0x3F, 0x0F, 0x07, 0x00,
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------------