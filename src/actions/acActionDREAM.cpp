# include "acActionDREAM.h"

using namespace std;

// Constructor
acActionDREAM::acActionDREAM(int locTotChains,
                             int locTotGenerations,
                             int locTotalCR,
                             int loctTotCrossoverPairs,
                             string locDreamChainFileName,
                             string locDreamGRFileName,
                             double locDreamGRThreshold,
                             int locDreamJumpStep,
                             int locDreamGRPrintStep,
                             string locDreamRestartReadFileName,
                             string locDreamRestartWriteFileName,
                             // External Prior
                             bool usePriorFromFile,
                             string priorFileName,
                             int priorModelType){
  // Total number of chains
  totChains = locTotChains;
  // Total number of samples per chain
  totGenerations = locTotGenerations;
  // Total number of CR
  totalCR = locTotalCR;
  // Total number of crossover pairs
  totCrossoverPairs = loctTotCrossoverPairs;

  // GR Convergence parameters
  dreamGRThreshold = locDreamGRThreshold;
  dreamJumpStep = locDreamJumpStep;
  dreamGRPrintStep = locDreamGRPrintStep;

  // Output file handling: chains and GR convergence
  dreamChainFileName = locDreamChainFileName;
  dreamGRFileName = locDreamGRFileName;
   
  // Restart Functionalities
  dreamRestartReadFileName = locDreamRestartReadFileName;
  dreamRestartWriteFileName = locDreamRestartWriteFileName;

  // External Prior
  this->usePriorFromFile = usePriorFromFile;
  this->priorFileName = priorFileName;
  this->priorModelType = priorModelType;

  // Init Print Level to Silent
  this->printLevel = 0;

}

void acActionDREAM::setPrintLevel(int level){
  this->printLevel = level;
}

void acActionDREAM::std_compute_ini (int chain_num, int gen_index, int gen_num, int par_num, 
                                     double z[], double currentMean[], double currentStd[]){
  int i;
  int j;
  int k;
  for ( i = 0; i < par_num; i++ ){
    currentMean[i] = 0.0;
    for ( k = 0; k < gen_index; k++ ){
      for ( j = 0; j < chain_num; j++ ){
        currentMean[i] = currentMean[i] + z[i+j*par_num+k*par_num*chain_num];
      }
    }
    currentMean[i] = currentMean[i] / ( double ) ( chain_num ) / ( double ) ( gen_index );

    currentStd[i] = 0.0;
    for ( k = 0; k < gen_index; k++ ){
      for ( j = 0; j < chain_num; j++ ){
        currentStd[i] = currentStd[i] + ( z[i+j*par_num+k*par_num*chain_num] - currentMean[i] ) * ( z[i+j*par_num+k*par_num*chain_num] - currentMean[i] );
      }
    }
    currentStd[i] = sqrt ( currentStd[i] / ( double ) ( chain_num * gen_index - 1 ) );
  }
  return;
}

void acActionDREAM::std_compute (int chain_num, int gen_index, int gen_num, int par_num, double z[], 
                                 double storedMean[], double storedStd[], 
                                 double currentMean[], double currentStd[]){

  int i;
  int j;
  int k = gen_index;
  // Loop on parameters
  for ( i = 0; i < par_num; i++ ){
    currentMean[i] = ((chain_num * (gen_index))/(double)(chain_num *(gen_index+1))) * storedMean[i];  
    // Loop on all the chains
    for ( j = 0; j < chain_num; j++ ){
      currentMean[i] = currentMean[i] + (1.0/(double)(chain_num *(gen_index+1))) * z[i+j*par_num+k*par_num*chain_num];
    }

    currentStd[i] = ((chain_num * gen_index - 1)/(double)(chain_num *(gen_index+1) - 1)) * storedStd[i] * storedStd[i];
    for ( j = 0; j < chain_num; j++ ){
      currentStd[i] = currentStd[i] + (1.0/(double)(chain_num *(gen_index+1) - 1)) * ( z[i+j*par_num+k*par_num*chain_num] - currentMean[i]) * ( z[i+j*par_num+k*par_num*chain_num] - currentMean[i]);
    }
    // Back to standard deviation
    currentStd[i] = sqrt (currentStd[i]);
  }
  return;
}

int acActionDREAM::r8_round_i4 ( double x ){
  int value;

  if ( x < 0.0 ){
    value = - floor ( - x + 0.5 );
  }else{
    value =   floor (   x + 0.5 );
  }

  return value;
}

double* acActionDREAM::r8vec_copy_new(int n, double a1[]){
  double *a2;
  int i;

  a2 = new double[n];

  for(i = 0; i < n; i++){
    a2[i] = a1[i];
  }
  return a2;
}

void acActionDREAM::r8vec_heap_d(int n, double a[]){
  int i;
  int ifree;
  double key;
  int m;

  //  Only nodes (N/2)-1 down to 0 can be "parent" nodes.
  for (i = (n/2)-1; 0 <= i; i--){
//
//  Copy the value out of the parent node.
//  Position IFREE is now "open".
//
    key = a[i];
    ifree = i;

    for ( ; ; )
    {
//
//  Positions 2*IFREE + 1 and 2*IFREE + 2 are the descendants of position
//  IFREE.  (One or both may not exist because they equal or exceed N.)
//
      m = 2 * ifree + 1;
//
//  Does the first position exist?
//
      if ( n <= m )
      {
        break;
      }
      else
      {
//
//  Does the second position exist?
//
        if ( m + 1 < n )
        {
//
//  If both positions exist, take the larger of the two values,
//  and update M if necessary.
//
          if ( a[m] < a[m+1] )
          {
            m = m + 1;
          }
        }
//
//  If the large descendant is larger than KEY, move it up,
//  and update IFREE, the location of the free position, and
//  consider the descendants of THIS position.
//
        if ( key < a[m] )
        {
          a[ifree] = a[m];
          ifree = m;
        }
        else
        {
          break;
        }
      }
    }
//
//  When you have stopped shifting items up, return the item you
//  pulled out back to the heap.
//
    a[ifree] = key;
  }

  return;
}

