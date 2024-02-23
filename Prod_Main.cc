#include "Prod_Helpers.hh"

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
  ParameterBox main_parameters("main", "Main Program options");

  // The set of arguments added are the following:
  Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<int> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);

  CommandLineParameters::Parse(argc, argv, false, true);
 
  
  if (!instance.IsSet())
    {
      cout << "Error: --main::instance filename option must always be set" << endl;
      return 1;
    }

  Prod_Input in(instance); 
  Prod_Output out(in); 
  if (seed.IsSet())
    Random::SetSeed(seed);
  
  // cost components: second parameter is the cost, third is the type (true -> hard, false -> soft)
  Delay cc1(in, 1, true);
  Advance cc2(in, 1, true);
  LineOverLoad cc3 (in, 2, true); 

  LinesToAOrder cc4 (in, 1, false);
  CategoryDeviation cc5 (in, 1, false); 

  Prod_MoveChangeDeltaDelay dcc1(in, cc1);
  Prod_MoveChangeDeltaAdvance dcc2(in, cc2);
  Prod_MoveChangeDeltaLineOverLoad dcc3(in, cc3);
  Prod_MoveChangeDeltaLinesToAOrder dcc4(in, cc4);
  Prod_MoveChangeDeltaCategoryDeviation dcc5(in, cc5);

  // helpers
  Prod_SolutionManager Prod_sm(in);
  Prod_MoveChangeNeighborhoodExplorer Prod_nhe(in, Prod_sm);
  
  Prod_sm.AddCostComponent(cc1);
  Prod_sm.AddCostComponent(cc2);
  Prod_sm.AddCostComponent(cc3);
  Prod_sm.AddCostComponent(cc4);
  Prod_sm.AddCostComponent(cc5);
    
  Prod_nhe.AddDeltaCostComponent(dcc1);
  Prod_nhe.AddDeltaCostComponent(dcc2);
  Prod_nhe.AddDeltaCostComponent(dcc3);
  Prod_nhe.AddDeltaCostComponent(dcc4);
  Prod_nhe.AddDeltaCostComponent(dcc5);
  
  
  // runners
  HillClimbing<Prod_Input, Prod_Output, Prod_MoveChange> Prod_hc(in, Prod_sm, Prod_nhe, "HC");
  SteepestDescent<Prod_Input, Prod_Output, Prod_MoveChange> Prod_sd(in, Prod_sm, Prod_nhe, "SD");
  TabuSearch<Prod_Input, Prod_Output, Prod_MoveChange> Prod_ts(in, Prod_sm, Prod_nhe, "TS", 
                                                   [](const Prod_MoveChange& m1, const Prod_MoveChange& m2)->bool
                                                   { return m1.order == m2.order && m1.operation == m2.operation && m1.old_d == m2.new_d && m1.old_l == m2.new_l;}); 

  SimulatedAnnealing<Prod_Input, Prod_Output, Prod_MoveChange> Prod_sa(in, Prod_sm, Prod_nhe, "SA");

  // tester
  Tester<Prod_Input, Prod_Output> tester(in,Prod_sm);

  MoveTester<Prod_Input, Prod_Output, Prod_MoveChange> move_test(in,Prod_sm,Prod_nhe, "Prod_MoveChange move", tester);

  SimpleLocalSearch<Prod_Input, Prod_Output> Prod_solver(in, Prod_sm, "Prod solver");
  
  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;

  if (!method.IsSet())
    {
      if (init_state.IsSet())		  
        tester.RunMainMenu(init_state);
      else
        tester.RunMainMenu();
    }
  else
    {
      if (method == string("SA"))
        Prod_solver.SetRunner(Prod_sa);
      else if (method == string("TS"))
        Prod_solver.SetRunner(Prod_ts);
      else if (method == string("HC"))
        Prod_solver.SetRunner(Prod_hc);
      else if (method == string("SD"))
        Prod_solver.SetRunner(Prod_sd);
      else
        {
          cerr << "unrecognized method " << string(method) << endl;
          exit(1);
        }

      // without using auto 

      SolverResult<Prod_Input, Prod_Output> result = Prod_solver.Solve();
      out = result.output;
      if (output_file.IsSet())
        {
          ofstream os(static_cast<string>(output_file));
          os << out << endl;
          os << "Cost: " << result.cost.objective << endl;
          os << "Violations: " << result.cost.violations << endl;
          os << "Time: " << result.running_time << endl;
          os.close();
        }
      else
        {
          cout << out << endl;
          cout << "Cost: " << result.cost.objective << endl;
          cout << "Violations: " << result.cost.violations << endl;
          cout << "Time: " << result.running_time << endl;
        }
    }
  return 0;
}
