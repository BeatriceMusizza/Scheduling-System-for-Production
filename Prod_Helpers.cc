// File Prod_Helpers.cc
#include "Prod_Helpers.hh"


Prod_SolutionManager::Prod_SolutionManager(const Prod_Input & pin) 
  : SolutionManager<Prod_Input,Prod_Output>(pin, "ProdSolutionManager")  {} 

void Prod_SolutionManager::RandomState(Prod_Output& out) 
{
  int o, p, l,d;
  out.Reset();
  
  for (o = 0; o < static_cast<int>(in.Orders()); o++)
  {
	  for (p = 0; p < static_cast<int>(in.NumberOfOperations(o)); p++)
		{
		  do //for each operation, randomly choose a day
            d = Random::Uniform<unsigned>(0,in.SchedulingDays()-1);
          while (out(o,d) != -1); // cycle until the day is free
		  l = Random::Uniform<int>(0, in.Lines() - 1); //randomly choose a line
		  out.Assign(o,p,-1,d,-1,l); 
    }
  }
} 


void Prod_SolutionManager::GreedyState(Prod_Output& out) 
{
  int l;
  unsigned min, d;
  vector<unsigned> ordered_orders(in.Orders()); //vector of index, ordered by end dates ,the opr will be assigned following this order 
  out.Reset();

  iota(ordered_orders.begin(), ordered_orders.end(), 0); //vector of index 
    
  for (unsigned i=0; i<in.Orders()-1; i++)
  {
    min=i;
    for(unsigned j = i+1; j < in.Orders(); j++)
    {
      if(in.MaxEndDates(ordered_orders[j]) < in.MaxEndDates(ordered_orders[min]))
          min=j;
    }
	  unsigned temp = ordered_orders[min];
    ordered_orders[min] = ordered_orders[i];
    ordered_orders[i] = temp; 
  }

  //assign the operations in the first day in which there is at least a line which is free (Not OverLoaded)
  //if there is no combination day-line free, violations will be accepted

  bool assigned_with_overload;
  for (unsigned o = 0; o < in.Orders(); o++)
  {
    d = in.MinStartDates(ordered_orders[o]);
    l = 0;
	  for (unsigned p = 0; p < in.NumberOfOperations(ordered_orders[o]); p++)
		{
      assigned_with_overload = false;
      while ((out.LineLoad(l,d) != 0 || out(ordered_orders[o],d) != -1) && (!assigned_with_overload)) //searching for a free line
      {
        l++; 
        if (l == static_cast<int>(in.Lines())) //if lines are finished, day will be increase
        {
          l = 0;
          d++;
          if (d == in.SchedulingDays()) //if day are finished, violations will be accepted
          {
              d = 0;
              while (out(ordered_orders[o],d) != -1)
              {
               d++;
              }
              //the line will be decided searching the one that in that day has the lowest number of opr
              int min_load = -1; 
              l = -1; 
              for (int i = 0; i < static_cast<int>(in.Lines()); ++i) 
              {
                if (static_cast<int>(out.LineLoad(i,d)) < min_load || min_load == -1)
                {
                  min_load = static_cast<int>(out.LineLoad(i,d));
                  l = i;
                }
              }
              assigned_with_overload = true;
          }
        }
      }
      out.Assign(ordered_orders[o], p, -1, d, -1, l);
    }
  }
}