void acActionDREAM::r8vec_sort_heap_a(int n, double a[])
//
//  Purpose:
//
//    R8VEC_SORT_HEAP_A ascending sorts an R8VEC using heap sort.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 April 1999
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    Albert Nijenhuis, Herbert Wilf,
//    Combinatorial Algorithms,
//    Academic Press, 1978, second edition,
//    ISBN 0-12-519260-6.
//
//  Parameters:
//
//    Input, int N, the number of entries in the array.
//
//    Input/output, double A[N].
//    On input, the array to be sorted;
//    On output, the array has been sorted.
//
{
  int n1;
  double temp;

  if ( n <= 1 )
  {
    return;
  }
//
//  1: Put A into descending heap form.
//
  r8vec_heap_d ( n, a );
//
//  2: Sort A.
//
//  The largest object in the heap is in A[0].
//  Move it to position A[N-1].
//
  temp = a[0];
  a[0] = a[n-1];
  a[n-1] = temp;
//
//  Consider the diminished heap of size N1.
//
  for ( n1 = n-1; 2 <= n1; n1-- )
  {
//
//  Restore the heap structure of the initial N1 entries of A.
//
    r8vec_heap_d ( n1, a );
//
//  Take the largest object from A[0] and move it to A[N1-1].
//
    temp = a[0];
    a[0] = a[n1-1];
    a[n1-1] = temp;
  }

  return;
}
//****************************************************************************80

double* acActionDREAM::r8vec_zero_new ( int n )

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_ZERO_NEW creates and zeroes an R8VEC.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    10 July 2008
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Output, double R8VEC_ZERO_NEW[N], a vector of zeroes.
//
{
  double *a;
  int i;

  a = new double[n];

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0.0;
  }
  return a;
}
//****************************************************************************80

int* acActionDREAM::i4vec_zero_new ( int n )

//****************************************************************************80
//
//  Purpose:
//
//    I4VEC_ZERO_NEW creates and zeroes an I4VEC.
//
//  Discussion:
//
//    An I4VEC is a vector of I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    11 July 2008
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Output, int I4VEC_ZERO_NEW[N], a vector of zeroes.
//
{
  int *a;
  int i;

  a = new int[n];

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0;
  }
  return a;
}

double* acActionDREAM::sample_candidate (int chain_index, int chain_num, double cr[], 
                                         int cr_index, int cr_num, int gen_index, int gen_num, 
                                         double jumprate_table[], int jumpstep, double limits[], int pair_num, 
                                         int par_num, double z[] )

//
//  Purpose:
//
//    SAMPLE_CANDIDATE generates candidate parameter samples.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jasper Vrugt, CJF ter Braak, CGH Diks, Bruce Robinson, James Hyman, 
//    Dave Higdon,
//    Accelerating Markov Chain Monte Carlo Simulation by Differential 
//    Evolution with Self-Adaptive Randomized Subspace Sampling,
//    International Journal of Nonlinear Sciences and Numerical Simulation,
//    Volume 10, Number 3, March 2009, pages 271-288.
//
//  Parameters:
//
//    Input, int CHAIN_INDEX, the chain index.
//    0 <= CHAIN_INDEX < CHAIN_NUM.
//
//    Input, int CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Input, double CR[CR_NUM], the CR values.
//
//    Input, int CR_INDEX, the index of the chosen CR value.
//    0 <= CR_INDEX < CR_NUM.
//
//    Input, int CR_NUM, the total number of CR values.
//    1 <= CR_NUM.
//
//    Input, int GEN_INDEX, the current generation.
//    0 <= GEN_INDEX < GEN_NUM.
//
//    Input, int GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Input, double JUMPRATE_TABLE[PAR_NUM], the jumprate table.
//
//    Input, int JUMPSTEP, forces a "long jump" every
//    JUMPSTEP generations.
//
//    Input, double LIMITS[2*PAR_NUM], limits for the parameters.
//
//    Input, int PAIR_NUM, the number of pairs of 
//    crossover chains.
//    0 <= PAIR_NUM.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, double Z[PAR_NUM*CHAIN_NUM*GEN_NUM], the Markov chain 
//    sample data.
//
//    Output, double SAMPLE_CANDIDATE[PAR_NUM], a candidate parameter sample.
//
//  Local parameters:
//
//    Input, int JUMP_DIM[JUMP_NUM], the dimensions in which
//    a jump is to be made.
//
//    Local, int JUMP_NUM, the number of dimensions in which
//    a jump will be made.  0 <= JUMP_NUM <= PAR_NUM.
//
//    Local, double JUMPRATE, the jump rate.
//
{
  double av;
  double b;
  double *diff;
  double *eps;
  int i;
  int *jump_dim;
  int jump_num;
  double jumprate;
  double *noise_e;
  int pair[2];
  int *r;
  double r2;
  double sd;
  double *zp;
//
//  Used to calculate E following a uniform distribution on (-B,+B).
//  Because B is currently zero, the noise term is suppressed.
//
  b = 0.0;
//
//  Pick pairs of other chains for crossover.
//
  r = new int[2*pair_num];

  for ( i = 0; i < pair_num; i++ ){
    
    while ( 1 ){
    
      r2 = uSampler->sample(0.0,1.0);
      pair[0] = ( int ) ( r2 * ( double ) chain_num );
      r2 = uSampler->sample(0.0,1.0);
      pair[1] = ( int ) ( r2 * ( double ) chain_num );

      if ( pair[0] != pair[1] && pair[0] != chain_index && pair[1] != chain_index ){
        break;
      }
    }
    r[0+i*2] = pair[0];
    r[1+i*2] = pair[1];
  }
//
//  Determine the jump rate.
//
  jump_dim = new int[par_num];

  jumprate_choose ( cr, cr_index, cr_num, gen_index, jump_dim, jump_num, 
    jumprate, jumprate_table, jumpstep, par_num );

  //  Calculate E in equation 4 of Vrugt.
  noise_e = new double[par_num];
  for(i = 0; i < par_num; i++){
    noise_e[i] = b * ( 2.0 * uSampler->sample(0.0,1.0) - 1.0 );
  }

  //  Get epsilon value from multinormal distribution
  eps = new double[par_num];
  av = 0.0;
  sd = 1.0E-10;
  for(i = 0; i < par_num; i++){
    eps[i] = nSampler->sample(av,sd);
  }
//
//  Generate the candidate sample ZP based on equation 4 of Vrugt.
//
  diff = diff_compute ( chain_num, gen_index, gen_num, jump_dim, jump_num, 
    pair_num, par_num, r, z );

  zp = new double[par_num];

  for ( i = 0; i < par_num; i++ )
  {
    zp[i] = z[i+chain_index*par_num+(gen_index-1)*par_num*chain_num];
  }
  for ( i = 0; i < par_num; i++ )
  {
    zp[i] = zp[i] + ( 1.0 + noise_e[i] ) * jumprate * diff[i] + eps[i];
  }
//
//  Enforce limits on the sample ZP.
//

  sample_limits ( limits, par_num, zp );

  delete [] diff;
  delete [] eps;
  delete [] jump_dim;
  delete [] noise_e;
  delete [] r;

  return zp;
}

