#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

constexpr double TOL = 1e-6;

bool close( double a, double b, double tol=TOL) {
  /*
  double aa = std::abs(a);
  double bb = std::abs(b);
  double max = (aa > bb ) ? aa : bb;
  if(max < tol)
    return true;
  if(std::abs(a -b)/max <= tol)
  */
  if(std::abs(a -b) <= tol)    
    return true;
  return false;
}

// printf("ptclHistory %d zInd -1 detId -1 pos %g %g %g iter %d "vel %g %g %g\n"
// ,id, pos[0], pos[1], pos[2], iter, vel[0], vel[1], vel[2]);
bool readIn(const std::string& fname, std::vector<double>& x, 
  std::vector<double>& y, std::vector<double>& z,
  std::vector<double>& vx, std::vector<double>& vy,
  std::vector<double>& vz, std::vector<int>& upd, std::vector<int>& inds, 
  std::vector<int>& id1, std::vector<int>& id2, bool debug = false) {
  std::ifstream fs(fname);
  if(!fs.good()) { 
    printf("Error reading file\n");
    return false;
  }
  int max = *std::max_element(inds.begin(), inds.end());
  int xcol = inds[0];
  int ycol = inds[1];
  int zcol = inds[2];
  int vxcol = inds[3];
  int vycol = inds[4];
  int vzcol = inds[5];
  int updcol = inds[6]; 
  int id1col1 = -1, id2col = -1;
  id1col1 = inds[7];
  id2col = inds[8];
  std::string line, str;
  while(std::getline(fs, line)) {
    std::stringstream ss(line);
    ss >> str;
    if(str.find_first_not_of(' ') == std::string::npos)
      continue;

    //input first column=1, not 0
    for(int i=1; i<=max; ++i) {
      if(debug)
        std::cout << "i " << i << " " << str << "\n";
      // when i=0, use str from above
      try {
        if(id1col1==i)
          id1.push_back(std::stoi(str));
        else if(id2col==i)
  	      id2.push_back(std::stoi(str));
        else if(updcol ==i)
          upd.push_back(std::stoi(str));
        else if(xcol == i)
          x.push_back(std::stod(str));
        else if(ycol == i)
          y.push_back(std::stod(str));
        else if(zcol == i)
          z.push_back(std::stod(str));
        else if(vxcol == i)
          vx.push_back(std::stod(str));
        else if(vycol == i)
          vy.push_back(std::stod(str));
        else if(vzcol == i)
          vz.push_back(std::stod(str));
      } catch (const std::invalid_argument& ia) {
        std::cout << " invalid " << i << " " << str << "\n";
        std::abort();
      }
      ss >> str;
    }
  }
  return true;
}


void findStoppedTimeStepsOfPtcls(std::vector<double>& x,  std::vector<double>& y,
  std::vector<double>& z, std::vector<double>& vx,  std::vector<double>& vy,
  std::vector<double>& vz, int& ptclStopIndex) {  

  double xx=0, yy=0, zz=0, vxx=0, vyy=0, vzz=0;
  for(int i=0; i<x.size(); ++i) {
    if(close(x[i], xx) && close(y[i], yy) && close(z[i], zz)
      && close(vx[i],vxx) && close(vy[i], vyy) && close(vz[i],vzz)) {
      ptclStopIndex = i-1;
      break;
    }
    xx = x[i];
    yy = y[i];
    zz = z[i];
    vxx = vx[i];
    vyy = vy[i];
    vzz = vz[i];
  } // for ip
}


