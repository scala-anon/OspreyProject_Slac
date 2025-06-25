#ifndef HDF5_PARSER_HPP
#define HDF5_PARSER_HPP

#include <string>

class HDF5Parser {
public:
    explicit HDF5Parser(const std::string& filepath);
    void parse();

private:
    std::string filename;
};

#endif // HDF5_PARSER_HPP

