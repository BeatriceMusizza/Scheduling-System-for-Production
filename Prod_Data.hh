// File Prod_Data.hh
#ifndef PROD_DATA_HH
#define PROD_DATA_HH
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>

using namespace std;
class Prod_Input  
{
  friend ostream& operator<<(ostream& os, const Prod_Input& in);
 public:
  Prod_Input(string file_name);
  unsigned Lines() const { return lines; }
  unsigned Orders() const { return orders; }
  unsigned SchedulingDays() const { return schedulingdays; }
  unsigned Categories() const { return categories; }
  unsigned MinStartDates(unsigned o) const { return min_start_dates[o]; }
  unsigned MaxEndDates(unsigned o) const { return max_end_dates[o]; }
  unsigned NumberOfOperations(unsigned o) const { return number_of_operations[o]; }
  unsigned TotalOperations() const { return totaloperations; }
  unsigned OrderOfTheOperation(unsigned p) const { return orders_operations[p].first; }
  unsigned OperationPositionInTheOrder(unsigned p) const { return orders_operations[p].second; }
  unsigned Category (unsigned o) const{ return category[o];}
 private:
  unsigned lines, orders, categories, schedulingdays, totaloperations;
  vector<unsigned> category; 
  vector<unsigned> min_start_dates;
  vector<unsigned> max_end_dates;
  vector<unsigned> number_of_operations;
  vector<pair<unsigned,unsigned >> orders_operations; //vector of pair to save for each operation the order to which it belongs and the position of the operation within this order
};


class Prod_Output 
{
  friend ostream& operator<<(ostream& os, const Prod_Output& out);
  friend istream& operator>>(istream& is, Prod_Output& out);
  friend bool operator==(const Prod_Output& out1, const Prod_Output& out2);
 public:
  Prod_Output(const Prod_Input& in);
  Prod_Output& operator=(const Prod_Output& out);
  int operator()(int i, int j) const { return assign[i][j]; }
  int& operator()(int i, int j) { return assign[i][j]; }
  void Assign(int order, int p, int old_d, int new_d, int old_l, int new_l); //assign a line to a order in a specific day
  void Reset();
  unsigned LineLoad (unsigned l, unsigned d) const { return line_load[l][d]; }
  int OperationDay(unsigned o, unsigned p) const { return operation_day[o][p];}
  unsigned OperationPerOrderToLines(unsigned o, unsigned l) const { return number_of_opr_per_order_to_lines[o][l];} 
  int MinCategory(unsigned l) const { return category_deviation[l][0];}
  int MaxCategory(unsigned l) const { return category_deviation[l][1];}
  int SecondMinCategory(unsigned l) const { return second_category_deviation[l][0]; }
  int SecondMaxCategory(unsigned l) const { return second_category_deviation[l][1]; }
  unsigned OperationPerCategoryToLines (int l, int c) const { return number_of_opr_per_category_to_lines[l][c]; }
  unsigned NumberOfLinesForAOrder (int o)  const { return number_of_lines_for_a_order[o]; }
  void Dump(ostream& os) const;
 protected:
  const Prod_Input& in; 
  vector<vector<int>> assign; //output matrix (orders x days)
  //Rebundant Data
  vector<vector<unsigned>> line_load; //how many operations has a line in a day (lines x days)
  vector<vector<int>> operation_day; //frast_matrix to know in which days the operations of each order are made (orders x operations)
  vector<vector<unsigned>> number_of_opr_per_order_to_lines; //how many operations of each order are assigned to a line  (orders x lines)
  vector<vector<int>> category_deviation; // matrix to know min and max category done by a line (lines x 2)
  vector<vector<int>> second_category_deviation; //matrix to know second min and second max category done by a line (lines x 2)
  vector<vector<unsigned>> number_of_opr_per_category_to_lines; //the number of order of each category that a line has (lines x category)
  vector<unsigned> number_of_lines_for_a_order; //the number of different lines that make an order 
};
#endif
