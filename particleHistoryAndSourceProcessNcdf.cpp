
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <algorithm>
#include <cassert>
#include <cstdlib>

constexpr double TOL = 1e-6;

bool same( double a, double b, double tol=TOL) {
  double aa = std::abs(a);
  double bb = std::abs(b);
  double max = (aa > bb ) ? aa : bb;
  if(max < tol)
    return true;
  if(std::abs(a -b)/max <= tol)
    return true;
  return false;
}

struct PtclStruct {
  PtclStruct(std::string n, std::string np, std::string nt, std::string x, 
  	std::string y, std::string z,std::string vx, std::string vy, 
  	std::string vz):
    name(n), nPname(np), nTname(nt), xName(x), yName(y), zName(z), 
    vxName(vx), vyName(vy), vzName(vz) {}
  std::string name;
  std::string nPname;// "nP"
  std::string nTname; //nT
  std::string xName;
  std::string yName;
  std::string zName;
  std::string vxName;
  std::string vyName;
  std::string vzName;
  int nComp = 6;  //pos, vel
  int nP = 0;
  int nT = 0; //sub cycling for history
};

//if nthStep >=0, then nT =1. If nthStpe<0, then all time steps
void parseFileFieldData(std::stringstream& ss, std::string sFirst, 
  std::string fieldName, bool semi, std::vector<double>& data, int& indP,
  int& indT, bool& dataLine, std::set<int>& nans, bool& expectEqual, 
  int iComp=0, int nComp=1, int numPtcls=0, int nT=1, bool debug=false) {
  std::string s2 = "", sd = "";
  // restart index when string matches
  if(sFirst == fieldName) {
    indP = 0;
    ss >> s2;
    dataLine = true;
    if(s2 != "=")
      expectEqual = true;
    // next character in the same line after fieldNme is '='' 
    if(debug) {
      std::cout << " dataLine: " << dataLine << " of " << fieldName << "\n";
      if(!s2.empty() && s2 != "=")
        std::cout << "WARNING: Unexpected entry: " << s2 << " discarded\n";
    }
  }
  if(debug && dataLine)
    std::cout << ":: "<< indP << " : " << ss.str() << " ::1st " << sFirst 
              << " field " << fieldName << "\n";
  
  if(dataLine) {
    // this is done for every line, not only that of fieldName string
    if(!(sFirst.empty() || sFirst == fieldName)) {
	    if(indP < numPtcls || !numPtcls) {
	      data[nComp*nT*indP + nComp*indT + iComp] = std::stod(sFirst);
        if(debug)
        	printf("%s @ %d indP %d  iComp %d indT %d\n", 
        		sFirst.c_str(), nComp*nT*indP + nComp*indT + iComp,	indP, iComp, indT);
      }
      ++indT;
      if(debug)
        printf( "1st:indT -> %d \n", indT);
      if(indT == nT) {
      	indT = 0;
      	++indP;
      	if(debug)
      	  printf("1st:indT==nT: indP -> %d indT %d\n", indP, indT);
      }
    }  
    if(! ss.str().empty()) {
      while(ss >> sd) {
        if(numPtcls>0 && indP >= numPtcls) {
        	if(debug)
        	  printf("break: numPtcls>0 && indP >= numPtcls \n");
          break;
        }
        // '=' if not with keyword, accept only if first next
        if(expectEqual) {
          expectEqual = false;
          if(sd=="=")
            continue;
        }

	      if(indP < numPtcls || !numPtcls) {
	        data[nComp*nT*indP + nComp*indT + iComp] = std::stod(sd);
	        if(debug)
	        	printf("%s @ %d indP %d  iComp %d indT %d\n", 
	        		sd.c_str(), nComp*nT*indP + nComp*indT + iComp, indP, iComp, indT);
	      }
        ++indT;
        if(debug)
        	printf( "indT -> %d \n", indT);
	      if(indT == nT) {
	      	indT = 0;
	      	++indP;
	      	if(debug)
      	    printf("indT==nT: indP -> %d  indT %d \n", indP, indT);
	      }
      } 
    }
    if(semi)
      dataLine = false;
  }
}


