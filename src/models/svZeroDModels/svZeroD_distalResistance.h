#ifndef SVZERODDISTALRESISTANCE
#define SVZERODDISTALRESISTANCE

# include "svZeroDModel.h"

using namespace std;

// DEFINE CONVERSION CONSTANT
const double pConv = 1.334E3;
const double qConv = 1E0;

class svZeroD_distalResistance: public svZeroDModel {
   
  public:
    
    //DEFAULT CONSTRUCTOR
    svZeroD_distalResistance(std::string input_target_file, std::string input_perufusion_volumes_file);

    // SET UP MODEL PARAMETERS
    virtual void setupModel(LPNSolverInterface& interface, std::vector<std::string> string_inputs);

    // GET NUMBER OF PARAMETERS
    virtual int getParameterTotal();

    // GET NUMBER OF PARAMETERS (UKNOWNS)
    virtual int getStateTotal();

    // GET TOTAL NUMBER OF RESULTS
    virtual int getResultTotal();

    // GET THE PARAMETER NAMES
    virtual string getParamName(int index);

    // GET THE RESULT NAMES
    virtual string getResultName(int index);

    // GET THE PARAMETER RANGES FOR THE LPN MODELS
    virtual void getParameterLimits(stdVec& limits);

    // GET PARAMETER SETS FOR THE LPN MODELS     
    virtual void getDefaultParams(stdVec& zp);

    // UPDATE ZEROD MODEL PARAMETERS
    virtual void setModelParams(LPNSolverInterface& interface, const stdVec& params);

    // POSTPROCESS ZEROD SIMULATION
    virtual void postProcess(LPNSolverInterface& interface, const stdVec& t, const stdMat& outVals,const stdMat& auxOutVals, stdVec& results);

    // SOLVE MODEL AND GET LOG LIKELIHOOD TO INTERFACE WITH DREAM
    virtual double evalModelError(stdVec inputs, stdVec& outputs, stdIntVec& errorCode);

    // SET 3D MODEL SURROGATE FILE
    //void set3DSurrogateFile(string surrModelFileName);

    // RETURN THE NUMBER OF EXTRA OUTPUTS
    //int  getAuxStateTotal();

    // READ CORONARY PARAMETERS FROM FILE
    //void readParamsFromFile(stdVec& inputs, std::string param_path);

    // PRINT OUT RESULTS
    void printResults(int totalResults, double *Xn);

    // READ TARGET FLOWS FROM A FILE
    void readTargetsFromFile(string targetFileName);

    // READ PERFUSION DATA FROM A FILE
    void readPerfusionFile(string perfusionFileName);

    // RETURN SCALING FACTOR FOR TOTAL RESISTANCE
    double getRScaling();

  protected:

    int num_blocks;
    std::vector<double> init_state_y, init_state_ydot;
    bool use_perfusion = false;
    std::map<std::string,double> perfusion_data;
    std::vector<double> coronary_params;
    std::vector<int> Q_lca_ids;
    std::vector<int> Q_rca_ids;
    std::vector<std::string> Q_lca_ids_names;
    std::vector<std::string> Q_rca_ids_names;
    std::vector<std::string> outlet_names;
    std::vector<double> target_flows;
    std::vector<double> target_flow_fracs;
    double total_target_flow;
    double R_total_inv_base;
    double R_scaling = 1.0;

    int numCycles;
    int total3DSteps;
    int total0DSteps;
    // Number of outlets and unknowns
    int nUnknowns;
    int nFaces;
    int nCOR;
    int nCOR_l;
    int nCOR_r;
    //int nRCR;
    std::vector<std::string> names_corBC_l, names_corBC_r;
    // Base vectors to store raw output from reading mesh data
    std::vector<double> Ra_l_base;
    std::vector<double> Ra_r_base;
    std::vector<double> Ram_l_base;
    std::vector<double> Ram_r_base;
    std::vector<double> Rv_l_base;
    std::vector<double> Rv_r_base;
    std::vector<double> Cim_l_base;
    std::vector<double> Ca_l_base;
    std::vector<double> Cim_r_base;
    std::vector<double> Ca_r_base;
    std::vector<double> iml_base;
    std::vector<double> imr_base;
};

#endif // SVZERODDISTALRESISTANCE