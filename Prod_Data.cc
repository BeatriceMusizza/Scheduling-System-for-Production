// File Prod_Data.cc
#include "Prod_Data.hh"
#include <fstream>
#include <algorithm>

//INPUT
Prod_Input::Prod_Input(string file_name) 
{  
  const unsigned MAX_DIM = 100;
  int o;
  char ch, buffer[MAX_DIM];
  unsigned p,t_operations = 0;

  ifstream is(file_name);
  if(!is)
  {
    cerr << "Cannot open input file " <<  file_name << endl;
    exit(1);
  }
  
  is >> buffer >> ch >> lines >> ch;
  is >> buffer >> ch >> orders >> ch;
  is >> buffer >> ch >> categories >> ch;
  
  
  min_start_dates.resize(orders);
  max_end_dates.resize(orders);
  number_of_operations.resize(orders);
  category.resize(orders);

  
  // read Min StartDays
  is.ignore(MAX_DIM,'['); // read "... MinStartDays = ["
  for (o = 0; o < static_cast<int>(orders); o++)
    is >> min_start_dates[o] >> ch;
  
  // read Max EndDays
  is.ignore(MAX_DIM,'['); // read "... MaxEndDays = ["
  for (o = 0; o < static_cast<int>(orders); o++)
    is >> max_end_dates[o] >> ch;

  // read NumberOfOperations and calculate the total number of operations
  is.ignore(MAX_DIM,'['); // read "...NumberOfOperations = ["
  for (o = 0; o < static_cast<int>(orders); o++)
  {
    is >> number_of_operations[o] >> ch;
    t_operations += number_of_operations[o];
    for (p = 0; p < static_cast<unsigned>(number_of_operations[o]); p++)
        orders_operations.push_back(make_pair(o,p));
  }

  totaloperations = t_operations;

  // read Orders Specialisation
  is.ignore(MAX_DIM,'['); // read "... OrdersSpecialisation = ["
  for (o = 0; o <static_cast<int>(orders); o++)
  {
    is >> category[o] >> ch;
  }
  schedulingdays= (*max_element(max_end_dates.begin(), max_end_dates.end())) + 1; // total number of scheduling days

  is >> ch >> ch;
}

ostream& operator<<(ostream& os, const Prod_Input& in)
{
  unsigned o;
  os << "Lines = " << in.lines << ";" << endl;
  os << "Orders = " << in.orders << ";" << endl;
  os << "Cateogries = " << in.categories << ";" << endl;
  os << endl;
  
  os << "Min Start SchedulingDays = [";
  for (o = 0; o < in.orders; o++)
    {
      os << in.min_start_dates[o];
      if (o < in.orders - 1)
        os << ", ";
      else
        os << "];" << endl;
    }
  
  os << "Max End SchedulingDays = [";
  for (o = 0; o < in.orders; o++)
    {
      os << in.max_end_dates[o];
      if (o < in.orders - 1)
        os << ", ";
      else
        os << "];" << endl;
    }
  
  os << "Orders NumberOfOperations = [";
  for (o = 0; o < in.orders; o++)
    {
      os << in.number_of_operations[o];
      if (o < in.orders - 1)
        os << ", ";
      else
        os << "];" << endl;
    }

    os << "Orders Specilisation = [";
  for (o = 0; o < in.orders; o++)
    {
      os << in.category[o];
      if (o < in.orders - 1)
        os << ", ";
      else
        os << "];" << endl;
    }
  return os;
}


//OUTPUT