bool Prod_SolutionManager::CheckConsistency(const Prod_Output& out) const //CHIARAs VERSION
{
	unsigned o, d, p, l, count, sum;
	bool found; 
  vector<unsigned> temp_line(in.Lines()); //to check LineLoead
  vector<unsigned> temp_line_order(in.Lines()); //to check OperationPerOrderToLines
  vector<unsigned> temp_opr_category(in.Categories());  //to check OperationPerCategoryToLines
  vector<int> temp_min_cat(in.Lines(), -1); //to check MinCategory
  vector<int> temp_max_cat(in.Lines(), -1); //to check MaxCategory
  vector<int> temp_second_min_cat(in.Lines(), -1); //to check SecondMinCategory
  vector<int> temp_second_max_cat(in.Lines(), -1); //to check SecondMinCategory
	for (o = 0; o < in.Orders(); o++)
	{
    count = 0;
    sum = 0;
    fill(temp_line_order.begin(), temp_line_order.end(), 0); //to reset 

	  for (d = 0; d < in.SchedulingDays(); d++)
		{
      found=false; 
			if(out(o,d) != -1)
			{
        count++; //to check NumberOfOperations later
				temp_line_order[out(o,d)]++; 

				for(p = 0; p < in.NumberOfOperations(o); p++) 
        {
					if(out.OperationDay(o,p) == static_cast<int>(d)) //to check OperationDay
					{
						found = true; //a match has been found between the day saved in OperationDay and the output
						break;
					}
        }
				if(found == false) //to check OperationDay
					{
            cerr<<"OperationDay is not consistent"<<endl;
            return false; //no correspondence has been found between the day saved in OperationDay and the output
          }
			}	
		}
    for (l = 0; l < in.Lines(); l++)
    {
      if(out.OperationPerOrderToLines(o,l) > 0) //+1 for each order (not operation)
        sum ++; //to check NumberOfLinesForAOrder later

      if(temp_line_order[l] != out.OperationPerOrderToLines(o,l)) //to check OperationPerOrderToLines
        {
          cerr<<"OperationPerOrderToLines is not consistent"<<endl;
          return false;
        }
    
      
      if(out.OperationPerOrderToLines(o,l) != 0) //to create vectors to check MinCategory, MaxCategory, SecondMinCategory and SecondMaxCategory later
      { 
        
        if(static_cast<int>(in.Category(o)) < temp_min_cat[l] || temp_min_cat[l] == -1)
        {
          temp_second_min_cat[l] = temp_min_cat[l];
          temp_min_cat[l] = static_cast<int>(in.Category(o));
        }        
        else if((static_cast<int>(in.Category(o)) < temp_second_min_cat[l] || temp_second_min_cat[l] == -1) && static_cast<int>(in.Category(o)) != temp_min_cat[l])
          temp_second_min_cat[l] = static_cast<int>(in.Category(o));

        if(static_cast<int>(in.Category(o)) > temp_max_cat[l] || temp_max_cat[l] == -1)
        {
          temp_second_max_cat[l] = temp_max_cat[l];
          temp_max_cat[l] = static_cast<int>(in.Category(o));
        }
        else if((static_cast<int>(in.Category(o)) > temp_second_max_cat[l] || temp_second_max_cat[l] == -1) && static_cast<int>(in.Category(o)) != temp_max_cat[l])
          temp_second_max_cat[l] = static_cast<int>(in.Category(o));
      }
    }
   
    if(count != in.NumberOfOperations(o)) //check the number of operations of that order
    {
      cerr<<"NumberOfOperations is not consistent"<<endl;
      return false;
    }
     
    if(sum != out.NumberOfLinesForAOrder(o)) //to check NumberOfLinesForAOrder
    {
      cerr<<"NumberOfLinesForAOrder is not consistent"<<endl;
      return false;
    }
  }
  
  for (d = 0; d < in.SchedulingDays(); d++)
	{
    fill(temp_line.begin(), temp_line.end(), 0); //to reset 
	  for (o = 0; o < in.Orders(); o++)
		{
			if(out(o,d) != -1)
			{
        temp_line[out(o,d)]++; //to check LineLoead later
      }
    } 

    for (l = 0; l < in.Lines(); l++) 
    {
      if(out.LineLoad(l,d) != temp_line[l])//to check LineLoead
      {
        cerr<<"LineLoad is not consistent"<<endl;
        return false;
      } 
    }
  }
  
  for (l= 0; l< in.Lines(); l++) //to check MinCategory, MaxCategory, SecondMinCategory and SecondMaxCategory
  {
    if(out.MinCategory(l) != temp_min_cat[l] || out.SecondMinCategory(l) != temp_second_min_cat[l] 
      || out.MaxCategory(l) != temp_max_cat[l] || out.SecondMaxCategory(l) != temp_second_max_cat[l])
    {
      cerr<<"Category is not consistent"<<endl;
      return false;
    }

    fill(temp_opr_category.begin(), temp_opr_category.end(), 0); //to reset 

    for (o = 0; o < in.Orders(); o++) //to check OperationPerCategoryToLines later
      temp_opr_category[in.Category(o)] += out.OperationPerOrderToLines(o,l);

    for (unsigned c = 0; c < in.Categories(); c++) //to check OperationPerCategoryToLines
    {
      if(out.OperationPerCategoryToLines(l,c) != temp_opr_category[c])
      {
        cerr<<"OperationPerCategoryToLines is not consistent"<<endl;
        return false;
      }
    }
      
  }
  return true;
}