int acActionDREAM::cr_index_choose (int cr_num, double cr_prob[])
//
//  Purpose:
//
//    CR_INDEX_CHOOSE chooses a CR index.
//
//  Discussion:
//
//    Index I is chosen with probability CR_PROB(I).
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int CR_NUM, the total number of CR values.
//    1 <= CR_NUM.
//
//    Input, double CR_PROB[CR_NUM], the probability of each CR.
//
//    Output, int CR_INDEX_CHOOSE, the index of the CR.
//    0 <= CR_INDEX_CHOOSE < CR_NUM.
//
{
  int cr_index;
  int i;
  int n;
  int *tmp_index;

  if (cr_num == 1){
  
    cr_index = 0;
    return cr_index;
  
  }else{ 
  
    // Need to test!!!!!
    stdVec pmf;
    for(int loopA=0;loopA<cr_num;loopA++){
      pmf.push_back(cr_prob[loopA]);
    }
    return catSampler->sample(pmf);
  }
}

void acActionDREAM::cr_init (double cr[], double cr_dis[], 
                             int cr_num, double cr_prob[], int cr_ups[] )

//
//  Purpose:
//
//    CR_INIT initializes the crossover probability values.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Output, double CR[CR_NUM], the CR values.
//
//    Output, double CR_DIS[CR_NUM], the CR distances.
//
//    Input, int CR_NUM, the total number of CR values.
//    1 <= CR_NUM.
//
//    Output, double CR_PROB[CR_NUM], the probability of each CR.
//
//    Output, int CR_UPS[CR_NUM], the number of updates
//    for each CR.
//
{
  int i;

  for (i=0;i<cr_num;i++){
    cr[i] = ( double ) ( i + 1 ) / ( double ) ( cr_num );
    cr_dis[i] = 1.0;
    cr_prob[i] = 1.0 / ( double ) ( cr_num );
    cr_ups[i] = 1;
  }
  return;
}

void acActionDREAM::restart_read(int chain_num, double fit[], int gen_num, int par_num, 
                                 string restart_read_filename, double z[])

//
//  Purpose:
//
//    RESTART_READ reads parameter sample data from a restart file.
//
//  Discussion:
//
//    Only a single generation (presumably the last one) was written to the file.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    07 April 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Output, double FIT[CHAIN_NUM*GEN_NUM], the likelihood of
//    each sample.
//
//    Input, int GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, string RESTART_READ_FILENAME, the name of 
//    the restart file.
//
//    Output, double Z[PAR_NUM*CHAIN_NUM*GEN_NUM], the Markov chain 
//    sample data.
//
{
  int chain_index;
  int gen_index = 0;
  int index;
  int dummy;
  string line;
  size_t n;
  int par_index;
  ifstream restart;

  restart.open ( restart_read_filename.c_str ( ) );

  if(!restart){
    cout << "\n";
    cout << "RESTART_READ - Fatal error!\n";
    cout << "  Could not open the file \"" 
         << restart_read_filename << "\".\n";
    exit ( 1 );
  }

  //  Read and ignore line 1.
  getline ( restart, line );

  //  Read the final fitness and parameter values for each chain.
  for(chain_index = 0; chain_index < chain_num; chain_index++){
    restart >> dummy;
    index = chain_index 
          + chain_num * gen_index;
    restart >> fit[index];
    for(par_index = 0; par_index < par_num; par_index++){
      index = par_index 
            + par_num * chain_index 
            + par_num * chain_num * gen_index;
      restart >> z[index];
    }
  }

  restart.close ( );

  return;
}

void acActionDREAM::gr_init(double gr[], int &gr_conv, int &gr_count, int gr_num, int par_num)

//
//  Purpose:
//
//    GR_INIT initializes Gelman-Rubin variables.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Output, double GR[PAR_NUM*GR_NUM], the Gelman-Rubin statistic.
//
//    Output, int &GR_CONV, the convergence flag.
//
//    Output, int &GR_COUNT, counts the number of generations
//    at which the Gelman-Rubin statistic has been computed.
//
//    Input, int GR_NUM, the number of times the Gelman-Rubin
//    statistic may be computed.
//
//    Input, int PAR_NUM, the number of parameters.
//    1 <= PAR_NUM.
//
{
  int i;
  int j;

  for(j = 0; j < gr_num; j++){
    for(i = 0; i < par_num; i++){
      gr[i+j*par_num] = 0.0;
    }
  }
  gr_conv = 0;
  gr_count = 0;

  return;
}