// NOTE: multiple particles not distinguished, and not handled in stop index calculation
// It is safe to grep single ptcls and use this script
//first column = 1, consistent with awk
int main(int argc, char* argv[]) {
  if(argc < 3) {
    //file1: ptclHistory_all 1 iter 1 pos 0.11 0.12 0.13  vel 22 33 44 updateiter 1
    //file2:  gitr 1 tstep 0 pos -0.02 0.003 1e-06 vel 22 33 44 stopped 3333
    //./compare file1 file2 4 6  10 10  14  1 1  2 8  2 4
    std::cout << argv[0] << " <file1> <file2> [ <pidmatch(0)> [<tstepmatch(0)> <debug(0)>]]\n";
    exit(1);
  }
  std::string file1 = argv[1];
  std::string file2 = argv[2];

  int x1col = 6, vx1col = 10, updcol1 = 14, pid1col = 2, t1col = 4;
  int x2col = 6, vx2col = 10, updcol2 = 14, pid2col = 2, t2col = 4;

  int id1Match = 0, id2Match = 0;
  if(argc > 3)
    id1Match = std::atoi(argv[3]);
  if(argc > 4)
    id2Match = std::atoi(argv[4]);
  int debug = 0;
  if(argc > 5)
    debug = std::atoi(argv[5]);

  std::vector<int> inds1, inds2;
  for(int i=0; i<3; ++i) {
    inds1.push_back(x1col+i);
    inds2.push_back(x2col+i);    
  }
  for(int i=0; i<3; ++i) {
    inds1.push_back(vx1col+i);
    inds2.push_back(vx2col+i);    
  }
  inds1.push_back(updcol1);
  inds1.push_back(pid1col);
  inds1.push_back(t1col);
  inds2.push_back(updcol2);  
  inds2.push_back(pid2col);
  inds2.push_back(t2col);
  
  if(debug) {
    std::cout << file1 << " " << file2 << "\n";
    std::cout << "File_columns: x,y,z, vx, vy, vz, upd, id1, id2 \n"; 
    for(auto v: inds1)
      std::cout << v << " ";
    std::cout << "\n";
    for(auto v: inds2)
      std::cout << v << " ";
    std::cout << "\n";
  }
  std::vector<double> x1, y1, z1, x2, y2, z2;
  std::vector<double> vx1, vy1, vz1, vx2, vy2, vz2;

  std::vector<int> pid1, tstep1, pid2, tstep2, upd1, upd2;
  if(debug)
    std::cout << "Reading file "<< file1 << "\n";
  bool ret = readIn(file1, x1, y1, z1, vx1, vy1, vz1, upd1, inds1, 
    pid1, tstep1);
  if(!ret) {
    std::cout << "Error reading file " << file1 << "\n";
    return 1;
  }
  if(debug)
    std::cout << "Reading file " << file2 << "\n";
  ret = readIn(file2, x2, y2, z2, vx2, vy2, vz2, upd2, inds2, pid2, 
    tstep2); 
  if(!ret) {
    std::cout << "Error reading file " << file2 << "\n";
    return 1;
  }
  int size1 = x1.size();
  int size2 = x2.size();
  std::vector<int> matches(size1, 0);
  if(debug) {
    printf("Num : file1 %d file2 %d \n", size1, size2);
    printf("First a: pid %d %g %g %g b: pid %d %g %g %g \n", pid1[0], x1[0], y1[0], 
      pid2[0], z1[0], x2[0], y2[0], z2[0]);
  }

  int ptclStopIndex1 = x1.size();
  int ptclStopIndex2 = x2.size();
  // NOTE: valid for input file containing single ptcl
  findStoppedTimeStepsOfPtcls(x1, y1, z1, vx1, vy1, vz1, ptclStopIndex1);
  findStoppedTimeStepsOfPtcls(x2, y2, z2, vx2, vy2, vz2, ptclStopIndex2);

  if(debug)
    std::cout << " stopped " << ptclStopIndex1 << " , " << ptclStopIndex2 << "\n";

  int id1matches = 0, id2matches = 0;

  for(int i=0; i<size1; ++i) { // mixed ptcl ids and time steps
    if(i > ptclStopIndex1) break;
    
    for(int j=0; j<size2; ++j) {
      if(j > ptclStopIndex2) break;
    
      if(close(x1[i], x2[j]) && close(y1[i], y2[j]) && close(z1[i], z2[j]) &&
        close(vx1[i], vx2[j]) && close(vy1[i], vy2[j]) && close(vz1[i], vz2[j])) {
        bool pidmatch = (!id1Match || pid1[i]==pid2[j]) ? 1 :0;
        bool tstepmatch = (!id2Match || tstep1[i]==tstep2[j]) ? 1 :0;
  	    if(pidmatch) ++id1matches;
        if(tstepmatch) ++id2matches;
        if(pidmatch && tstepmatch) {
          matches[i] = 1;
  	      //printf("Match a: %g %g %g b: %g %g %g \n", x1[i], y1[i], z1[i], x2[j], y2[j], z2[j]);
          break;
        }
      }
    }
  }
  
  int noMatch = 0, match = 0;
  for(int i=0; i<size1; ++i)
    if(!matches[i]) {
      noMatch++;
      // printf("nomatch %d %g %g %g %g %g %g upd %d file %s \n", i, x1[i],
      // y1[i], z1[i], vx1[i], vy1[i], vz1[i], upd1[i], file1.c_str());
    }
    else 
      match++;    
  // int numTsteps = 
  //int numPtcls = 
  printf("id %d no_match: %d  match: %d id1Matches: %d id2Matches: %d "
         ": total:ref-file1 %d file2 %d\n",
     pid1[0], noMatch, match, id1matches, id2matches, size1, size2);
  return 0;
}
