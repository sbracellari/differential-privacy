#include "differential_privacy/college_example/college_data.h"

#include <fstream>
#include <sstream>

#include "differential_privacy/algorithms/bounded-mean.h"
#include "differential_privacy/algorithms/bounded-sum.h"
#include "differential_privacy/algorithms/count.h"
#include "differential_privacy/algorithms/order-statistics.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"

namespace differential_privacy {
namespace college_example {




DataReporter::DataReporter(std::string data_filename, double epsilon)
    : epsilon_(epsilon) {
  std::ifstream file(data_filename);
  std::string line;
    
  while(getline(file, line)) {
    std::vector<std::string> data = absl::StrSplit(line, ',');
    first_name.push_back(data[0]);
    last_name.push_back(data[1]);
    age.push_back(data[2]);
    gender.push_back(data[3]);
    email.push_back(data[4]);
    college_gpa.push_back(data[5]);
    standing.push_back(data[6]);
    majors.push_back(data[7]);
    minors.push_back(data[8]);
    transfer_student.push_back(data[9]);
    greek_life.push_back(data[10]);
    commuter.push_back(data[11]);
    highschool_gpa.push_back(data[12]);
    sat_score.push_back(data[13]);

    Student student;
    student.FIRST_NAME = data[0];
    student.LAST_NAME = data[1];
    student.MAJOR = data[7];
    student.COLLEGE_GPA = data[5];
    student.HIGHSCHOOL_GPA = data[12];
    student.SAT_SCORE = data[13];

    students.push_back(student);
  }
}

// Creates the map students_per_major, which contains a value of major
// with an associated key of how many students study that major
std::map<std::string, int> DataReporter::MajorsMap() {
  std::map<std::string, int> students_per_major;
  for (auto data_ : majors) {
    if (students_per_major.find(data_) == students_per_major.end()) {
      students_per_major[data_] = 1;
    } else {
      students_per_major[data_] += 1;
    }  
  }
  return students_per_major;
}

// Creates the map students_per_minor, which contains a value of minor
// with an associated key of how many students study that minor
std::map<std::string, int> DataReporter::MinorsMap() {
  std::map<std::string, int> students_per_minor;
  for (auto data_ : minors) {
    if (students_per_minor.find(data_) == students_per_minor.end()) {
      students_per_minor[data_] = 1;
    } else {
      students_per_minor[data_] += 1;
    }
  }
  return students_per_minor;
}

// Calculates the number of students studying a specified major
int DataReporter::MajorSum(std::string major) {
  std::map<std::string, int> students_per_major = MajorsMap();
  return students_per_major.at(major);
}

// Calculates the number of students studying a specified minor
int DataReporter::MinorSum(std::string minor) {
  std::map<std::string, int> students_per_minor = MinorsMap();
  return students_per_minor.at(minor);
}

// Calculates the number of majors that have above a specified amount
// of students
int DataReporter::MajorCountAbove(int limit) {
  std::map<std::string, int> students_per_major = MajorsMap();
  int major_count = 0;

  for (const auto& pair : students_per_major) {
    if (pair.second > limit) {
      major_count += 1;
    }
  }

  return major_count;
}

// Calculates the number of minors that have above a specified amount
// of students
int DataReporter::MinorCountAbove(int limit) {
  std::map<std::string, int> students_per_minor = MinorsMap();
  int minor_count = 0;

  for (const auto& pair : students_per_minor) {
    if (pair.second > limit) {
      minor_count += 1;
    }
  }

  return minor_count;
}

// Calculates average GPA for a specified college major
double DataReporter::GPAMajorMean(std::string major) {
  double sum = 0;
  int count = 0;
  for (int i = 0; i < students.size(); i++) {
    if (students[i].MAJOR == major) {
      sum += stoi(students[i].COLLEGE_GPA);
      count++;
    }
  }
  return sum/count;
}

// Calculates average highschool SAT score for a specified college major
double DataReporter::SATMajorMean(std::string major) {
  double sum = 0;
  int count = 0;
  for (int i = 0; i < students.size(); i++) {
    if (students[i].MAJOR == major) {
      sum += stoi(students[i].SAT_SCORE);
      count++;
    }
  }
  return sum/count;
}

double DataReporter::PrivacyBudget() { return privacy_budget_; }

base::StatusOr<Output> DataReporter::PrivateMajorSum(std::string major, double privacy_budget) {
  if (privacy_budget_ < privacy_budget) {
    return base::InvalidArgumentError("Not enough privacy budget.");
  }
  privacy_budget_ -= privacy_budget;
  ASSIGN_OR_RETURN(std::unique_ptr<BoundedSum<int>> sum_algorithm,
                   BoundedSum<int>::Builder()
                       .SetEpsilon(epsilon_)
                       .SetLower(0)
                       .SetUpper(25)
                       .Build());

  for (auto m : majors) {
    if (m == major) {
      sum_algorithm->AddEntry(1);
    }
  }

  return sum_algorithm->PartialResult(privacy_budget);
}

base::StatusOr<Output> DataReporter::PrivateMinorSum(std::string minor, double privacy_budget) {
  if (privacy_budget_ < privacy_budget) {
    return base::InvalidArgumentError("Not enough privacy budget.");
  }
  privacy_budget_ -= privacy_budget;
  ASSIGN_OR_RETURN(std::unique_ptr<BoundedSum<int>> sum_algorithm,
                   BoundedSum<int>::Builder()
                      .SetEpsilon(epsilon_)
                      .SetLower(0)
                      .SetUpper(15)
                      .Build());

  for (auto m : minors) {
    if (m == minor) {
      sum_algorithm->AddEntry(1);
    }
  }

  return sum_algorithm->PartialResult(privacy_budget);
}

base::StatusOr<Output> DataReporter::PrivateMajorCountAbove(int limit, double privacy_budget) {
  if (privacy_budget_ < privacy_budget) {
    return base::InvalidArgumentError("Not enough privacy budget.");
  }
  privacy_budget_ -= privacy_budget;
  ASSIGN_OR_RETURN(std::unique_ptr<Count<std::string>> count_algorithm,
                   Count<std::string>::Builder().SetEpsilon(epsilon_).Build());
  std::map<std::string, int> students_per_major = MajorsMap();

  for (const auto& pair : students_per_major) {
    if (pair.second > limit) {
      count_algorithm->AddEntry(pair.first);
    }
  }
  return count_algorithm->PartialResult(privacy_budget);
}

base::StatusOr<Output> DataReporter::PrivateMinorCountAbove(int limit, double privacy_budget) {
  if (privacy_budget_ < privacy_budget) {
    return base::InvalidArgumentError("Not enough privacy budget.");
  }
  privacy_budget_ -= privacy_budget;
  ASSIGN_OR_RETURN(std::unique_ptr<Count<std::string>> count_algorithm,
                   Count<std::string>::Builder().SetEpsilon(epsilon_).Build());
  std::map<std::string, int> students_per_minor = MinorsMap();

  for (const auto& pair : students_per_minor) {
    if(pair.second > limit) {
      count_algorithm->AddEntry(pair.first);
    }
  }
  return count_algorithm->PartialResult(privacy_budget);
}

base::StatusOr<Output> DataReporter::PrivateGPAMajorMean(std::string major, double privacy_budget) {
  if (privacy_budget_ < privacy_budget) {
    return base::InvalidArgumentError("Not enough privacy budget.");
  }
  privacy_budget_ -= privacy_budget;
  ASSIGN_OR_RETURN(std::unique_ptr<BoundedMean<int>> mean_algorithm,
                   BoundedMean<int>::Builder().SetEpsilon(epsilon_).Build());
  for (int i = 0; i < students.size(); i++) {
    if (students[i].MAJOR == major) {
      mean_algorithm->AddEntry(stoi(students[i].COLLEGE_GPA));
    }
  }
  return mean_algorithm->PartialResult(privacy_budget);
}

base::StatusOr<Output> DataReporter::PrivateSATMajorMean(std::string major, double privacy_budget) {
  if (privacy_budget_ < privacy_budget) {
    return base::InvalidArgumentError("Not enough privacy budget.");
  }
  privacy_budget_ -= privacy_budget;
  ASSIGN_OR_RETURN(std::unique_ptr<BoundedMean<int>> mean_algorithm,
                   BoundedMean<int>::Builder().SetEpsilon(epsilon_).Build());
  for (int i = 0; i < students.size(); i++) {
    if (students[i].MAJOR == major) {
      mean_algorithm->AddEntry(stoi(students[i].SAT_SCORE));
    }
  }
  return mean_algorithm->PartialResult(privacy_budget);
}


}
}