int Delay::ComputeCost(const Prod_Output& out) const 
{
  unsigned cost = 0;
  for(unsigned o = 0; o < in.Orders(); o++) //the total delay is the sum of the total delay of the order
  {
    for (unsigned p = 0; p < in.NumberOfOperations(o); p++) //the total delay of the order is the sum of the delay of each opr
    {
      if(out.OperationDay(o,p) > static_cast<int>(in.MaxEndDates(o)))
        cost+= out.OperationDay(o,p) - in.MaxEndDates(o); //the delay of the single opr is the differnce  
    }
  }
  return cost;
}
          
void Delay::PrintViolations(const Prod_Output& out, ostream& os) const
{
  unsigned delay = 0;
  for(unsigned o = 0; o < in.Orders(); o++)
  {
    for (unsigned p = 0; p < in.NumberOfOperations(o); p++)
    {
      if(out.OperationDay(o,p) > static_cast<int>(in.MaxEndDates(o)))
      {
        delay = out.OperationDay(o,p) - in.MaxEndDates(o);
        os << "The operation number " << p << " of the order number " 
                 <<  o << " has a delay of " 
                 << delay << " days." << endl;
      }
    }
  }
}

int Advance::ComputeCost(const Prod_Output& out) const //same logic of delay
{
  unsigned cost = 0;
  for(unsigned o = 0; o < in.Orders(); o++)
  {
    for (unsigned p = 0; p < in.NumberOfOperations(o); p++)
    {
      if(out.OperationDay(o,p) < static_cast<int>(in.MinStartDates(o)))
        cost+= in.MinStartDates(o) - out.OperationDay(o,p);
    }
  }
  return cost;
}

void Advance::PrintViolations(const Prod_Output& out, ostream& os) const
{
  unsigned advance = 0;
  for(unsigned o = 0; o < in.Orders(); o++)
  {
    for (unsigned p = 0; p < in.NumberOfOperations(o); p++)
    {
      if(out.OperationDay(o,p) < static_cast<int>(in.MinStartDates(o)))
      {
        advance = in.MinStartDates(o) - out.OperationDay(o,p);
        os << "The operation number " << p << " of the order number " 
                 <<  o << " is " 
                 << advance << " days in advance." << endl;
      }
    }
  }
}

int LineOverLoad::ComputeCost(const Prod_Output& out) const
{
  unsigned cost = 0;
  for(unsigned l = 0; l < in.Lines(); l++) //the total cost is the sum of the total cost for each line
  {
    for (unsigned d = 0; d < in.SchedulingDays(); d++)
    {
      if(out.LineLoad(l,d) > 1)
        cost += (out.LineLoad(l,d) - 1); //the total cost for each line is the number of over-operations in each day
    }
  }
  return cost;
}

void LineOverLoad::PrintViolations(const Prod_Output& out, ostream& os ) const
{
  unsigned overload = 0;
  for(unsigned l = 0; l < in.Lines(); l++)
  {
    for (unsigned d = 0; d < in.SchedulingDays(); d++)
    {
      if( out.LineLoad(l,d) > 1)
      {
      overload = (out.LineLoad(l,d) - 1); 
      os << "The Line number " << l << " in the day number " 
                 <<  d << " has " 
                 << overload << " over-operations." << endl;
      }
    }
  }
}

int LinesToAOrder::ComputeCost(const Prod_Output& out) const //the number of lines in excess that make an order, the cost will be 0 if the order is assigned to only 1 line
{
  unsigned cost = 0;
  for(unsigned o = 0; o < in.Orders(); o++)
  {
    cost += (out.NumberOfLinesForAOrder(o)-1); //orders with no operation are not allowed
  }
  return cost;
}