double* acActionDREAM::jumprate_table_init(int pair_num, int par_num)
//
//  Purpose:
//
//    JUMPRATE_TABLE_INIT initializes the jump rate table.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int PAIR_NUM, the number of pairs of 
//    crossover chains.
//    0 <= PAIR_NUM.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Output, double JUMPRATE_TABLE_INIT[PAR_NUM], the jumprate table.
//
{
  double c;
  int i;
  double *jumprate_table;
 
  jumprate_table = new double[par_num];

  c = 2.38 / sqrt ( ( double ) ( 2 * pair_num ) );

  for(i = 0; i < par_num; i++){
    jumprate_table[i] = c / sqrt ( ( double ) ( i + 1 ) );
  }

  return jumprate_table;
}

double* acActionDREAM::r8block_zero_new ( int l, int m, int n )
//
//  Purpose:
//
//    R8BLOCK_ZERO_NEW returns a new zeroed R8BLOCK.
//
//  Discussion:
//
//    An R8BLOCK is a triple dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    13 April 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int L, M, N, the number of rows and columns.
//
//    Output, double R8BLOCK_ZERO_NEW[L*M*N], the new zeroed matrix.
//
{
  double *a;
  int i;
  int j;
  int k;

  a = new double[l*m*n];

  for ( k = 0; k < n; k++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( i = 0; i < l; i++ )
      {
        a[i+j*l+k*l*m] = 0.0;
      }
    }
  }
  return a;
}

double* acActionDREAM::r8mat_zero_new ( int m, int n )
//
//  Purpose:
//
//    R8MAT_ZERO_NEW returns a new zeroed R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    03 October 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns.
//
//    Output, double R8MAT_ZERO_NEW[M*N], the new zeroed matrix.
//
{
  double *a;
  int i;
  int j;

  a = new double[m*n];

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      a[i+j*m] = 0.0;
    }
  }
  return a;
}

double acActionDREAM::prior_density(int par_num, double zp[], 
                                    int prior_num, int* prPtr, 
                                    const stdVec& prAv, const stdVec& prSd)
//
//  Purpose:
//
//    PRIOR_DENSITY evaluates the prior density function.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    25 May 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, double ZP[PAR_NUM], the argument of the density
//    function.
//
//    Output, real PRIOR_DENSITY, the value of the prior density function.
//
{
  double priorValue = 1.0;
  int currComponent = 0;
  for(int loopA=0;loopA<prior_num;loopA++){
    currComponent = prPtr[loopA];
    if(currComponent>=0){
      if(prSd[loopA]>0.0){
        priorValue = priorValue * nSampler->evaluate(prAv[loopA],prSd[loopA],zp[currComponent]);  
      }else{
        priorValue = priorValue * 1.0;
      }
      
    }
  }

  // Return Prior Value
  return priorValue;
}

void acActionDREAM::chain_outliers (int chain_num, int gen_index, int gen_num, 
                                    int par_num, double fit[], double z[])
//
//  Purpose:
//
//    CHAIN_OUTLIERS identifies and modifies outlier chains during burn-in.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jasper Vrugt, CJF ter Braak, CGH Diks, Bruce Robinson, James Hyman, 
//    Dave Higdon,
//    Accelerating Markov Chain Monte Carlo Simulation by Differential 
//    Evolution with Self-Adaptive Randomized Subspace Sampling,
//    International Journal of Nonlinear Sciences and Numerical Simulation,
//    Volume 10, Number 3, March 2009, pages 271-288.
//
//  Parameters:
//
//    Input, int CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Input, int GEN_INDEX, the index of the current generation.
//    2 <= GEN_INDEX <= GEN_NUM.
//
//    Input, int GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input/output, double FIT[CHAIN_NUM*GEN_NUM], the likelihood of
//    each sample.
//
//    Input/output, double Z[PAR_NUM*CHAIN_NUM*GEN_NUM], the Markov
//    chain sample data.
//
{
  double *avg;
  double avg_max;
  double *avg_sorted;
  int best;
  int i;
  int ind1;
  int ind3;
  int j;
  int klo;
  int knum;
  int k;
  int outlier_num;
  double q1;
  double q3;
  double qr;
  double t;

  klo = ( ( gen_index + 1 ) / 2 ) - 1;
  knum = gen_index + 1 - klo;

  avg = new double[chain_num];

  for (j = 0; j < chain_num; j++){
    t = 0.0;
    for (k = klo; k <= gen_index; k++){
      t = t + fit[j+k*chain_num];
    }
    avg[j] = t  / ( double ) ( knum );
  }
  
  //  Set BEST to be the index of the chain with maximum average.
  best = 0;
  avg_max = avg[0];
  for (j = 1; j < chain_num; j++){
    if (avg_max < avg[j]){
      best = j;
      avg_max = avg[j];
    }
  }

  //  Determine the indices of the chains having averages 1/4 "above" 
  //  and "below" the average.
  avg_sorted = r8vec_copy_new ( chain_num, avg );

  r8vec_sort_heap_a ( chain_num, avg_sorted );

  ind1 = r8_round_i4 ( 0.25 * ( double ) ( chain_num ) );
  ind3 = r8_round_i4 ( 0.75 * ( double ) ( chain_num ) );

  q1 = avg_sorted[ind1];
  q3 = avg_sorted[ind3];
  qr = q3 - q1;

  delete [] avg_sorted;

  //  Identify outlier chains, and replace their later samples
  //  with values from the "best" chain.
  outlier_num = 0;
  for(j = 0; j < chain_num; j++){
    if (avg[j] < q1 - 2.0 * qr){
      outlier_num = outlier_num + 1;
      for(i = 0; i < par_num; i++){
        z[i+j*par_num+gen_index*par_num*chain_num] = 
          z[i+best*par_num+gen_index*par_num*chain_num];
      }
      for (k = klo; k <= gen_index; k++){
        fit[j+k*chain_num]  = fit[best+k*chain_num];
      }
    }
  }
  
  //  List the outlier chains.
  if((outlier_num > 0)&&(printLevel > 0)){
    cout << "\n";
    cout << "CHAIN_OUTLIERS:\n";
    cout << "  At iteration " << gen_index 
         << " found " << outlier_num << " outlier chains,\n";
    cout << "  whose indices appear below, and for which samples\n";
    cout << "  from the chain with the largest log likelihood function,\n";
    cout << "  index number " << best << " will be substituted.\n";

    for (j = 0; j < chain_num; j++){
      if ( avg[j] < q1 - 2.0 * qr ){
        cout << "  " << j << "\n";
      }
    }
  }

  delete [] avg;

  return;
}