void processPtclFile(std::string& fName, std::vector<double>& data,
  PtclStruct& ps, int& numPtcls, bool debug=false) {
  std::ifstream ifs(fName);
  if (!ifs.good()) { 
    printf("Error opening PtclInitFile file %s \n", fName.c_str());
    exit(1);
  }

  // can't set in ps, since field names in ps used are not from array
  int nComp = ps.nComp;
  bool foundNP, foundNT, dataInit, foundComp[nComp], dataLine[nComp]; //6=x,y,z,vx,vy,vz
  std::string fieldNames[nComp];
  bool expectEqual = false;
  int indP[nComp], indT[nComp];
  std::set<int> nans;
  for(int i = 0; i < nComp; ++i) {
    indP[i] = 0;
    indT[i] = 0;
    foundComp[i] = dataLine[i] = false;
  }

  fieldNames[0] = ps.xName;
  fieldNames[1] = ps.yName;
  fieldNames[2] = ps.zName;
  fieldNames[3] = ps.vxName;
  fieldNames[4] = ps.vyName;
  fieldNames[5] = ps.vzName;      
  foundNP = foundNT = dataInit = false;
  std::string line, s1, s2, s3;
  while(std::getline(ifs, line)) {
    if(debug)
      std::cout << "Processing  line " << line << '\n';
    // depend on semicolon to mark the end of fields, otherwise
    // data of unused fields added to the previous valid field.
    bool semi = (line.find(';') != std::string::npos);
    std::replace (line.begin(), line.end(), ',' , ' ');
    std::replace (line.begin(), line.end(), ';' , ' ');
    std::stringstream ss(line);
    // first string or number of EACH LINE is got here
    ss >> s1;
    if(debug)
      std::cout << "str s1:" << s1 << "\n";
    
    // Skip blank line
    if(s1.find_first_not_of(' ') == std::string::npos) {
      s1 = "";
      if(!semi)
       continue;
    }
    if(s1 == ps.nPname) {
      ss >> s2 >> s3;
      assert(s2 == "=");
      ps.nP = std::stoi(s3);
      if(numPtcls <= 0)
        numPtcls = ps.nP;
      else if(numPtcls < ps.nP)
        ps.nP = numPtcls;
      else if(numPtcls > ps.nP) {
        numPtcls = ps.nP;
        if(debug)
          std::cout << "Warning: resetting numPtcls to max \n";
      }
      foundNP = true;
      if(debug)
          std::cout << "nP:" << ps.nP << " numPtcls " << numPtcls << "\n";
    }
    if(s1 == ps.nTname) {
      ss >> s2 >> s3;
      assert(s2 == "=");
      ps.nT = std::stoi(s3);
      foundNT = true;
      if(debug)
      	std::cout << " foundNT: " << ps.nT << "\n";
    }

    if(!dataInit && foundNP && foundNT) {
    	if(debug)
    	  std::cout << " nComp " << nComp <<  " nP:" << ps.nP 
    	    << " Vsize " << nComp*ps.nP*ps.nT << "\n";
      data.resize(nComp * ps.nP * ps.nT);
      dataInit = true;
    }
    int compBeg = 0, compEnd = nComp;
    // if ; ends data of each parameters, otherwise comment this block
    // to search for each parameter for every data line
    for(int iComp = 0; iComp<nComp; ++iComp) {
      if(dataInit && dataLine[iComp]) {
        compBeg = iComp;
        compEnd = compBeg + 1;
      }
    }
    // NOTE: NaN is replaced with 0 to preserve sequential index of particle
    //TODO change it in storeData()
    if(dataInit) {
      // stored in a single data array of 6+1 components.
      for(int iComp = compBeg; iComp<compEnd; ++iComp) {
        parseFileFieldData(ss, s1, fieldNames[iComp], semi, data, indP[iComp], 
          indT[iComp], dataLine[iComp], nans, expectEqual, iComp, nComp, 
          numPtcls, ps.nT, debug);

        if(!foundComp[iComp] && dataLine[iComp]) {
          foundComp[iComp] = true;
          if(debug)
            printf("Found data Component %d\n", iComp);
        }
      }
    }
    s1 = s2 = s3 = "";
  } //while
}