void LinesToAOrder::PrintViolations(const Prod_Output& out, ostream& os) const 
{
  for(unsigned o = 0; o < in.Orders(); o++)
  {
     if (out.NumberOfLinesForAOrder (o)>1)
      os << "The Order number " << o << " is assigned to " 
                 <<  out.NumberOfLinesForAOrder (o) << " lines."  << endl;
  }
}

int CategoryDeviation::ComputeCost(const Prod_Output& out) const //the distance beetween Categories
{
  unsigned cost = 0;
   for (unsigned l = 0; l < in.Lines(); l++)
   { 
     if(out.MaxCategory(l)!= -1 ) //just check one (min or max); it's a simple check that the line has at least one order (min and max must have the same value and are both not equal to -1)
      cost+= (out.MaxCategory(l) - out.MinCategory(l)); 
   }
return cost;
}

void CategoryDeviation::PrintViolations(const Prod_Output& out, ostream& os) const 
{
  unsigned deviation = 0;
   for (unsigned l = 0; l < in.Lines(); l++)
   {
    deviation = (out.MaxCategory(l) - out.MinCategory(l));
    os << "The Line number " << l << " has a difference beetween Categories of " 
    <<  deviation << " Categories."<< endl;
   }
}

/*****************************************************************************
 * Prod_MoveChange Neighborhood Explorer Methods
 *****************************************************************************/

Prod_MoveChange::Prod_MoveChange() 
{
  order= -1;
  operation = -1;
  old_l= -1;
  new_l= -1;
  old_d= -1;
  new_d= -1;
}

bool operator==(const Prod_MoveChange& mv1, const Prod_MoveChange& mv2)  
{
  return mv1.order == mv2.order && mv1.operation == mv2.operation && mv1.new_d == mv2.new_d && mv1.new_l == mv2.new_l && mv1.old_d == mv2.old_d && mv1.old_l == mv2.old_l ;
}

bool operator!=(const Prod_MoveChange& mv1, const Prod_MoveChange& mv2)
{
  return mv1.order!= mv2.order || mv1.operation != mv2.operation ||  mv1.new_d != mv2.new_d || mv1.new_l != mv2.new_l || mv1.old_d != mv2.old_d || mv1.old_l != mv2.old_l;
}

bool operator<(const Prod_MoveChange& mv1, const Prod_MoveChange& mv2) //the hierarcy is order, opr, new_d, new_l, old_d, new_d
{
  return (mv1.order < mv2.order)
  || (mv1.order == mv2.order && mv1.operation < mv2.operation)
  || (mv1.order == mv2.order && mv1.operation == mv2.operation && mv1.new_d < mv2.new_d)
  || (mv1.order == mv2.order && mv1.operation == mv2.operation && mv1.new_d == mv2.new_d && mv1.new_l < mv2.new_l)
  || (mv1.order == mv2.order && mv1.operation == mv2.operation && mv1.new_d == mv2.new_d && mv1.new_l == mv2.new_l && mv1.old_d < mv2.old_d)
  || (mv1.order == mv2.order && mv1.operation == mv2.operation && mv1.new_d == mv2.new_d && mv1.new_l == mv2.new_l && mv1.old_d == mv2.old_d && mv1.old_l < mv2.old_l);
}

istream& operator>>(istream& is, Prod_MoveChange& mv) //format: order, operation: old_l -> new_l, old_d -> new_d 
{
  char ch;
  is >> mv.order >> ch >> mv.operation >> ch >> mv.old_l >> ch >> ch >> mv.new_l >> ch >> mv.old_d >> ch >> ch >>mv.new_d; 
  return is; 
}

ostream& operator<<(ostream& os, const Prod_MoveChange& mv)
{
  os <<"Move Order: "<< mv.order << " Operation: "<<mv.operation<<" Line change: " << mv.old_l << " -> " << mv.new_l << ", Day change: " << mv.old_d<< " -> " << mv.new_d << endl;;
  return os;
}