void acActionDREAM::chain_write (string chain_filename, int chain_num, double fit[], 
                                 int gen_num, int par_num, double z[] )
//
//  Purpose:
//
//    CHAIN_WRITE writes samples of each chain to separate files.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version John Burkardt.
//
//  Parameters:
//
//    Input, string CHAIN_FILENAME, the "base" filename
//    to be used for the chain files.  If this is ""
//    then the chain files will not be written.  This name should 
//    include a string of 0's which will be replaced by the chain 
//    indices.  For example, "chain000.txt" would work as long as the
//    number of chains was 1000 or less.
//
//    Input, int CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Input, double FIT[CHAIN_NUM*GEN_NUM], the likelihood of
//    each sample.
//
//    Input, int GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, double Z[PAR_NUM*CHAIN_NUM*GEN_NUM], the Markov chain 
//    sample data.
//
{
  ofstream chain;
  string chain_filename2;
  int i;
  int j;
  int k;

  //  Make a temporary copy of the filename template, which we can alter.
  chain_filename2 = chain_filename;

  //  Write parameter samples of all chains.
  if(printLevel > 0){
    cout << "\n";
    cout << "CHAIN_WRITE:\n";
  }

  for(j = 0; j < chain_num; j++){
    chain.open ( chain_filename2.c_str ( ) );

    if(!chain){
      cout << "\n";
      cout << "CHAIN_WRITE - Fatal error!\n";
      cout << "  Could not open file \"" << chain_filename2 << "\".\n";
      exit ( 1 );
    }

    chain <<
      "DREAM.CPP:Parameters_and_log_likelihood_for_chain_#" << j << "\n";

    for(k = 0; k < gen_num; k++){
      chain << "  " << k
            << "  " << fit[j+k*chain_num];
      for (i = 0; i < par_num; i++){
        chain << "  " << z[i+j*par_num+k*par_num*chain_num];
      }
      chain << "\n";
    }

    chain.close ( );

    if(printLevel > 0){
      cout << "  Created file \"" << chain_filename2 << "\".\n";
    }

    filename_inc ( &chain_filename2 );
  }
  return;
}

void acActionDREAM::cr_prob_update(double cr_dis[], int cr_num, double cr_prob[], int cr_ups[])
//
//  Purpose:
//
//    CR_PROB_UPDATE updates the CR probabilities.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, double CR_DIS[CR_NUM], the CR distances.
//
//    Input, int CR_NUM, the total number of CR values.
//    1 <= CR_NUM.
//
//    Output, double CR_PROB[CR_NUM], the updated CR probabilities.
//
//    Input, int CR_UPS[CR_NUM], the number of updates 
//    for each CR.
//
{
  double cr_prob_sum;
  int i;
 
  for (i = 0; i < cr_num - 1; i++){
    cr_prob[i] = cr_dis[i] / ( double ) cr_ups[i];
  }

  cr_prob_sum = uqUtils::sumvec(cr_num,cr_prob);

  for ( i = 0; i < cr_num - 1; i++ ){
    cr_prob[i] = cr_prob[i] / cr_prob_sum;
  }

  return;
}

double* acActionDREAM::diff_compute (int chain_num, int gen_index, int gen_num, 
                                     int jump_dim[], int jump_num, int pair_num, int par_num, int r[], 
                                     double z[] ) 
//
//  Purpose:
//
//    DIFF_COMPUTE computes the differential evolution.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jasper Vrugt, CJF ter Braak, CGH Diks, Bruce Robinson, James Hyman, 
//    Dave Higdon,
//    Accelerating Markov Chain Monte Carlo Simulation by Differential 
//    Evolution with Self-Adaptive Randomized Subspace Sampling,
//    International Journal of Nonlinear Sciences and Numerical Simulation,
//    Volume 10, Number 3, March 2009, pages 271-288.
//
//  Parameters:
//
//    Input, int CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Input, int GEN_INDEX, the index of the current generation.
//    1 <= GEN_INDEX <= GEN_NUM.
//
//    Input, int GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Input, int JUMP_DIM[JUMP_NUM], the dimensions in which
//    a jump is to be made.
//
//    Input, int JUMP_NUM, the number of dimensions in which
//    a jump will be made.  0 <= JUMP_NUM <= PAR_NUM.
//
//    Input, int PAIR_NUM, the number of pairs of 
//    crossover chains.
//    0 <= PAIR_NUM.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, int R[2*PAIR_NUM], pairs of chains used
//    to compute differences.
//
//    Input, double Z[PAR_NUM*CHAIN_NUM*GEN_NUM], the Markov chain 
//    sample data.
//
//    Output, double DIFF_COMPUTE[PAR_NUM], the vector of pair differences.
//
{
  double *diff;
  int i1;
  int i2;
  int j;
  int k;
  int pair;
  int r1;
  int r2;
//
//  Produce the difference of the pairs used for population evolution.
//
  diff = r8vec_zero_new ( par_num );

  for(pair = 0; pair < pair_num; pair++){
    r1 = r[0+pair*2];
    r2 = r[1+pair*2];
    for(j = 0; j < jump_num; j++){
      k = jump_dim[j];
      i1 = k+r1*par_num+(gen_index-1)*par_num*chain_num;
      i2 = k+r2*par_num+(gen_index-1)*par_num*chain_num;
      diff[k] = diff[k] + ( z[i1] - z[i2] );
    }
  }

  return diff;
}

