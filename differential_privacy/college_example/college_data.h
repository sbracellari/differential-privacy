#ifndef DIFFERENTIAL_PRIVACY_COLLEGE_EXAMPLE_COLLEGE_DATA_H_
#define DIFFERENTIAL_PRIVACY_COLLEGE_EXAMPLE_COLLEGE_DATA_H_
 
#include <map>
#include <string>
 
#include "differential_privacy/proto/data.pb.h"
#include "differential_privacy/base/statusor.h"
 
namespace differential_privacy {
namespace college_example {
 
// The DataReporter class helps students report differentially private (DP)
// aggregate statistics about various college data.
class DataReporter {
  public:
    // Epsilon is the differential privacy parameter. Epsilon is shared between all private
    // function calls. The fraction of epsilon remaining is tracked by privacy_budget_.
    DataReporter(std::string data_filename, double epsilon);

    // Function to generate a map of students per major
    std::map<std::string, int> MajorsMap();
    // Function to generate a map of students per minor
    std::map<std::string, int> MinorsMap();

    // True sum of individual majors
    int MajorSum(std::string major);
    // True sum of individual minors
    int MinorSum(std::string minor);
   
    // Number of majors that have over a specified amount of students
    int MajorCountAbove(int);
    // Number of minors that have over a specified amount of students
    int MinorCountAbove(int);

    // Average college GPA for a specified major
    double GPAMajorMean(std::string major);
   
    // Average highschool SAT score for a specified major
    double SATMajorMean(std::string major);

    // Returns remaining privacy budget
    double PrivacyBudget();
   
    // DP sum of individual majors
    base::StatusOr<Output> PrivateMajorSum(std::string major, double privacy_budget);
    // DP sum of individual minors
    base::StatusOr<Output> PrivateMinorSum(std::string minor, double privacy_budget);
   
    // DP number of majors that have over a specified amount of students
    base::StatusOr<Output> PrivateMajorCountAbove(int limit, double privacy_budget);
    // DP number of minors that have over a specified amount of students
    base::StatusOr<Output> PrivateMinorCountAbove(int limit, double privacy_budget);

    // DP average college GPA for a specified major
    base::StatusOr<Output> PrivateGPAMajorMean(std::string major, double privacy_budget);

    // DP average SAT score for a specified major
    base::StatusOr<Output> PrivateSATMajorMean(std::string major, double privacy_budget);
 
 private:
    std::vector<std::string> first_name;
    std::vector<std::string> last_name;
    std::vector<std::string> age;
    std::vector<std::string> gender;
    std::vector<std::string> email;
    std::vector<std::string> college_gpa;
    std::vector<std::string> standing;
    std::vector<std::string> majors;
    std::vector<std::string> minors;
    std::vector<std::string> transfer_student;
    std::vector<std::string> commuter;
    std::vector<std::string> greek_life;
    std::vector<std::string> highschool_gpa;
    std::vector<std::string> sat_score;

    struct Student {
      std::string FIRST_NAME;
      std::string LAST_NAME;
      std::string MAJOR;
      std::string COLLEGE_GPA;
      std::string HIGHSCHOOL_GPA;
      std::string SAT_SCORE;
    };

    std::vector<Student> students;

    // Differential privacy parameter epsilon. A larger epsilon corresponds to less
    // privacy and more accuracy.
    double epsilon_;
  
    // The initial privacy budget. If the budget depletes, no more questions can be asked
    // about the data. Privacy budget is represented as a fraction on [0, 1].
    double privacy_budget_ = 1;
};
 
} // namespace example
} // namespace differential_privacy
 
#endif // DIFFERENTIAL_PRIVACY_COLLEGE_EXAMPLE_COLLEGE_DATA_H_