void Prod_MoveChangeNeighborhoodExplorer::RandomMove(const Prod_Output& out, Prod_MoveChange& mv) const 
{
  
	int operation = Random::Uniform<int>(0,in.TotalOperations()-1); //randomly choose an opr from the total number, to not prefer the opr of orders with less oprs
	mv.order = in.OrderOfTheOperation(operation); //find out to which order the opr belongs
  mv.operation = in.OperationPositionInTheOrder(operation); //find out the position of the opr in its order
	mv.old_d = out.OperationDay(mv.order,in.OperationPositionInTheOrder(operation)); 
	mv.old_l = out(mv.order,mv.old_d);

	do
	{
		mv.new_d = Random::Uniform<int>(0, in.SchedulingDays()-1);
		mv.new_l = Random::Uniform<int>(0, in.Lines()-1);
	}while(!FeasibleMove(out,mv));
} 

void Prod_MoveChangeNeighborhoodExplorer::FirstMove(const Prod_Output& out, Prod_MoveChange& mv) const 
{
	mv.order = 0;
	mv.operation = 0;
	mv.old_d = out.OperationDay(mv.order,mv.operation) ;
	mv.old_l = out(mv.order,mv.old_d);
	mv.new_d = 0;
	mv.new_l = 0;

	while(!FeasibleMove(out,mv))
  {
    if(!AnyNextMove(out,mv))
      throw runtime_error("Empty neighborhood exception");
  }
}

bool Prod_MoveChangeNeighborhoodExplorer::NextMove(const Prod_Output& out, Prod_MoveChange& mv) const
{
  do
    if (!AnyNextMove(out,mv))
    {
      return false;
    }
  while (!FeasibleMove(out,mv));
  return true;
}

bool Prod_MoveChangeNeighborhoodExplorer::AnyNextMove(const Prod_Output& out, Prod_MoveChange& mv) const 
{ 
  if(mv.new_l < static_cast<int>(in.Lines()-1)) //first, check if it's possible to increase the line
  {
    mv.old_d = out.OperationDay(mv.order,mv.operation);
    mv.old_l = out(mv.order,mv.old_d);
    mv.new_l++; 
  }
  else if (mv.new_d < static_cast<int>(in.SchedulingDays()-1)) //second, check if it's possible to increase the day
  {
      mv.old_d = out.OperationDay(mv.order,mv.operation);
      mv.old_l = out(mv.order,mv.old_d); 
      mv.new_l = 0;
      mv.new_d++;
  }
  else if(mv.operation < static_cast<int>(in.NumberOfOperations(mv.order)-1)) //then, check if it's possible to increase the opr
  {
      mv.operation++;
      mv.old_d = out.OperationDay(mv.order,mv.operation); 
      mv.old_l = out(mv.order,mv.old_d); 
      mv.new_l = 0;
      mv.new_d = 0;
  }
  else 
  {
    if (mv.order == static_cast<int>(in.Orders()-1)) //finally, check if it's possible to increase the order
      return false;
    mv.order++;
    mv.operation=0;
    mv.old_d = out.OperationDay(mv.order,mv.operation); 
    mv.old_l = out(mv.order,mv.old_d); 
    mv.new_l = 0;
    mv.new_d = 0; 
  }
  return true;
}

bool Prod_MoveChangeNeighborhoodExplorer::FeasibleMove(const Prod_Output& out, const Prod_MoveChange& mv) const 
{
	return (mv.new_l < static_cast<int>(in.Lines())) && (mv.new_d < static_cast<int>(in.SchedulingDays())) && (mv.order < static_cast<int>(in.Orders()))
  && (mv.new_l != mv.old_l || mv.new_d != mv.old_d ) 
  && (mv.new_l != -1) && (out(mv.order, mv.new_d) == -1 || mv.old_d == mv.new_d);
} 

void Prod_MoveChangeNeighborhoodExplorer::MakeMove(Prod_Output& out, const Prod_MoveChange& mv) const 
{
  out.Assign(mv.order, mv.operation, mv.old_d, mv.new_d, mv.old_l, mv.new_l);
  return;
}   

