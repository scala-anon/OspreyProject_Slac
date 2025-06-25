#include "HDF5_Parser.hpp"
#include <H5Cpp.h>
#include <iostream>
#include <string>

void openHDF5File(const std::string &filename) {
  try {
    H5::H5File file(filename, H5F_ACC_RDONLY);
    std::cout << "Opened file: " << filename << std::endl;

    hsize_t num_objs = file.getNumObjs();
    for (hsize_t i = 0; i < num_objs; i++) {
      std::string obj_name = file.getObjnameByIdx(i);
      std::cout << "Found object: " << obj_name << std::endl;
    }
  } catch (const H5::FileIException &e) {
    std::cerr << "HDF5 FIle Exception: " << e.getDetailMsg() << std::endl;
    throw;
  } catch (const H5::Exception &e) {
    std::cerr << "HDF5 Exception: " << e.getDetailMsg() << std::endl;
  }
}
