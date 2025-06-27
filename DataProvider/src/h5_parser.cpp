#include "h5_parser.hpp"
#include <H5Cpp.h>
#include <iostream>

HDF5Parser::HDF5Parser(const std::string& filepath)
    : filename(filepath) {}

void HDF5Parser::parse() {
    std::cout << "[DEBUG] Entered parse()" << std::endl;
    try {
        H5::H5File file(filename, H5F_ACC_RDONLY);
        std::cout << "Opened file: " << filename << std::endl;

        hsize_t num_objs = file.getNumObjs();
        for (hsize_t i = 0; i < num_objs; i++) {
            std::string obj_name = file.getObjnameByIdx(i);
            std::cout << "Found object: " << obj_name << std::endl;
        }
    } catch (const H5::FileIException& e) {
        std::cerr << "HDF5 File Exception: " << e.getDetailMsg() << std::endl;
        throw;
    } catch (const H5::Exception& e) {
        std::cerr << "HDF5 Exception: " << e.getDetailMsg() << std::endl;
    }
}