Prod_Output::Prod_Output(const Prod_Input& in) 

  : in(in), assign(in.Orders(), vector<int>(in.SchedulingDays(),-1)),
  line_load(in.Lines(),vector<unsigned>(in.SchedulingDays(),0)),
  operation_day(in.Orders()),
  number_of_opr_per_order_to_lines(in.Orders(), vector<unsigned>(in.Lines(),0)),
  category_deviation(in.Lines(),vector<int>( 2,-1)),
  second_category_deviation(in.Lines(), vector<int>( 2,-1)),
  number_of_opr_per_category_to_lines (in.Lines(), vector<unsigned>(in.Categories(),0)),
  number_of_lines_for_a_order (in.Orders(),0)
{ 

  for (unsigned o = 0; o < in.Orders(); o++)
    {
      operation_day[o].resize(in.NumberOfOperations(o));
    }
  
  for (unsigned o = 0; o < in.Orders(); o++)
  {
    for(unsigned p = 0; p < in.NumberOfOperations(o); p++)
    {
      operation_day[o][p]=-1;
    }
  }
}


Prod_Output& Prod_Output::operator= (const Prod_Output& out) 
{
  assign = out.assign;
  line_load = out.line_load; 
  operation_day = out.operation_day;
  number_of_opr_per_order_to_lines = out.number_of_opr_per_order_to_lines; 
  number_of_opr_per_category_to_lines = out. number_of_opr_per_category_to_lines; 
  category_deviation = out.category_deviation;
  second_category_deviation = out.second_category_deviation;
  number_of_lines_for_a_order = out.number_of_lines_for_a_order;
  return *this;
}