void acActionDREAM::filename_inc ( string *filename )
//
//  Purpose:
//
//    FILENAME_INC increments a partially numeric file name.
//
//  Discussion:
//
//    It is assumed that the digits in the name, whether scattered or
//    connected, represent a number that is to be increased by 1 on
//    each call.  If this number is all 9's on input, the output number
//    is all 0's.  Non-numeric letters of the name are unaffected.
//
//    If the name is empty, then the routine stops.
//
//    If the name contains no digits, the empty string is returned.
//
//  Example:
//
//      Input            Output
//      -----            ------
//      "a7to11.txt"     "a7to12.txt"  (typical case.  Last digit incremented)
//      "a7to99.txt"     "a8to00.txt"  (last digit incremented, with carry.)
//      "a9to99.txt"     "a0to00.txt"  (wrap around)
//      "cat.txt"        " "           (no digits to increment)
//      " "              STOP!         (error)
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    22 November 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input/output, string *FILENAME, the filename to be incremented.
//
{
  char c;
  int change;
  int i;
  int lens;

  lens = (*filename).length ( );

  if ( lens <= 0 )
  {
    cout << "\n";
    cout << "FILENAME_INC - Fatal error!\n";
    cout << "  The input string is empty.\n";
    exit ( 1 );
  }

  change = 0;

  for ( i = lens - 1; 0 <= i; i-- )
  {
    c = (*filename)[i];

    if ( '0' <= c && c <= '9' )
    {
      change = change + 1;

      if ( c == '9' )
      {
        c = '0';
        (*filename)[i] = c;
      }
      else
      {
        c = c + 1;
        (*filename)[i] = c;
        return;
      }
    }
  }
//
//  No digits were found.  Return blank.
//
  if ( change == 0 )
  {
    for ( i = lens - 1; 0 <= i; i-- )
    {
      (*filename)[i] = ' ';
    }
  }

  return;
}

void acActionDREAM::gr_compute (int chain_num, int gen_index, int gen_num, double gr[], 
                                int &gr_conv, int &gr_count, int gr_num, double gr_threshold, 
                                int par_num, double z[])
//
//  Purpose:
//
//    GR_COMPUTE computes the Gelman Rubin statistics R used to check
//    convergence.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jasper Vrugt, CJF ter Braak, CGH Diks, Bruce Robinson, James Hyman, 
//    Dave Higdon,
//    Accelerating Markov Chain Monte Carlo Simulation by Differential 
//    Evolution with Self-Adaptive Randomized Subspace Sampling,
//    International Journal of Nonlinear Sciences and Numerical Simulation,
//    Volume 10, Number 3, March 2009, pages 271-288.
//
//  Parameters:
//
//    Input, int CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Input, int GEN_INDEX, the index of the current generation.
//    0 < GEN_INDEX < GEN_NUM.
//
//    Input, int GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Output, double GR[PAR_NUM*GR_NUM], the Gelman-Rubin R statistic.
//
//    Output, int &GR_CONV, the Gelman-Rubin convergence flag.
//
//    Input/output, int &GR_COUNT, counts the number of 
//    generations at which the Gelman-Rubin statistic has been computed.
//
//    Input, int GR_NUM, the number of times the Gelman-Rubin
//    statistic may be computed.
//
//    Input, double GR_THRESHOLD, the convergence tolerance for the
//    Gelman-Rubin statistic.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, double Z[PAR_NUM*CHAIN_NUM*GEN_NUM], the Markov chain 
//    sample data.
//
{
  double b_var;
  int chain_index;
  int ind0;
  int k;
  double mean_all;
  double *mean_chain;
  int par_index;
  double rnd0;
  double s;
  double s_sum;
  double var;
  double w_var;

  ind0 = ( ( gen_index + 1 ) / 2 ) - 1;
  rnd0 = ( double ) ( ind0 + 1 );

  mean_chain = new double[chain_num];

  for ( par_index = 0; par_index < par_num; par_index++ ){

    for ( chain_index = 0; chain_index < chain_num; chain_index++ ){
      mean_chain[chain_index] = 0.0;
      // Loop on half of the parameter sets collected so far
      for ( k = ind0; k <= gen_index; k++ ){
        mean_chain[chain_index] = mean_chain[chain_index] 
          + z[par_index+chain_index*par_num+k*par_num*chain_num];
      }
      mean_chain[chain_index] = mean_chain[chain_index] / rnd0;
    }
    // Overall chain Mean
    mean_all = uqUtils::sumvec(chain_num,mean_chain) / double(chain_num);

    // Compute variance of the chain means
    b_var = 0.0;
    for ( chain_index = 0; chain_index < chain_num; chain_index++ ){
      b_var = b_var + pow ( mean_chain[chain_index] - mean_all, 2 );
    }
    b_var = rnd0 * b_var / ( double ) ( chain_num - 1 );

    // Compute intra-chain average variance
    s_sum = 0.0;
    for ( chain_index = 0; chain_index < chain_num; chain_index++ ){
      s = 0.0;
      for ( k = ind0; k <= gen_index; k++ ){
        s = s + pow ( z[par_index+chain_index*par_num+k*par_num*chain_num] 
          - mean_chain[chain_index], 2 );
      }
      s_sum = s_sum + s;
    }
    s_sum = s_sum / ( rnd0 - 1.0 );

    w_var = s_sum / ( double ) ( chain_num );

    var = ( ( rnd0 - 1.0 ) * w_var + b_var ) / rnd0;
    
    // Careful: if one parameter is fixed then w_var is zero
    if(fabs(w_var) > 1.0e-8){
      gr[par_index+gr_count*par_num] = sqrt ( var / w_var );  
    }else{
      gr[par_index+gr_count*par_num] = 0.0;
    }
    
  }
//
//  Set the convergence flag.
//
  gr_conv = 1;

  for ( par_index = 0; par_index < par_num; par_index++ )
  {
    if ( gr_threshold < gr[par_index+gr_count*par_num] )
    {
      gr_conv = 0;
      break;
    }
  }

  if((gr_conv)&&(printLevel > 0)){
    cout << "\n";
    cout << "GR_COMPUTE:\n";
    cout << "  GR convergence at iteration: " << gen_index << "\n";
  }

  delete [] mean_chain;

  gr_count = gr_count + 1;

  return;
}