void findStoppedTimeStepsOfPtcls(std::vector<double>& data, 
  std::vector<int>& ptclStopIndex, int nT, int dof=6, bool print = false) {  
  double pos[3], vel[3];

  for(int ip=0; ip<ptclStopIndex.size(); ++ip) {
    double x[3] = {0, 0, 0};
    double v[3] = {0, 0, 0};
    for(int it=0; it<nT; ++it) {
      //nComp*nT*indP + nComp*indT + iComp
      auto beg = ip*nT*dof + it*dof;
      for(int i=0; i<3; ++i) {
        pos[i] = data[beg+i];
        vel[i] = data[beg+3+i];
      }

      if(same(pos[0], x[0]) && same(pos[1], x[1]) && same(pos[2], x[2])
        && same(vel[0],v[0]) && same(vel[1], v[1]) && same(vel[2],v[2])) {
         ptclStopIndex[ip] = it-1;
        // NOTE: pid start at 1
        if(print)
          printf("Stopped %d @t %d pos %g %g %g vel %g %g %g\n", 
            ip+1, it, pos[0], pos[1], pos[2], vel[0], vel[1], vel[2]);

         break;
      }
      for(int i=0; i<3; ++i) {
        x[i] = pos[i];
        v[i] = vel[i];
      }        
    }//for it
  } // for ip
}

