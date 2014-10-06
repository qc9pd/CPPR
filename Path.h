#ifndef PATH_HEADER
#define PATH_HEADER

#include <iostream>
#include <string>

using namespace std;

class Path
{
 private:
  string PathInfo;
  double pre_slack_hold;
  double pre_slack_setup;
  double post_slack_hold;
  double post_slack_setup;

 public:
  Path(string n , double pre_hold);
  Path(double pre_setup, string n);
  Path(string n, double pre_hold, double pre_setup);
  ~Path();
  string GetPathName();
  void SetPreSlackHold(double t);
  void SetPreSlackSetup(double t);
  void SetPostSlackHold(double t);
  void SetPostSlackSetup(double t);
  const double GetPreSlackHold();
  const double GetPreSlackSetup();
  const double GetPostSlackHold();
  const double GetPostSlackSetup();
  // void PrintPath(string filename);
};

#endif