void acActionDREAM::gr_write(double gr[], string gr_filename, int gr_num, int par_num, int printstep)
//
//  Purpose:
//
//    GR_WRITE writes Gelman-Rubin R statistics into a file.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, double GR[PAR_NUM*GR_NUM], the Gelman-Rubin R statistic.
//
//    Input, string GR_FILENAME, the Gelman-Rubin filename.
//
//    Input, int GR_NUM, the number of times the Gelman-Rubin
//    statistic may be computed.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, int PRINTSTEP, the interval between generations on 
//    which the Gelman-Rubin statistic will be computed and written to a file.
//
{
  ofstream gr_unit;
  int i;
  int j;

  gr_unit.open ( gr_filename.c_str ( ) );

  if (! gr_unit){
    cout << "\n";
    cout << "GR_WRITE - Fatal error!\n";
    cout << "  Could not open the file \"" << gr_filename << "\"\n";
    exit(1);
  }

  gr_unit <<
    "DREAM.CPP:Monitored_parameter_interchains_Gelman_Rubin_R_statistic\n";

  for ( j = 0; j < gr_num; j++ )
  {
    gr_unit << printstep * ( j + 1 ) - 1;
    for ( i = 0; i < par_num; i++ )
    {
      gr_unit << "  " << gr[i+j*par_num];
    }
    gr_unit << "\n";
  }

  gr_unit.close ( );

  if(printLevel > 0){
    cout << "\n";
    cout << "GR_WRITE:\n";
    cout << "  Created the file \"" << gr_filename << "\".\n";    
  }

  return;
}

void acActionDREAM::jumprate_choose (double cr[], int cr_index, int cr_num, int gen_index,
                                     int jump_dim[], int &jump_num, double &jumprate, double jumprate_table[],
                                     int jumpstep, int par_num)
//
//  Purpose:
//
//    JUMPRATE_CHOOSE chooses a jump rate from the jump rate table.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jasper Vrugt, CJF ter Braak, CGH Diks, Bruce Robinson, James Hyman, 
//    Dave Higdon,
//    Accelerating Markov Chain Monte Carlo Simulation by Differential 
//    Evolution with Self-Adaptive Randomized Subspace Sampling,
//    International Journal of Nonlinear Sciences and Numerical Simulation,
//    Volume 10, Number 3, March 2009, pages 271-288.
//
//  Parameters:
//
//    Input, double CR[CR_NUM], the CR values.
//
//    Input, int CR_INDEX, the index of the CR.
//    1 <= CR_INDEX <= CR_NUM.
//
//    Input, int CR_NUM, the total number of CR values.
//    1 <= CR_NUM.
//
//    Input, int GEN_INDEX, the current generation.
//    1 <= GEN_INDEX <= GEN_NUM.
//
//    Output, int JUMP_DIM[PAR_NUM], the indexes of the
//    parameters to be updated.
//
//    Output, int &JUMP_NUM, the number of dimensions in which
//    a jump will be made.  0 <= JUMP_NUM <= PAR_NUM.
//
//    Output, double &JUMPRATE, the jump rate.
//
//    Input, double JUMPRATE_TABLE[PAR_NUM], the jump rate table.
//
//    Input, int JUMPSTEP, forces a "long jump" every
//    JUMPSTEP generations.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
{
  int i;
  double r;

  //  Determine the dimensions that will be updated.
  jump_num = 0;
  for(i = 0; i < par_num; i++){
    jump_dim[i] = 0;
  }

  for(i = 0; i < par_num; i++){
    r = uSampler->sample(0.0,1.0);

    if(1.0 - cr[cr_index] < r){
      jump_dim[jump_num] = i;
      jump_num = jump_num + 1;
    }
  }

  //  Calculate the general jump rate.
  if(jump_num == 0){
    jumprate = 0.0;
  }else{
    jumprate = jumprate_table[jump_num-1];
  }
  
  //  If parameter dimension is 1, 2, or 3, fix the jump rate to 0.6.
  if(par_num <= 3){
    jumprate = 0.6;
  }

  //  Determine if a long jump is forced.
  if ((gen_index % jumpstep) == 0){
    jumprate = 0.98;
  }
  return;
}

void acActionDREAM::sample_limits(double limits[], int par_num, double zp[])
//
//  Purpose:
//
//    SAMPLE_LIMITS enforces limits on a sample variable.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 May 2013
//
//  Author:
//
//    Original FORTRAN90 version by Guannan Zhang.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, double LIMITS[2*PAR_NUM], the parameter limits.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input/output, double ZP[PAR_NUM], a variable, whose entries,
//    if necessary, will be "folded" so that they lie within the limits.
//
{
  int i;
  double w;

  for(i = 0; i < par_num; i++){
    // Parameter Range
    w = limits[i*2 + 1] - limits[i*2 + 0];
    if ((w >= 0.0)&&(w < 1.0e-8)){
      // Fix Parameter
      zp[i] = limits[i*2 + 0];
    }else if ( w < 0.0 ){
      cout << "\n";
      cout << "SAMPLE_LIMITS - Fatal error!\n";
      cout << "  Upper limit less than lower limit.\n";
      exit ( 1 );
    }else{
      while ( zp[i] < limits[0+i*2] ){
        zp[i] = zp[i] + w;
      }
      while ( limits[1+i*2] < zp[i] ){
        zp[i] = zp[i] - w;
      }
    }
  }
  return;
}

