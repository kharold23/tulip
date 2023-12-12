#include "svZeroD_R_RCR.h"

using namespace std;

svZeroD_R_RCR::svZeroD_R_RCR(std::string target_file, std::string perfusion_volumes_file) {
  // Read target flows
  this->readTargetsFromFile(target_file);

}

// ==========================
// SET UP MODEL-SPECIFIC PARAMETERS FOR OPTIMIZATION
// ==========================
void svZeroD_R_RCR::setupModel(LPNSolverInterface& interface){
  
  // Load shared library and get interface functions.
  this->nUnknowns = interface.system_size_;
  
  // Number of blocks and number of each type
  this->num_blocks = interface.block_names_.size();
  std::string block_name, branch_name;
  for (int i = 0; i < this->num_blocks; i++) {
    block_name = interface.block_names_[i];
    std::cout << block_name << std::endl;
    this->names.push_back(block_name);
  }

  // Initialize parameter vectors and read baseline block params
  // Baseline parameters (named *_base) are set in the svZeroD config file
  
  // Only get parameters for the Vessel
  this->read_R.resize(4);
  this->R_base.reserve(1);
  interface.read_block_params(this->names[0], this->read_R);
  this->R_base[0] = this->read_R[0];
  
  // Save solution IDs corresponding to important quantities (Vessel pressure)
  this->P_ids.resize(1);
  this->P_ids_names.resize(1);
  int ct = 0;
  string var_name;
  bool press_flag;
  
  // Iterate through the names of variables in the 0D system
  for (int i = 0; i < interface.system_size_; i++) {
    var_name = interface.variable_names_[i];
    // std::cout << var_name << std::endl;
    press_flag = (var_name.substr(0,8) == "pressure"); // Is this a pressure variable?

    // Find last occurence of ":" in variable name. 
    // The rest of var_name is either the exit block name or the main block name (if it is an internal variable).
    std::size_t blk_name_start = var_name.rfind(":");
    if (blk_name_start == std::string::npos) {
      std::cout<<"Error: Invalid variable name format for:"<<var_name<<std::endl;
      throw std::runtime_error("Error: Invalid variable name format.");
    }
    
    // If the variable is the pressure for branch0_seg0
    if (press_flag) {
      var_name = var_name.substr(blk_name_start+1,var_name.length()-1);
      if (var_name == "branch0_seg0"){
          this->P_ids[ct] = i;
          this->P_ids_names[ct] = var_name;
          std::cout<<"Found P_ids for "<<P_ids_names[ct]<<std::endl;
          ct++;
      }
    }
  }
  
  // Check to make sure all variables ids have been assigned
  for (int i = 0; i < this->num_blocks; i++) {
    if (this->P_ids[i] < 0) {
      std::cout << "P index: "<< i << std::endl;
      std::cout << "Error: Did not find all solution IDs" << std::endl;
      throw std::runtime_error("Error: Did not find all solution IDs");
    }
  }
  
  if (this->target_press.size() != (this->P_ids.size()) * 2) {
    std::cout << "Error: Number of target pressures does not match number of saved pressure state IDs" << std::endl;
    throw std::runtime_error("Error: Number of target pressures does not match number of saved pressure state IDs");
  }

  //Add target_flows and outlet_names to header file
  int idx;
  std::vector<double> targets_copy = this->target_press;
  for (int i = 0; i < this->P_ids.size(); i++) {
    std::cout<<"Branch name: "<<this->P_ids_names[i]<<std::endl;
    auto itr = find(this->outlet_names.begin(), this->outlet_names.end(), this->P_ids_names[i]);
    if(itr != this->outlet_names.end()) {
      idx = std::distance(this->outlet_names.begin(), itr);
      std::cout<<"Found in targets at idx: "<<idx<<std::endl;
      this->target_press[i * 2] = targets_copy[idx * 2];
      this->target_press[i * 2 + 1] = targets_copy[idx * 2 + 1];
    } else {
      throw std::runtime_error("Error: Could not find "+this->P_ids_names[i]+" in outlet_names.");
    }
  }
}