void Prod_Output::Assign(int order, int p, int old_d, int new_d, int old_l, int new_l)
{	
 if( old_l!=-1 && old_d!=-1) //this is not the first time this operation is assigned; it would be enough to look at one of the two: if one is -1, the other one is too
  {
    line_load[old_l][old_d]--;
    number_of_opr_per_order_to_lines[order][old_l]--;
    number_of_opr_per_category_to_lines[old_l][in.Category(order)]--;
    assign[order][old_d] = -1;
    if (number_of_opr_per_order_to_lines [order][old_l] == 0) //is the last operation of that order that is performed in old_l
    {
      number_of_lines_for_a_order[order]--;
    }
  }
  
 if (number_of_opr_per_order_to_lines [order][new_l] == 0) //is the first operation of that order that is performed in new_l
  {
    number_of_lines_for_a_order[order]++;
  }

  line_load[new_l][new_d]++;
  number_of_opr_per_order_to_lines[order][new_l]++;
  number_of_opr_per_category_to_lines[new_l][in.Category(order)]++;
  operation_day[order][p] = new_d;
  assign[order][new_d] = new_l;
  
  //update min category for the new_l
 if (static_cast<int>(in.Category(order)) < category_deviation[new_l][0] || category_deviation[new_l][0] == -1) //a category smaller than the saved min has been found or the first min has not been initialized
  {
    second_category_deviation[new_l][0] = category_deviation[new_l][0]; //the second min becomes equal to the first min (scaled)
    category_deviation[new_l][0] = static_cast<int>(in.Category(order)); //the first min becomes equal to the newly found category
  }
 else if ((static_cast<int>(in.Category(order)) < second_category_deviation[new_l][0] || second_category_deviation[new_l][0]== -1) && (static_cast<int>(in.Category(order)) != category_deviation[new_l][0]))
  //a category smaller than the second saved min but greater than the first min has been found, or the first min has not been initialized In both cases, the found category must be different from that of the first min
  {
    second_category_deviation[new_l][0] = static_cast<int>(in.Category(order));
  }

  //update max category for the new_l
 if (static_cast<int>(in.Category(order)) > category_deviation[new_l][1] || category_deviation[new_l][1] == -1) //a category grater than the saved maximum has been found or the first maximum has not been initialized
  {
    second_category_deviation[new_l][1] = category_deviation[new_l][1]; //first max is scaled
    category_deviation[new_l][1] = static_cast<int>(in.Category(order));
  }
 else if ((static_cast<int>(in.Category(order)) > second_category_deviation[new_l][1] || second_category_deviation[new_l][1] == -1) && (static_cast<int>(in.Category(order)) != category_deviation[new_l][1]) )
  //a category larger than the second saved max but smaller than the first max has been found, or the first max has not been initialized. In both cases, the found category must be different from that of the first max
  {
    second_category_deviation[new_l][1] = static_cast<int>(in.Category(order));
  }

 //update for the old_l
  bool mininitalised=false;
  bool maxinitalised=false;
  bool secondmininit=false;
  bool secondmaxinit=false;

 if (old_l != -1 && number_of_opr_per_category_to_lines[old_l][in.Category(order)]==0) //it is not the first initialization, and it is the last operation of that order carried out by the old_l
  {
   //MIN old_l update
    if (static_cast<int>(in.Category(order)) == category_deviation[old_l][0]) //if the first min was the one of the order that we are unassigning
    {
      for(int c = 0; c < static_cast<int>(in.Categories()); c++)
      {
        if( number_of_opr_per_category_to_lines[old_l][c] > 0 && !mininitalised) 
        {
          category_deviation[old_l][0] = c; //the new first min has been found
          mininitalised=true;
        }
        else if( number_of_opr_per_category_to_lines[old_l][c] > 0 && mininitalised) //to find new second min
        {
          second_category_deviation[old_l][0] = c;
          secondmininit=true;
          break;
        }
      }
      if(!mininitalised) //the new first min has not been found because the old_l was only performing the operation that we are unassigning from it
      {
          category_deviation[old_l][0] = -1; //the line has no order anymore
      }
      else if (!secondmininit) //the line has only orders of one category; the first min has been found, but not the second
      {
       second_category_deviation[old_l][0] = -1;
      }
      
    }
    else if (static_cast<int>(in.Category(order)) == second_category_deviation[old_l][0]) //if the second min was the one of the order that we are unassigning
    {
      for(int c = 0; c < static_cast<int>(in.Categories()); c++)
      {   
        if(( number_of_opr_per_category_to_lines[old_l][c] > 0 && c != category_deviation[old_l][0]) && !secondmininit)   
        {
          second_category_deviation[old_l][0] = c;
          secondmininit=true;
        }
      }
      if (!secondmininit)
      {
        second_category_deviation[old_l][0] = -1;
      }
    }


   //MAX old_l update
    if (static_cast<int>(in.Category(order)) == category_deviation[old_l][1]) //if the first max was the one of the order that we are unassigning
    {
      for(int c = static_cast<int>(in.Categories()-1); c >= 0; c--)
      {
        if(number_of_opr_per_category_to_lines[old_l][c] > 0 && !maxinitalised)
        {
          category_deviation[old_l][1] = c; //the new first max has been found
          maxinitalised=true;
        }
        else if( number_of_opr_per_category_to_lines[old_l][c] > 0 && maxinitalised) //to find new second max
        {
          second_category_deviation[old_l][1] = c;
          secondmaxinit=true;
          break;
        }
      }
      if(!maxinitalised) //the new first max has not been found because the old_l was only performing the operation that we are unassigning from it
      {
        category_deviation[old_l][1] = -1; //the old_l has no order anymore
      }
      else if(!secondmaxinit) //the line has only orders of one category; the first max has been found, but not the second
      {
        second_category_deviation[old_l][1] = -1;
      }
    }
    else if (static_cast<int>(in.Category(order)) == second_category_deviation[old_l][1]) //if the second max was the one of the order that i am unassigning
    {
        for(int c = static_cast<int>(in.Categories()-1); c >= 0; c--)
      {
        if ((number_of_opr_per_category_to_lines[old_l][c] > 0 && c!= category_deviation[old_l][1]) && !secondmaxinit)   
        {
          second_category_deviation[old_l][1] = c;
          secondmaxinit=true;
        }
      }
      if  (!secondmaxinit)
      {
        second_category_deviation[old_l][1] = -1;
      }
    }
  }
}

