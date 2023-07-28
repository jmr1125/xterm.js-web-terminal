#include <string>
#include <vector>
using std::string;
using std::vector;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
WORD to_big(WORD x);
DWORD to_big(DWORD x);
QWORD to_big(QWORD x);
/* const inline DWORD A = to_big((DWORD)0x67452301); */
/* const inline DWORD B = to_big((DWORD)0xEFCDAB89); */
/* const inline DWORD C = to_big((DWORD)0x98BADCFE); */
/* const inline DWORD D = to_big((DWORD)0x10325476); */
/* const inline DWORD E = to_big((DWORD)0xc3d2e1f0); */
const inline DWORD A = (DWORD)0x67452301;
const inline DWORD B = (DWORD)0xEFCDAB89;
const inline DWORD C = (DWORD)0x98BADCFE;
const inline DWORD D = (DWORD)0x10325476;
const inline DWORD E = (DWORD)0xc3d2e1f0;
const inline vector<DWORD> Initabcde{A, B, C, D, E};
vector<bool> pad(vector<bool>);
vector<vector<bool>> blocks(vector<bool>);
vector<DWORD> blocksGetw(vector<bool>);
vector<DWORD> loop80(vector<DWORD>, vector<DWORD>);
vector<bool> to_vector(string);
vector<bool> to_vector(DWORD);
