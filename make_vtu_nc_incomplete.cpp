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

// module load gcc mpich netcdf;
// export LD_LIBRARY_PATH=/lore/gopan/install/build-netcdfcxx431/install/lib64:$LD_LIBRARY_PATH
// g++ --std=c++11 <thisfile>.cpp -o makevtk -I/lore/gopan/install/build-netcdfcxx431/install/include 
// -lnetcdf -L/lore/gopan/install/build-netcdfcxx431/install/lib64/ -lnetcdf-cxx4

//nP name = dimNames[0]; nT name = dimNames[1]
int readNetcdfData(const std::string& ncFileName,
   std::vector<std::string> dimNames, std::vector<std::string> varNames,
   std::vector<double>& xd, std::vector<double>& yd, std::vector<double>& zd,
   int& numPtcls, int& nTimeSteps) {

  int ncSizePerComp = 1;
  int nComp = varNames.size();
  // 1st is nP; 2nd is nT
  std::vector<int> dims;
  try {
    netCDF::NcFile ncf(ncFileName, netCDF::NcFile::read);
    for(int i=0; i< dimNames.size(); ++i) {
      netCDF::NcDim ncDimName(ncf.getDim(dimNames[i]));
      auto size = ncDimName.getSize();
      dims.push_back(size);
      if(i==0) { //numPtcls
      //  if(size < numPtcls)
         numPtcls = size;
      }
      if(i==1) { // nTimeSteps
        nTimeSteps = size;
      }
      std::cout << dimNames[i] << " : " << dims[i] << "\n";
      ncSizePerComp *= dims[i];
    }
    std::cout << "ncSizePerComp: " << ncSizePerComp << "; nComp " << nComp << "\n";

    xd.resize(ncSizePerComp); 
    yd.resize(ncSizePerComp); 
    zd.resize(ncSizePerComp); 
    // TODO use maxNPtcls and numPtclsRead
    netCDF::NcVar ncvar1(ncf.getVar(varNames[0]));
    ncvar1.getVar(&(xd[0]));
    netCDF::NcVar ncvar2(ncf.getVar(varNames[1]));
    ncvar2.getVar(&(yd[0]));
    netCDF::NcVar ncvar3(ncf.getVar(varNames[2]));
    ncvar3.getVar(&(zd[0]));
    std::cout << "Done reading " << ncFileName << "\n";
  } catch (netCDF::exceptions::NcException &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  // NaNs
  int nans = 0;
  for(int i=0; i<xd.size(); ++i)
    if(std::isnan(xd[i]) || std::isnan(yd[i]) || std::isnan(zd[i])) {
      ++nans;
    }
  if(nans)
    std::cout << "\n*******WARNING found "<< nans << " NaNs in data ******\n\n";
  assert(nans == 0);

  return 0;
}

// https://stackoverflow.com/questions/10913666/error-writing-binary-vtk-files
template <typename T>
void swapEnd(T& var) {
  char* varArray = reinterpret_cast<char*>(&var);
  for(long i = 0; i < static_cast<long>(sizeof(var)/2); i++)
    std::swap(varArray[sizeof(var) - 1 - i],varArray[i]);
}
// https://stackoverflow.com/questions/585257/\
// is-there-a-better-way-to-reverse-an-array-of-bytes-in-memory
void reverseBytes( void *start, int size) {
  char *istart = reinterpret_cast<char*>(start);
  char *iend = reinterpret_cast<char*>(istart + size);
  std::reverse(istart, iend);
}

int main(int argc, char* argv[]) {

  if(argc < 2) {
    // ./make_vtk <ncfile> 1 5 # for 10 ptcls starting 5th
    std::cout << argv[0] << " <NCfile> [ <numPtcls> <pstart#> <numTimesteps> <tstart#>" 
              << " <nP-name> <nT-name> [<x-name> <y-name> <z-name>]]\n"
              << "Example: \n"
              << "./make_vtk <ncfile> [10 0 1000 0 \"nP\" \"nT\" \"x\" \"y\" \"z\"] \n";
    exit(1);
  }
  bool write_binary = false;  
  int debug = 1;
  std::string ncFileName = argv[1];
  int np = 0, nt = 0, pstart = 0, tstart = 0;
  if(argc > 2)
    np = atoi(argv[2]);
  if(argc > 3)
    pstart = atoi(argv[3]);
  if(argc > 4)
    nt = atoi(argv[4]);
  if(argc > 5)
    tstart = atoi(argv[5]);

  std::string npName = "nP";
  std::string ntName = "nT";
  std::string xName = "x";
  std::string yName = "y";
  std::string zName = "z";

  if(argc > 6) 
    npName = argv[6];
  if(argc > 7)
    ntName = argv[7];
  if(argc > 10) {
    xName = argv[8];
    yName = argv[9];
    zName = argv[10];
  }

  std::vector<std::string> dimNames, varNames;
  dimNames.push_back(npName);
  dimNames.push_back(ntName);
  varNames.push_back(xName);
  varNames.push_back(yName);
  varNames.push_back(zName);
  std::vector<double> xdata, ydata, zdata;

  // Now reading all data. When NC can read subset, replace npFile & ntFile by np & nt
  int npFile = 0, ntFile = 0;
  readNetcdfData(ncFileName, dimNames, varNames, xdata,ydata, zdata, npFile, ntFile);

  if(np < 1 || np > npFile)
    np = npFile;
  if(nt < 1 || nt > ntFile)
    nt = ntFile;
  std::cout << "NcFile: nP " << npFile << " ; nT " << ntFile << "\n";
  std::cout << "MakeVtk: nP " << np << " ; nT " << nt << "\n";  
  //write vtk file for all particles
  int points = np*nt;
  
  int dataSize = 1;
  if(write_binary)
    dataSize = 3*points;

  std::vector<double> data(dataSize);
  int numCells = np*(nt-1);
  std::vector<unsigned long int>cellLinks(3*numCells);
  std::vector<int>cellCodes(numCells, 3);
  unsigned long int cumul = 0, cellN = 0;
  for(int i=0, pf=pstart; i<np && pf<npFile; ++i, ++pf) {
    for(int j=0, tf=tstart; j<nt && tf<ntFile; ++j, ++tf){
      if(write_binary) {
        data[(i*nt+j)*3] = xdata[pf*ntFile+tf];
        data[(i*nt+j)*3+1] = ydata[pf*ntFile+tf];
        data[(i*nt+j)*3+2] = zdata[pf*ntFile+tf];
      }
      if(j < nt-1) {
        cellLinks[3*cellN] = 2;
        cellLinks[3*cellN+1] = cumul;
        cellLinks[3*cellN+2] = cumul+1;
        ++cellN;
      }
      ++cumul;
    }
  }
  
  std::string outFileName = ncFileName.substr(0, ncFileName.find_last_of(".")) + ".vtk";
  outFileName = outFileName.substr(ncFileName.find_last_of("/")+1);
  std::cout << "Writing VTK file ./" << outFileName << "\n";
  std::ofstream outf;
  if(write_binary)
    outf.open(outFileName, std::ios::out | std::ios::binary);
  else
    outf.open(outFileName);
  int cell_dim = 2;
  int nverts = 0;
  int nents = 0;

  static std::uint16_t const endian = 0x1;
  std::uint8_t const* ptr = reinterpret_cast<std::uint8_t const*>(&endian);
  bool little_endian = (*ptr == 0x1);

  outf << "<VTKFile type=\"UnstructuredGrid\" byte_order=\"";
  if(little_endian)
    outf << "LittleEndian";
  else
    outf << "BigEndian";
  outf << "\" header_type=\"";
  outf << Traits<std::uint64_t>::name();
  outf << "\">\n";
  outf << "<UnstructuredGrid>\n";
  outf << "<Piece NumberOfPoints=\"" << nverts << "\"";
  outf << " NumberOfCells=\"" << nents << "\">\n";
  outf << "<Cells>\n";


  write_connectivity(outf, mesh, cell_dim, compress);


  outf << "</Cells>\n";
  outf << "<Points>\n";
  auto coords = mesh->coords();
  write_array(outf, "coordinates", 3, resize_vectors(coords, mesh->dim(), 3),
      compress);
  outf << "</Points>\n";
  outf << "<PointData>\n";
  /* globals go first so read_vtu() knows where to find them */
  if (mesh->has_tag(VERT, "global") && tags[VERT].count("global")) {
    write_tag(outf, mesh->get_tag<GO>(VERT, "global"), mesh->dim(), compress);
  }
  write_locals_and_owners(outf, mesh, VERT, tags, compress);
  for (Int i = 0; i < mesh->ntags(VERT); ++i) {
    auto tag = mesh->get_tag(VERT, i);
    if (tag->name() != "coordinates" && tag->name() != "global" &&
        tags[VERT].count(tag->name())) {
      write_tag(outf, tag, mesh->dim(), compress);
    }
  }
  outf << "</PointData>\n";
  outf << "<CellData>\n";
  /* globals go first so read_vtu() knows where to find them */
  if (mesh->has_tag(cell_dim, "global") &&
      tags[size_t(cell_dim)].count("global")) {
    write_tag(
        outf, mesh->get_tag<GO>(cell_dim, "global"), mesh->dim(), compress);
  }
  write_locals_and_owners(outf, mesh, cell_dim, tags, compress);
  for (Int i = 0; i < mesh->ntags(cell_dim); ++i) {
    auto tag = mesh->get_tag(cell_dim, i);
    if (tag->name() != "global" && tags[size_t(cell_dim)].count(tag->name())) {
      write_tag(outf, tag, mesh->dim(), compress);
    }
  }
  outf << "</CellData>\n";
  outf << "</Piece>\n";
  outf << "</UnstructuredGrid>\n";
  outf << "</VTKFile>\n";





  outf << "# vtk DataFile Version 2.0\n" 
       << "particle paths\n";
  if(write_binary)  
     outf << "BINARY\n";
  else
     outf << "ASCII\n"; 
  outf << "DATASET UNSTRUCTURED_GRID\n"
       << "POINTS " << points << " double\n";
  std::cout << "Writing x,y,z \n";
  if(write_binary) {
    //TODO try swapping individual elements
    //https://stackoverflow.com/questions/55829282/write-vtk-file-in-binary-format
    reverseBytes(&data[0], 3*points*sizeof(double)); 
    reverseBytes(&cellLinks[0], 3*numCells*sizeof(unsigned long int)); 
    reverseBytes(&cellCodes[0], numCells*sizeof(int)); 
    outf.write((char*)&data[0], 3*points);
  } else {
    for(int i = pstart; i < pstart+np; ++i)
      for(int j = tstart; j < tstart+nt; ++j)
        outf << xdata[i*ntFile+j] << " " << ydata[i*ntFile+j] << " " << zdata[i*ntFile+j] << "\n";
  } 
  
  std::cout << "Writing cell links \n";
  outf << "CELLS " << numCells << " " << 3*numCells << "\n";
  if(write_binary)
    outf.write((char*)&cellLinks[0], 3*numCells);
  else
    for(int i=0; i< numCells; ++i)
      outf << cellLinks[3*i] << " " <<  cellLinks[3*i+1] << " " <<  cellLinks[3*i+2] << "\n";
  
  std::cout << "Writing cell-code 3 \n";
  outf << "CELL_TYPES " << numCells << "\n";
  if(write_binary)
    outf.write((char*)&cellCodes[0], numCells);
  else
    for(int i=0; i< numCells; ++i)
      outf << "3\n";

  outf.close();
  std::cout << "Done !\n";  
  return 0;
}