//all time steps are read, since particle stopping needs it.
//nthStep-of-file1 = 1 to nT : picks only that from the given list, for each of pos & vel.
//nT in history file is that recorded after sub-sampling, not actual sim steps.
//printList is to re-shape history file from netcdf-dump format to list format.
//ptcl_positions_file : to check if selected entries of ptcl_history_file is 
// present in ptcl_positions_file, not the other way.
int main(int argc, char** argv) {
  if(argc < 2)
  {
    std::cout << "Usage: " << argv[0]  << " <ptcl_history_file> "
      << "[<nthStep-of-file1(-1=all optional)> <numPtcls(100k optional)> " 
      << "<printPtclStopped=1/0(0 optional)> <printList(0 optional)> " 
      << " <ptcl_positions_file(optional)> ]\n";
    exit(1);
  }
  bool debug = false;
  int printPtclStopped = 0;
  int printList = 0;

  std::string ptclHistory = argv[1];
  int nthStep1 = -1; //-1 for all time steps
  if(argc > 2)
    nthStep1 = atoi(argv[2]);
  
  int numPtcls = 100000;
  if(argc > 3)
    numPtcls = atoi(argv[3]);

  if(argc > 4)
    printPtclStopped = atoi(argv[4]);

  if(argc > 5)
    printList = atoi(argv[5]);

  std::string ptclPositions = "";
  if(argc > 6)
    ptclPositions = argv[6];

  std::vector<double> data1;
  PtclStruct pst("ptcl_init_data", "nP", "nT", "x", "y", "z", "vx", "vy", "vz");
  processPtclFile(ptclHistory, data1, pst, numPtcls, debug);

  constexpr int dof = 6;  
  int nT = pst.nT;

  assert(dof == pst.nComp);
  auto size = data1.size();
  fprintf(stderr, " size: %d  numPtcls %d nT %d nthStep1: %d printPtclStopped %d"
    "printList %d\n", size, numPtcls, nT, nthStep1, printPtclStopped, printList);

  assert(size == numPtcls*dof*nT); 
  double pos[3], vel[3];

  // for exact step info, input file should have all time steps
  // Last step nT-1 is by default
  std::vector<int> ptclStopIndex1(numPtcls,nT-1);
  findStoppedTimeStepsOfPtcls(data1, ptclStopIndex1, nT, dof, printPtclStopped);

  // print list of file1.
  if(printList) {
    if(nT < 1) {
      printf("Error: time step numbers (nT) not found \n");
      return 1;
    }
    for(int ip=0; ip<numPtcls; ++ip) {
      auto stop = ptclStopIndex1[ip];
      for(int it=0; it<nT; ++it) {
        if(nthStep1 <0 || nthStep1 == it) {
          auto beg = ip*nT*dof + it*dof;
          for(int i=0; i<3; ++i) {
            pos[i] = data1[beg+i];
            vel[i] = data1[beg+3+i];
          }
  	  // NOTE: pid start at 1
          printf("gitr %d tstep %d pos %g %g %g vel %g %g %g stopped %d\n", 
  	        ip+1, it, pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], stop);
        }
      }
    }
  } 
 
 if(ptclPositions.empty())
    return 0;

 // MATCHING. Check if 1st file contents are present in 2nd file
  int numPtcls2 = 0; //all
  std::vector<double> data2;
  PtclStruct pst2("ptcl_init_data", "nP", "nT", "x", "y", "z", "vx", "vy", "vz");
  processPtclFile(ptclPositions, data2, pst2, numPtcls2);
  // numPtcls2 is a reference and it got updated with max from file
  int nT2 = pst2.nT;
  fprintf(stderr, "numPtcls2 %d nT2 %d \n", numPtcls2, nT2);
  // Last step nT-1 is by default
  std::vector<int> ptclStopIndex2(numPtcls2,nT2-1);
  // To print stopped time steps, run this with file order changed
  findStoppedTimeStepsOfPtcls(data2, ptclStopIndex2, nT2, dof, false);

  int nMatch = 0, noMatch = 0;

  std::vector<int> matchSteps(numPtcls, -1);

  // based on ordered particles in both files !
  // check selected time steps
  double pos2[3], vel2[3];
  for(int i=0; i<numPtcls; ++i) {
    int it1 = ptclStopIndex1[i];
    auto beg1 = i*nT*dof + it1*dof;
    if(debug)
      std::cout << i << " it1 " << it1  << " @ " << beg1 << "\n";
    for(int k=0; k<3; ++k) {
      pos[k] = data1[beg1+k];
      vel[k] = data1[beg1+3+k];
    }
    // If dof is different, change it here
    // i is same
    assert(nT == nT2);
    int it2 = ptclStopIndex2[i];
    auto beg2 = i*nT2*dof + it2*dof;
    for(int k=0; k<3; ++k) {
      pos2[k] = data2[beg2+k];
      vel2[k] = data2[beg2+3+k];
    }
    if(debug)
      std::cout << pos[0] << " " << pos[1] << " " << pos[2] 
      << " " << pos2[0] << " " << pos2[1] << " " << pos2[2] << "\n";

    if(same(pos[0], pos2[0]) && 
    same(pos[1], pos2[1])  &&
    same(pos[2], pos2[2]) ) {
      ++nMatch;
      matchSteps[i] = it1; //note
    }
  }

  std::cout << "Matching last time steps of ptcls: out of " 
     << "nT " << nT << " nT2 " << nT2 << "\n";
  for(int i=0; i<numPtcls && i<numPtcls2; ++i) {
    std::cout << matchSteps[i] << " ";
  }
  std::cout << "Common time step for all ptcls: " << 
     *std::min_element(std::begin(matchSteps), std::end(matchSteps)) << "\n";

  std::cout << "Match: " << nMatch << " total_in_1st_file " << numPtcls << "\n";

  fprintf(stderr, "done\n");
  return 0;
}
