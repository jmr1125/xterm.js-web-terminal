#include<stdint.h>
#include<string>
#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>

#ifndef SHA1_H
#define SHA1_H

class SHA1
{
public:
    SHA1();
    void update(const std::string &s);
    void update(std::istream &is);
    std::string final();
    static std::string from_file(const std::string &filename);
    uint32_t digest[5];

private:
    std::string buffer;
    uint64_t transforms;
};


static const size_t BLOCK_INTS = 16;  /* number of 32bit integers per SHA1 block */
static const size_t BLOCK_BYTES = BLOCK_INTS * 4;


#endif /* SHA1_H */