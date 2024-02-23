// File Prod_Helpers.hh
#ifndef Prod_HELPERS_HH
#define Prod_HELPERS_HH

#include "Prod_Data.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;

/***************************************************************************
 * State Manager 
 ***************************************************************************/
class Prod_SolutionManager : public SolutionManager<Prod_Input,Prod_Output> 
{
public:
  Prod_SolutionManager(const Prod_Input &);
  void RandomState(Prod_Output& out) override;   
  void GreedyState(Prod_Output& out) override;   
  void DumpState(const Prod_Output& out, ostream& os) const override { out.Dump(os); }   
  bool CheckConsistency(const Prod_Output& out) const override;
protected:
};

class Delay : public CostComponent<Prod_Input,Prod_Output> 
{
public:
  Delay(const Prod_Input & in, int w, bool hard) : CostComponent<Prod_Input,Prod_Output>(in,w,hard,"Delay") 
  {}
  int ComputeCost(const Prod_Output& st) const override;
  void PrintViolations(const Prod_Output& st, ostream& os = cout) const override;
};

class Advance: public CostComponent<Prod_Input,Prod_Output> 
{
public:
  Advance(const Prod_Input & in, int w, bool hard) : CostComponent<Prod_Input,Prod_Output>(in,w,hard,"Advance") 
  {}
  int ComputeCost(const Prod_Output& st) const override;
  void PrintViolations(const Prod_Output& st, ostream& os = cout) const override;
};

class LineOverLoad : public CostComponent<Prod_Input,Prod_Output>
{
public:
  LineOverLoad (const Prod_Input & in, int w, bool hard) : CostComponent<Prod_Input,Prod_Output>(in,w,hard,"LineOverLoad ") 
  {}
  int ComputeCost(const Prod_Output& st) const override;
  void PrintViolations(const Prod_Output& st, ostream& os = cout) const override;
};

class LinesToAOrder : public CostComponent<Prod_Input,Prod_Output> 
{
public:
  LinesToAOrder (const Prod_Input & in, int w, bool soft) : CostComponent<Prod_Input,Prod_Output>(in,w,soft,"LinesToAOrder") 
  {}
  int ComputeCost(const Prod_Output& st) const override;
  void PrintViolations(const Prod_Output& st, ostream& os = cout) const override;
};

class CategoryDeviation : public CostComponent<Prod_Input,Prod_Output> 
{
public:
  CategoryDeviation (const Prod_Input & in, int w, bool soft) : CostComponent<Prod_Input,Prod_Output>(in,w,soft,"CategoryDeviation") 
  {}
  int ComputeCost(const Prod_Output& st) const override;
  void PrintViolations(const Prod_Output& st, ostream& os = cout) const override;
};
/***************************************************************************
 * Prod_MoveChange Neighborhood Explorer:
 ***************************************************************************/

class Prod_MoveChange
{
  friend bool operator==(const Prod_MoveChange& m1, const Prod_MoveChange& m2);
  friend bool operator!=(const Prod_MoveChange& m1, const Prod_MoveChange& m2);
  friend bool operator<(const Prod_MoveChange& m1, const Prod_MoveChange& m2);
  friend ostream& operator<<(ostream& os, const Prod_MoveChange& c);
  friend istream& operator>>(istream& is, Prod_MoveChange& c);
 public:
  static bool Inverse(const Prod_MoveChange& m1, const Prod_MoveChange& m2) 
    {return m1.order==m2.order && m1.operation==m2.operation && m1.new_l == m2.old_l && m1.old_l == m2.new_l && m1.new_d == m2.old_d && m1.old_d == m2.new_d;}
  Prod_MoveChange();
  //move attributes:
  int order;
  int operation;
  int old_l;
  int new_l;
  int old_d;
  int new_d;
}; 

class Prod_MoveChangeNeighborhoodExplorer
  : public NeighborhoodExplorer<Prod_Input, Prod_Output, Prod_MoveChange> 
{
	public:
  Prod_MoveChangeNeighborhoodExplorer(const Prod_Input & pin, SolutionManager<Prod_Input,Prod_Output>& psm)  
    : NeighborhoodExplorer<Prod_Input,Prod_Output,Prod_MoveChange>(pin, psm, "Prod_MoveChangeNeighborhoodExplorer") {} 
  void RandomMove(const Prod_Output& out, Prod_MoveChange& mv) const override;          
  bool FeasibleMove(const Prod_Output& out, const Prod_MoveChange& mv) const override;  
  void MakeMove(Prod_Output& out, const Prod_MoveChange& mv) const override;             
  void FirstMove(const Prod_Output& out, Prod_MoveChange& mv) const override;  
  bool NextMove(const Prod_Output& out, Prod_MoveChange& mv) const override; 
	protected:
  bool AnyNextMove(const Prod_Output& out, Prod_MoveChange& mv) const;
};

class Prod_MoveChangeDeltaDelay
  : public DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange> 
{
public:
  Prod_MoveChangeDeltaDelay(const Prod_Input & in, Delay& cc) 
    : DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange> (in,cc,"Prod_MoveChangeDeltaDelay") 
  {}
  int ComputeDeltaCost(const Prod_Output& out, const Prod_MoveChange& mv) const override;
};

class Prod_MoveChangeDeltaAdvance
  : public DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange> 
{
public:
  Prod_MoveChangeDeltaAdvance(const Prod_Input & in, Advance& cc) 
    : DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange>(in,cc,"Prod_MoveChangeDeltaAdvance") 
  {}
  int ComputeDeltaCost(const Prod_Output& st, const Prod_MoveChange& mv) const override;
};

class Prod_MoveChangeDeltaLineOverLoad
  : public DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange> 
{
public:
  Prod_MoveChangeDeltaLineOverLoad(const Prod_Input & in, LineOverLoad& cc) 
    : DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange>(in,cc,"Prod_MoveChangeDeltaLineOverLoad") 
  {}
  int ComputeDeltaCost(const Prod_Output& st, const Prod_MoveChange& mv) const override;
};

class Prod_MoveChangeDeltaLinesToAOrder
  : public DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange> 
{
public:
  Prod_MoveChangeDeltaLinesToAOrder(const Prod_Input & in, LinesToAOrder& cc) 
    : DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange>(in,cc,"Prod_MoveChangeDeltaLinesToAOrder") 
  {}
  int ComputeDeltaCost(const Prod_Output& st, const Prod_MoveChange& mv) const override;
};

class Prod_MoveChangeDeltaCategoryDeviation
  : public DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange> 
{
public:
  Prod_MoveChangeDeltaCategoryDeviation(const Prod_Input & in, CategoryDeviation& cc) 
    : DeltaCostComponent<Prod_Input, Prod_Output, Prod_MoveChange>(in,cc,"Prod_MoveChangeDeltaCategoryDeviation") 
  {}
  int ComputeDeltaCost(const Prod_Output& st, const Prod_MoveChange& mv) const override;
};
#endif