int Prod_MoveChangeDeltaDelay::ComputeDeltaCost(const Prod_Output& out, const Prod_MoveChange& mv) const 
{
  unsigned cost = 0;
  if(out.OperationDay(mv.order,mv.operation) > static_cast<int>(in.MaxEndDates(mv.order))) //if it was late
    cost-= out.OperationDay(mv.order,mv.operation) - in.MaxEndDates(mv.order); //the cost decreases
  if(mv.new_d > static_cast<int>(in.MaxEndDates(mv.order))) //if it it late
    cost+= mv.new_d - in.MaxEndDates(mv.order); //the cost increases
  return cost;
}

int Prod_MoveChangeDeltaAdvance::ComputeDeltaCost(const Prod_Output& out, const Prod_MoveChange& mv) const 
{
  unsigned cost = 0;
  if(out.OperationDay(mv.order,mv.operation)< static_cast<int>(in.MinStartDates(mv.order))) //if it was in advance
    cost-= in.MinStartDates(mv.order) - out.OperationDay(mv.order,mv.operation); //the cost decreases
  if(mv.new_d < static_cast<int>(in.MinStartDates(mv.order))) //if it it late
    cost+= in.MinStartDates(mv.order) - mv.new_d; //the cost increases
  return cost;
}

int Prod_MoveChangeDeltaLineOverLoad::ComputeDeltaCost(const Prod_Output& out, const Prod_MoveChange& mv) const 
{
  unsigned cost = 0;
  if(out.LineLoad(mv.old_l,mv.old_d) > 1) //if the old line was overloaded
    cost-= 1; //the cost decreases
  if(out.LineLoad(mv.new_l,mv.new_d) > 0) //if the new line already has at least an opr
    cost++; //the cost increases
  return cost;
}

int Prod_MoveChangeDeltaLinesToAOrder::ComputeDeltaCost(const Prod_Output& out, const Prod_MoveChange& mv) const 
{
  unsigned cost=0;
  if( out.OperationPerOrderToLines(mv.order, mv.old_l) == 1 && mv.new_l != mv.old_l) //the old line would have no more opr of that order
    cost--;
  if(out.OperationPerOrderToLines(mv.order, mv.new_l) == 0 ) //if the new line has no opr of that order, one more line would be used
    cost++;
  return cost; 
}

int Prod_MoveChangeDeltaCategoryDeviation::ComputeDeltaCost(const Prod_Output& out, const Prod_MoveChange& mv) const 
{
  int cost=0 ;
  if(mv.old_l == mv.new_l) 
   return cost;

  //old line
  if (out.OperationPerCategoryToLines(mv.old_l,in.Category(mv.order)) == 1 && static_cast<int>(in.Category(mv.order)) == out.MinCategory(mv.old_l) //if the opr unassigned was the only one of that category, the category was the min
   && out.SecondMinCategory(mv.old_l) != -1) //and the old line has more than one opr (in this case the deviation was 0 and will be 0)
    cost -= out.SecondMinCategory(mv.old_l) - out.MinCategory(mv.old_l);

  if (out.OperationPerCategoryToLines(mv.old_l,in.Category(mv.order)) == 1 && static_cast<int>(in.Category(mv.order)) == out.MaxCategory(mv.old_l) //if the opr unassigned was the only one of that category, the category was the max
   && out.SecondMaxCategory(mv.old_l) !=-1) //and the old line has more than one opr (in this case the deviation was 0 and will be 0)
    cost -= out.MaxCategory(mv.old_l) - out.SecondMaxCategory(mv.old_l); 

  //new line
  if (static_cast<int>(in.Category(mv.order)) < out.MinCategory(mv.new_l))// if the category of the order that its assigned, is lower that the mincategory of the new line
    cost += out.MinCategory(mv.new_l) - in.Category(mv.order);//the deviation will increase 

  if (static_cast<int>(in.Category(mv.order)) > out.MaxCategory(mv.new_l) && out.MaxCategory(mv.new_l) != -1)//if the category of the order that its assigned, is higher that the maxcategory of the new line
    cost += in.Category(mv.order) - out.MaxCategory(mv.new_l);//the deviation will increase 
  
  return cost;
}