// ==========================
// READ TARGET DATA FROM FILE
// ==========================
void svZeroD_R_RCR::readTargetsFromFile(string targetFileName)
{
  std::ifstream read_file;
  read_file.open(targetFileName.c_str());
  
  std::string buffer;
  std::vector<std::string> tokens;
  if(!read_file.is_open())
    throw cmException("ERROR: Cannot open targets file");
  if(read_file.eof())
    throw cmException("ERROR: No targets found!");

  while(std::getline(read_file,buffer))
  {
    cmUtils::schSplit(buffer,tokens," ");
    this->target_press.push_back(atof(tokens[1].c_str()));
    this->target_press.push_back(atof(tokens[2].c_str()));
    this->outlet_names.push_back(tokens[0].c_str());
  }
  
  read_file.close();
}

// ==========================
// READ PERFUSION DATA FROM FILE
// ==========================
void svZeroD_R_RCR::readPerfusionFile(string perfusionFileName)
{
  std::ifstream read_file;
  read_file.open(perfusionFileName.c_str());
  
  std::string buffer;
  std::vector<std::string> tokens;
  if(!read_file.is_open())
    throw cmException("ERROR: Cannot open perfusion file");
  if(read_file.eof())
    throw cmException("ERROR: No perfusion data found!");
  
  int line_ct = 0;
  while(std::getline(read_file,buffer))
  {
    if (line_ct > 0) { //skip header
      cmUtils::schSplit(buffer,tokens," ");
      this->perfusion_data.insert({tokens[0].c_str(),atof(tokens[1].c_str())});
    }
    line_ct++;
  }
  
  read_file.close();
}

// ========================
// GET NUMBER OF PARAMETERS
// ========================
int svZeroD_R_RCR::getParameterTotal(){
  return 1; //Just the resistance for the vessel
}

// ===================================
// GET NUMBER OF PARAMETERS (UNKNOWNS)
// ===================================
int svZeroD_R_RCR::getStateTotal(){
  return this->nUnknowns; 
}

// ===========================
// GET TOTAL NUMBER OF RESULTS
// ===========================
int svZeroD_R_RCR::getResultTotal(){
  return 2; // max and min pressure
}

// ==================
// GET PARAMETER NAME
// ==================
string svZeroD_R_RCR::getParamName(int index) {
  if (index < 1) {
    return this->P_ids_names[index];
  } else {
    throw std::runtime_error("ERROR: Invalid index in svZeroD_R_RCR::getParamName(index)");
  }
}

// ==================
// GET PARAMETER NAME
// ==================
string svZeroD_R_RCR::getResultName(int index) {
  if (index < 2) {
    return this->P_ids_names[0];
  } else {
    throw std::runtime_error("ERROR: Invalid index in svZeroD_R_RCR::getResultName(index)");
  }
}

// ====================
// RETURN PARAMETER SPECIFIED BY STRING SPECIFIER
// ====================
void svZeroD_R_RCR::getSpecifiedParameter(string& specifier, double& return_db_param, int& return_int_param) {
  if (specifier == "RScaling") {
    //return_db_param = this->R_scaling;
    std::cout << "RScaling not implemented for this case." << std::endl;
  } else {
    throw std::runtime_error("ERROR: Invalid specifier in svZeroD_R_RCR::getSpecifiedParameter.");
  }
}

// ====================
// GET MODEL PARAMETERS
// ====================
void svZeroD_R_RCR::getDefaultParams(stdVec& default_params){
      
  default_params.resize(getParameterTotal());

  for (int i = 0; i < getParameterTotal(); i++) {
    default_params[i] = 110.0;
  }

}

// ====================
// GET PARAMETER RANGES
// ====================
void svZeroD_R_RCR::getParameterLimits(stdVec& limits){

  limits.resize(2*getParameterTotal());
  for (int i = 0; i < getParameterTotal(); i++) {
    limits[2*i] = 10.0;
    limits[2*i+1] = 1000.0;
  }

}

void svZeroD_R_RCR::printResults(int totalResults, double* Xn) {
  printf("RESULT PRINTOUT\n");
  for(int loopA = 0; loopA < totalResults; loopA++) {
    string s = getResultName(loopA);
    printf("%20s : %f\n",s.c_str(),Xn[loopA]);
  }
}


// ==========================================
// UPDATE PARAMETERS OF THE ZEROD MODEL
// ==========================================
void svZeroD_R_RCR::setModelParams(LPNSolverInterface& interface, const stdVec& params) {

  std::string block_name;

  // Update the model parameters
  this->read_R[0] = params[0];
  block_name = this->names[0];
  interface.update_block_params(block_name, this->read_R);
}

