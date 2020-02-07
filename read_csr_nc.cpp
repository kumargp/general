#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <netcdf>

// module load gcc mpich netcdf; export LD_LIBRARY_PATH=/lore/gopan/install/build-netcdfcxx431/install/lib64:$LD_LIBRARY_PATH
// g++ --std=c++11 main.cpp -o exename -I/lore/gopan/install/build-netcdfcxx431/install/include 
// -lnetcdf -L/lore/gopan/install/build-netcdfcxx431/install/lib64/ -lnetcdf-cxx4   # -g

// NOTE the order of names and variables
int readNetcdfCsr(const std::string& ncFileName,
   std::vector<std::string> varNumNames, std::vector<std::string> varNames,
   std::vector<int>& ptrs, std::vector<int>& data, std::vector<int>&fel,  int& nPtrs, 
   int& nData, int& nCells) {
   int nFel = 0;
  try {
    netCDF::NcFile ncf(ncFileName, netCDF::NcFile::read);
    for(int i=0; i< varNumNames.size(); ++i) {
      netCDF::NcDim ncDimName(ncf.getDim(varNumNames[i]));
      auto size = ncDimName.getSize();
      // NOte order
      if(i==0) {
        nCells = size;
        std::cout << varNumNames[i] << " : " << nCells << "\n";
      }
      if(i==1) {
        nPtrs = size;
        std::cout << varNumNames[i] << " : " << nPtrs << "\n";
      }
      if(i==2) {
        nData = size;
        std::cout << varNumNames[i] << " : " << nData << "\n";
      }
      if(i==3) {
        nFel = size;
        std::cout << varNumNames[i] << " : " << nFel << "\n";
      }
    }
    data.resize(nData); 
    ptrs.resize(nPtrs);
    fel.resize(nFel);
    netCDF::NcVar ncvar1(ncf.getVar(varNames[0]));//ptr
    ncvar1.getVar(&(ptrs[0]));
    netCDF::NcVar ncvar2(ncf.getVar(varNames[1]));//data
    ncvar2.getVar(&(data[0]));
    netCDF::NcVar ncvar3(ncf.getVar(varNames[1]));//fel
    ncvar2.getVar(&(fel[0]));
    std::cout << "Done reading " << ncFileName << "\n";
  } catch (netCDF::exceptions::NcException &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  // NaNs
  int nans = 0;
  for(int i=0; i<data.size(); ++i)
    if(std::isnan(data[i])) {
      ++nans;
    }
  if(nans)
    std::cout << "\n*******WARNING found "<< nans << " NaNs in data ******\n\n";
  assert(nans == 0);

  return 0;
}

int main(int argc, char* argv[]) {

  if(argc < 2) {
    std::cout << argv[0] << " <NCfile> [<cell_ids> <num_csr_entries> <entry_start#>\n"
              << "Example: type - for skipping entry. If cell_ids "
              "(seperated by comma) given, the rest not taken\n"
              << "./read_csr_nc <ncfile> 43645,67876\n";
    exit(1);
  }
  int debug = 1;
  std::string ncFileName = argv[1];
  int ne = 0, start = 0;
  std::vector <int>ids;
  if(argc > 2 && argv[2] !="-") {
    std::string idnames = argv[2];
    std::istringstream iss(idnames);
    std::string word;
    while (std::getline(iss, word, ','))
      ids.push_back(stoi(word));
  }
  if(argc > 3 && argv[3] !="-")
    ne = atoi(argv[3]);
  if(argc > 4 && argv[4] !="-")
    start = atoi(argv[4]);

  std::string neName = "nelems";
  std::string nPtrName = "nindices";
  std::string nDataName = "nfaces";
  std::string nfeName = "nfaces";
  std::string ptrName = "indices";
  std::string dataName = "bdryfaces";
  std::string felemName = "face_elements";
  std::vector<std::string> nVarNames, varNames;
  nVarNames.push_back(neName);
  nVarNames.push_back(nPtrName);
  nVarNames.push_back(nDataName);
  nVarNames.push_back(nfeName);
  varNames.push_back(ptrName);
  varNames.push_back(dataName);
  varNames.push_back(felemName);
  std::vector<int> ptrs, data, felems;

  // Now reading all data. When NC can read subset, replace npFile & ntFile by np & nt
  int nPtrsFile = 0, nDataFile = 0, neFile = 0;
  readNetcdfCsr(ncFileName, nVarNames, varNames, ptrs, data, felems, 
      nPtrsFile, nDataFile, neFile);

  if(ne < 1 || ne > neFile)
    ne = neFile;
  std::cout << "NcFile: ne " << neFile << " nPtrs " << nPtrsFile 
    << " nData " << nDataFile << "\n";  
  assert(neFile == nPtrsFile-1);
  
  std::string outFileName = ncFileName.substr(0, ncFileName.find_last_of(".")) + "_selected";
  outFileName = outFileName.substr(ncFileName.find_last_of("/")+1);
  std::cout << "Writing selected values to file ./" << outFileName << "\n";
  std::ofstream outf(outFileName);
  std::ofstream oute(outFileName+"_elids");
  if(ids.size()) {
    for(int i=0; i<ids.size(); ++i) {
      auto el = ids[i];
      outf << "element " << el << " :\n";
      oute << "element " << el << " :\n";
      assert(el < ptrs.size()-1);
      for(int j = ptrs[el]; j < ptrs[el+1]; ++j) {
        outf << data[j] << ",";
        oute << felems[data[j]] << ","; 
      }
    } 
  } 
  else {
    for(int el=start; el < ne; ++el)
      for(int j = ptrs[el]; j < ptrs[el+1]; ++j) {
        outf << "element " << el << " :\n";
        oute << "element " << el << " :\n";
        outf << data[j] << ","; 
        oute << felems[data[j]] << ",";
      }
  }
  outf << "\n";
  outf.close();
  std::cout << "Done !\n";  
  return 0;
}
