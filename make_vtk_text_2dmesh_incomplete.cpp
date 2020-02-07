#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cassert>

// g++ --std=c++11 <thisfile>.cpp -o makevtk_text


int main(int argc, char* argv[]) {

  if(argc < 2) {
    // ceate legacy vtk from text file having rows of coordinates.
    // Example: 2d mesh text input: x1 y1 z1 x2 y2 z2 x3 y3 z3
    // Current version supports only 2D mesh input text
    std::cout << argv[0] << " <textfile> [<x1_column_id=0>]\n"
              << "Example: \n ./makevtk_text <textfile> 5\n";
    exit(1);
  }
  std::string infileName = argv[1];
  int x1Col = 0;
  int dim = 2;
  if(argc > 2)
    x1Col = atoi(argv[2]);
  
  std::vector<double> xdata, ydata, zdata;
  long int ncells = 0;
  long int size = 0;
  std::ifstream inf(infileName);


  //write vtk file
  auto points = size; //4th to close-out
  std::vector<unsigned long int>cellLinks(4*ncells);
  std::vector<int>cellCodes(ncells, 3);
  long int cumul = 0;
  for(auto i=0; i<ncells; ++i) {
    cellLinks[i*4] = 3;
    cellLinks[i*4+1] = cumul;
    cellLinks[i*4+2] = cumul+1;
    cellLinks[i*4+3] = cumul+2;
    cumul += 3;
  }
  
  std::string outFileName = ncFileName.substr(0, ncFileName.find_last_of(".")) + ".vtk";
  outFileName = outFileName.substr(ncFileName.find_last_of("/")+1);
  std::cout << "Writing VTK file ./" << outFileName << "\n";
  std::ofstream outf;
  outf.open(outFileName);

  outf << "# vtk DataFile Version 2.0\n" 
       << "Mesh 2D\n";
  outf << "ASCII\n"; 
  outf << "DATASET UNSTRUCTURED_GRID\n"
       << "POINTS " << points << " double\n";
  std::cout << "Writing x,y,z \n";

  for(auto i = 0; i < size; ++i)
    outf << xdata[i] << " " << ydata[i] << " " << zdata[i] << "\n";

  
  std::cout << "Writing cell links \n";
  outf << "CELLS " << ncells << " " << 4*ncells << "\n";
  for(auto i=0; i< ncells; ++i)
    outf << cellLinks[4*i] << " " <<  cellLinks[4*i+1] << " " 
         << cellLinks[4*i+2] << " " <<  cellLinks[4*i+3] << "\n";
  
  std::cout << "Writing cell-code 5 \n";
  outf << "CELL_TYPES " << ncells << "\n";
  for(auto i=0; i< ncells; ++i)
    outf << "5\n";

  outf.close();
  std::cout << "Done !\n";  
  return 0;
}