void acActionDREAM::problem_size(int &chain_num, int &cr_num, int &gen_num, int &pair_num, int &par_num)
//
//  Purpose:
//
//    PROBLEM_SIZE sets information having to do with dimensions.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    25 May 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Output, int &CHAIN_NUM, the total number of chains.
//    3 <= CHAIN_NUM.
//
//    Output, int &CR_NUM, the total number of CR values.
//    1 <= CR_NUM.
//
//    Output, int &GEN_NUM, the total number of generations.
//    2 <= GEN_NUM.
//
//    Output, int &PAIR_NUM, the number of pairs of 
//    crossover chains.
//    0 <= PAIR_NUM.
//
//    Output, int &PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
{
  // LPN HEART MODELS - Total Parameters
  chain_num = totChains;
  cr_num = totalCR;
  // Number of sample per chain
  gen_num = totGenerations;
  pair_num = totCrossoverPairs;
  // Number Of Model Parameters
  par_num = model->getParameterTotal();

  return;
}

void acActionDREAM::problem_value (string *chain_filename, string *gr_filename, 
                                   double &gr_threshold, int &jumpstep, double limits[], int par_num, 
                                   int &printstep, string *restart_read_filename, string *restart_write_filename)
//
//  Purpose:
//
//    PROBLEM_VALUE sets information, including numeric data.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    26 May 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Output, string CHAIN_FILENAME, the "base" filename
//    to be used for the chain files.  If this is ""
//    then the chain files will not be written.  This name should 
//    include a string of 0's which will be replaced by the chain 
//    indices.  For example, "chain000.txt" would work as long as the
//    number of chains was 1000 or less.
//
//    Output, string *GR_FILENAME, the name of the file
//    in which values of the Gelman-Rubin statistic will be recorded,
//    or "" if this file is not to be written.
//
//    Output, double &GR_THRESHOLD, the convergence tolerance for
//    the Gelman-Rubin statistic.
//
//    Output, int &JUMPSTEP, forces a "long jump" every
//    JUMPSTEP generations.
//
//    Output, double LIMITS[2*PAR_NUM], lower and upper bounds
//    for each parameter.
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Output, int &PRINTSTEP, the interval between generations on 
//    which the Gelman-Rubin statistic will be computed and written to a file.
//
//    Output, string *RESTART_READ_FILENAME, the name of the file
//    containing restart information.  If this calculation is not a restart,
//    then this should be "".
//
//    Output, string *RESTART_WRITE_FILENAME, the name of the file
//    to be written, containing restart information.  If a restart file is not
//    to be written, this should be "".
//
{
  // GET PARAMETERS FOR THE HEART MODEL
  *chain_filename = dreamChainFileName.c_str();
  *gr_filename = dreamGRFileName.c_str();
  gr_threshold = dreamGRThreshold;
  jumpstep = dreamJumpStep;
  // GET PARAMETER RANGES FOR HEART LPN MODELS
  stdVec currLimits;
  model->getParameterLimits(currLimits);
  for(int loopA=0;loopA<currLimits.size();loopA++){
    limits[loopA] = currLimits[loopA];
  }
  printstep = dreamGRPrintStep;
  *restart_read_filename = dreamRestartReadFileName.c_str();
  *restart_write_filename = dreamRestartWriteFileName.c_str();

  return;
}

double* acActionDREAM::prior_sample ( int par_num, int prior_num, int* prPtr, const stdVec& prAv, const stdVec& prSd)
//
//  Purpose:
//
//    PRIOR_SAMPLE samples from the prior distribution.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    25 May 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Output, double PRIOR_SAMPLE[PAR_NUM], the sample from the distribution.
//
{

  double *zp;
  double currRange = 0.0;

  // ALLOCATE SAMPLE
  zp = ( double * ) malloc ( par_num * sizeof ( double ) );
  
  // GET PARAMETER RANGES
  stdVec currLimits;
  model->getParameterLimits(currLimits);
  double limits[(int)currLimits.size()];
  for(int loopA=0;loopA<currLimits.size();loopA++){
    limits[loopA] = currLimits[loopA];
  }

  // GET PARAMETER ESTIMATE
  stdVec stdStartingParams;
  model->getDefaultParams(stdStartingParams);
  double startingParams[(int)stdStartingParams.size()];
  for(int loopA=0;loopA<stdStartingParams.size();loopA++){
    startingParams[loopA] = stdStartingParams[loopA];
  }

  // GENERATE INITIAL SAMPLES
  for (int loopA=0;loopA<par_num;loopA++){
    zp[loopA] = uSampler->sample(limits[0+loopA*2],limits[1+loopA*2]);
  }

  // GENERATE FROM PRIOR
  int currComponent = 0;
  for (int loopA=0;loopA<prior_num;loopA++){
    currComponent = prPtr[loopA];
    if(currComponent>=0){
      // Sample From Prior
      if(prSd[loopA]>0.0){
        zp[currComponent] = nSampler->sample(prAv[loopA],prSd[loopA]);  
      }else{
        zp[currComponent] = prAv[loopA];
      }        
    }
  }
  
  // RETURN
  return zp;
}

double acActionDREAM::sample_likelihood (int par_num, double zp[])
//
//  Purpose:
//
//    SAMPLE_LIKELIHOOD computes the log likelihood function.
//
//  Discussion:
//
//    This is a one mode Gaussian.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    25 May 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int PAR_NUM, the total number of parameters.
//    1 <= PAR_NUM.
//
//    Input, double ZP[PAR_NUM], a sample.
//
//    Output, double SAMPLE_LIKELIHOOD, the log likelihood function 
//    for the sample.
//
{
  // States
  int state_num = model->getStateTotal();
  // Number of Result quantities
  int res_num = model->getResultTotal();

  // EVAL LIKELIHOOD  
  stdVec inputs;
  for(int loopA=0;loopA<par_num;loopA++){
    inputs.push_back(zp[loopA]);
  }
  stdVec outputs;
  stdIntVec errors;
  double value = 0.0;
  int error = 0;
  try{
    value = -model->evalModelError(inputs,outputs,errors);  
  }catch(exception &e){
    value = -10000.0;
  }  

  // Print Log Likelihood
  if(printLevel > 0){
    printf("LL: %.5f\n",value);
    fflush(stdout);
  }
  
  // Return
  return value;
}


