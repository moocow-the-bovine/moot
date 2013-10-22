
// scanner return value semantics

#define TOKEN_UNKNOWN    -1
#define TOKEN_EOF        0
#define TOKEN_WORD       1
#define TOKEN_HYPH       2

#define TOKEN_SPACE      3
#define TOKEN_NL         4

#define TOKEN_GREEK      5

#define TOKEN_NUM        6
#define TOKEN_DOT        7
#define TOKEN_EOS        8
#define TOKEN_COMMA      9
#define TOKEN_QUOTE     10
#define TOKEN_SC        11
#define TOKEN_ROMAN_NUM 12

#define STOP_LOWER      13
#define STOP_UPPER      14
#define STOP_CAPS       15

#define LATIN_LOWER     16
#define LATIN_UPPER     17
#define LATIN_CAPS      18

#define TOKEN_MONEY     19
#define TOKEN_SB        20

#define LATIN_LOWER_TRUNC     21
#define LATIN_LOWER_TRUNC1    22
#define LATIN_LOWER_TRUNC2    23
#define TOKEN_TRUNC     24

#define TOKEN_PERCENT   25

#define TOKEN_XML       26

#define ABBREV          27
#define TOKEN_APOS      28
#define TOKEN_PLUS      29
#define TOKEN_LBR       30
#define TOKEN_RBR       31
#define TOKEN_SLASH     32
#define TOKEN_COLON     33
#define TOKEN_SEMICOLON 34

#define TOKEN_WB        35
#define TOKEN_REST     100