void Prod_Output::Reset()
{
  int o, d, p, l, c;
  for(o = 0; o < static_cast<int>(in.Orders()); o++)
  {
	  for(d = 0; d < static_cast<int>(in.SchedulingDays()); d++)
		  assign[o][d] = -1;
    for (p = 0; p < static_cast<int>(in.NumberOfOperations(o)); p++)
      operation_day[o][p]=-1;
    for (l = 0; l < static_cast<int>(in.Lines()); l++)
      number_of_opr_per_order_to_lines[o][l]=0;
  }   

  for(l = 0; l < static_cast<int>(in.Lines()); l++)
  {
	  for(d = 0; d < static_cast<int>(in.SchedulingDays()); d++)
      line_load[l][d]=0;
    for(c = 0; c < static_cast<int>(in.Categories()); c++)
      number_of_opr_per_category_to_lines[l][c]=0;
  }    

  for (l = 0; l < static_cast<int>(in.Lines()); l++)
  {
    category_deviation[l][0]=-1;
    category_deviation[l][1]=-1;
    second_category_deviation[l][0]=-1;
    second_category_deviation[l][1]=-1;
  }
   
}   


istream& operator>>(istream& is, Prod_Output& out)
{ 
  unsigned i, j, day, line, order, operation;
  out.Reset(); //initializing to -1
  for (i = 0; i < out.assign.size(); i++)
  {
    for (j = 0; j < out.in.NumberOfOperations(i); j++) //fill in only where needed
    {
      is >> order >> operation >> day >> line; 
      if(order!=j)
      {
        cerr<<"Error: inconsistency";
      }
      out.Assign(order, operation, -1, day, -1, line); //old_d and old_l are -1
    }
  }
  return is; 
} 


ostream& operator<<(ostream& os, const Prod_Output& out) 
{ 
  unsigned i, k;
  int r,j;

  for (i = 0; i < out.assign.size(); i++)
  {
	os <<"Ordine: "<< i <<endl;
    for (j = 0; j <static_cast<int>(out.assign[i].size()); j++) 
    {
      r = out.assign[i][j];
      
      os <<"Giorno "<< j <<" nella linea "<< r;
      if (r != -1)
      {
        for(k = 0; k < out.assign[i].size(); k++)
        {
          if(out.OperationDay(i,k) == j)
          {
            os<<" operazione numero: " << k << endl;
            break;
          }
        }
      }
      else
        os<<endl;
    }
  }
  return os; 
}

bool operator==(const Prod_Output& out1, const Prod_Output& out2)
{
  return out1.assign == out2.assign;    
}


void Prod_Output::Dump(ostream& os) const
{
  unsigned o, d, p, l,c;

  for(o = 0; o < in.Orders(); o++)
  {
    os << "Order number:" << o <<endl;
  
    for(d = 0; d < in.SchedulingDays(); d++)
      os << "Assign, day: "<<d<<" in line " << assign[o][d] <<endl;
    for (p = 0; p < in.NumberOfOperations(o); p++)
      os<< "The operation "<<p<<" is assigned in the day: " << operation_day[o][p]<<endl;
    for (l = 0; l < in.Lines(); l++)
    os <<  "The line number "<<l<<" has :" << number_of_opr_per_order_to_lines[o][l]<< " operations of this order" << endl;
    os<< "Number of lines for this order:"<< number_of_lines_for_a_order[o]<<endl;
  }   

  for(l = 0; l < in.Lines(); l++)
  {
    os << "Line number" << l <<endl;
    for(d = 0; d < in.SchedulingDays(); d++)
      os << "In the day "<<d<<" the line has "<< line_load[l][d] << " operations."<<endl;
    for(c = 0; c < in.Categories(); c++)
      os <<"The line has "<< number_of_opr_per_category_to_lines[l][c]<< " operations for the category " << c <<endl;
  }    
  os << "Number of Categories" << in.Categories() <<endl;
  for (l = 0; l < in.Lines(); l++)
    {
      os <<"The min category for line " <<l << " is "<< category_deviation[l][0] << " the max is "<< category_deviation[l][1]<<endl;
      os <<"The second min category for line " <<l<< " is " <<second_category_deviation[l][0] << " the max is "<< second_category_deviation[l][1]<<endl;
    }  
}  