// ==========================================
// POSTPROCESS ZEROD SIMULATION
// ==========================================
void svZeroD_R_RCR::postProcess(LPNSolverInterface& interface, const stdVec& t, const stdMat& outVals,const stdMat& auxOutVals, stdVec& results) {
  int totOutputSteps = interface.num_output_steps_;
  int totalStepsOnSingleCycle = interface.pts_per_cycle_;
  
  // std::cout << outVals[0].size() << std::endl; Returns 2001
  // std::cout << totOutputSteps << ", " << totalStepsOnSingleCycle << std::endl; Returns 2001, 201
  
  double min_block_press;
  double max_block_press;
  for(int i = 0; i < P_ids.size(); i++){  
    max_block_press = 0.0;
    for (int t = totOutputSteps - totalStepsOnSingleCycle - 1; t < totOutputSteps; t++) {
      if (outVals[this->P_ids[i]][t] > max_block_press) {
        max_block_press = outVals[this->P_ids[i]][t];
      }
    }
    min_block_press = max_block_press;
    for (int t = totOutputSteps - totalStepsOnSingleCycle - 1; t < totOutputSteps; t++) {
      if (outVals[this->P_ids[i]][t] < min_block_press) {
        min_block_press = outVals[this->P_ids[i]][t];
      }
    }
    results[2*i] = min_block_press;
    results[2*i + 1] = max_block_press;
  }

}


// =========================
// EVAL MODEL ERROR FUNCTION
// =========================
double svZeroD_R_RCR::evalModelError(std::vector<double>& results) {
  //std::cout << "[evalModelError] START " << std::endl;

  int totalParams = getParameterTotal();
  int resultTotal = getResultTotal();

  // Compute mean squared percentage error and mean percentage error of flow fractions
  double loss = 0.0;
  double sq_pct_error = 0.0, pct_error = 0.0;
  for(int i=0; i < resultTotal; i++){
    sq_pct_error = (results[i] - this->target_press[i])*(results[i] - this->target_press[i])/(this->target_press[i]*this->target_press[i]);
    loss += sq_pct_error;
    pct_error += sqrt(sq_pct_error);
    std::cout<<"[evalModelError] results, targets: "<<results[i]<<" "<<this->target_press[i]<<std::endl;
  }
  loss = loss/double(resultTotal); // mean of square pct error
  pct_error = 100.0*pct_error/double(resultTotal); // mean pct error
  std::cout<<"[evalModelError] Mean pct error: "<<pct_error<<std::endl;


  //std::cout << "[evalModelError] END " << std::endl;
  return loss;
}

// ====================
// GET DEFAULT RANGES
// ====================
void svZeroD_R_RCR::getDefaultParameterLimits(stdVec& limits) {
  std::cout<<"ERROR: svZeroD_R_RCR::getDefaultParameterLimits not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_R_RCR::getDefaultParameterLimits not implemented.");
}

// ====================
// GET PRIOR MAPPING
// ====================
void svZeroD_R_RCR::getPriorMapping(int priorModelType,int* prPtr) {
  std::cout<<"ERROR: svZeroD_R_RCR::getPriorMapping not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_R_RCR::getPriorMapping not implemented.");
}

// ====================
// GET NUMBER OF AUXILIARY STATES
// ====================
int svZeroD_R_RCR::getAuxStateTotal(){
  std::cout<<"ERROR: svZeroD_R_RCR::getAuxStateTotal not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_R_RCR::getAuxStateTotal not implemented.");
  return 0;
}

// =========================
// KEY/NAME FOR EACH TARGET QUANTITY
// =========================
void svZeroD_R_RCR::getResultKeys(vector<string>& keys) {
  std::cout<<"ERROR: svZeroD_R_RCR::getResultKeys not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_R_RCR::getResultKeys not implemented.");
}

// =========================
// STANDARD DEVIATION OF EACH TARGET MEASUREMENT
// =========================
void svZeroD_R_RCR::getDataStd(stdVec& stdFactors) {
  std::cout<<"ERROR: svZeroD_R_RCR::getDataStd not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_R_RCR::getDataStd not implemented.");
}

// =========================
// INVERSE WEIGHT OF EACH TARGET QUANTITY IN LOG LIKELIHOOD
// =========================
void svZeroD_R_RCR::getResultWeights(stdVec& weights) {
  std::cout<<"ERROR: svZeroD_R_RCR::getResultWeights not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_R_RCR::getResultWeights not implemented.");
}
