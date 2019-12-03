#include "differential_privacy/algorithms/confidence-interval.pb.h"
#include "differential_privacy/algorithms/util.h"
#include "differential_privacy/proto/data.pb.h"
#include "differential_privacy/proto/util.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"
#include "differential_privacy/college_example/college_data.h"

using absl::PrintF;
using differential_privacy::BoundingReport;
using differential_privacy::ConfidenceInterval;
using differential_privacy::DefaultEpsilon;
using differential_privacy::GetValue;
using differential_privacy::Output;
using differential_privacy::college_example::DataReporter;
using differential_privacy::base::StatusOr;

ABSL_FLAG(std::string, CollegeDataFile,
          "differential_privacy/college_example/college_data.csv",
          "path to datafile where college data lives");
        
double epsilon = 4 * DefaultEpsilon();
DataReporter reporter(absl::GetFlag(FLAGS_CollegeDataFile), epsilon);

int main(int argc, char **argv) {
  std::string major = "Chemistry";
  std:: string minor = "Accounting";
  int major_limit = 400;
  int minor_limit = 200;

  PrintF(
    "\nIt is a new new semester, and the President of Oakland University wants "
    "to collect some statistics on the students' grades and paths of study. "
    "The president has noticed that she has bumped into a lot of %s majors "
    "in the past few semesters, and she wants to know how many students are "
    "majoring in %s.\n", major, major);

  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  PrintF(
    "\nUnsure of the President's intentions, the students decide to provide her "
    "with differentially private results.\n");
    
  PrintF("\nTrue sum of %s majors: %d\n", major, reporter.MajorSum(major));
  PrintF("DP sum of %s majors: %d\n", major, GetValue<int>(reporter.PrivateMajorSum(major, 0.10).ValueOrDie()));

  PrintF(
    "\nUpon receiving these results, the President is pleased to see that there are "
    "still plenty of students with an interest in %s. As a former %s major "
    "herself, these results make her happy.\n", major, major);

  PrintF(
    "\nAfter some thought, the President becomes curious about the number of students "
    "minoring in %s.\n", minor);
  
  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  PrintF(
    "\nFearful that the President might cut this program if she knew the true sum of students "
    "minoring in %s, they decide to again provide her with differentially private results.\n",
    minor);
 
  PrintF("\nTrue sum of %s minors %d\n", minor, reporter.MinorSum(minor));
  PrintF("DP sum of %s minors: %d\n", minor, GetValue<int>(reporter.PrivateMinorSum(minor, 0.10).ValueOrDie()));

  PrintF(
    "\nThe President catches on that the students are providing her with differentially private results. "
    "She wants to know how many minor programs have above %d students, but this time, she wants more "
    "information provided with the answer.\n", minor_limit);
 
  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  {
    Output count_output = reporter.PrivateMinorCountAbove(minor_limit, 0.20).ValueOrDie();
    int count = GetValue<int>(count_output);
    ConfidenceInterval ci =
      count_output.error_report().noise_confidence_interval();
    double confidence_level = ci.confidence_level();
    double lower_bound = ci.lower_bound();
    double upper_bound = ci.upper_bound();
    PrintF("\nTrue count of minors with over %d students: %d\n", minor_limit, reporter.MinorCountAbove(minor_limit));
    PrintF("DP count output:\n%s\n", count_output.DebugString());
    PrintF(
      "%d is the DP count of minors with over %d students. [%.2f, %.2f] is the "
      "%.2f confidence interval of the noise added to the count.\n",
      count, minor_limit, lower_bound, upper_bound, confidence_level);
  }

  PrintF(
    "\nSimilarly, she wants to know how many major programs have over %d students, with additional "
    "information provided.\n", major_limit);

  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  {
    Output count_output = reporter.PrivateMajorCountAbove(major_limit, 0.20).ValueOrDie();
    int count = GetValue<int>(count_output);
    ConfidenceInterval ci =
      count_output.error_report().noise_confidence_interval();
    double confidence_level = ci.confidence_level();
    double lower_bound = ci.lower_bound();
    double upper_bound = ci.upper_bound();
    PrintF("\nTrue count of majors with over %d students: %d\n", major_limit, reporter.MajorCountAbove(major_limit));
    PrintF("DP count output:\n%s\n", count_output.DebugString());
    PrintF(
      "%d is the DP count of majors with over %d students. [%.2f, %.2f] is the "
      "%.2f confidence interval of the noise added to the count.\n",
      count, major_limit, lower_bound, upper_bound, confidence_level);
  }

  PrintF(
    "\nNow the president is concerned with the average GPA of %s majors. She knows that %s is a tough "
    "program, and she wants to make sure that the students have all the resources they need to "
    "succeed.\n", major, major);

  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  PrintF("\nTrue average college GPA for %s majors: %.2f\n", major, reporter.GPAMajorMean(major));
  StatusOr<Output> gpa_mean_status = reporter.PrivateGPAMajorMean(major, 0.20);
  if (!gpa_mean_status.ok()) {
    PrintF("Error obtaining mean: %s\n", gpa_mean_status.status().message());
    PrintF(
        "Unable to get the private mean with the current "
        "privacy parameters. This is due to the small size of the dataset and "
        "random chance. Please re-run report_the_carrots to try again.\n");
  } else {
    Output mean_output = gpa_mean_status.ValueOrDie();
    BoundingReport report = mean_output.error_report().bounding_report();
    double mean = GetValue<double>(mean_output);
    int lower_bound = GetValue<int>(report.lower_bound());
    int upper_bound = GetValue<int>(report.upper_bound());
    double num_inputs = report.num_inputs();
    double num_outside = report.num_outside();
    PrintF("DP average college GPA for %s majors:\n%s\n", major, mean_output.DebugString());
    PrintF(
        "%.2f is the DP average college GPA for %s majors. "
        "Since no bounds were set for  the DP mean algorithm, bounds on the "
        "input data were automatically determined. Most of the data fell "
        "between [%d, %d]. Thus, these bounds were used to determine clamping "
        "and global sensitivity. In addition, around %.0f input values fell "
        "inside of these bounds, and around %.0f inputs fell outside of these "
        "bounds. num_inputs and num_outside are themselves DP counts.\n",
        mean, major, lower_bound, upper_bound, num_inputs, num_outside);
  }

  PrintF(
    "\nThe President has a hunch that those with a higher highschool SAT score choose harder major "
    "major programs. Knowing that %s is a tough program, she wants to know the average highschool "
    "SAT score of %s majors, to see if her hunch is correct.\n", major, major);

  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  PrintF("\nTrue average highschool SAT score for %s majors: %.2f\n", major, reporter.SATMajorMean(major));
  StatusOr<Output> sat_mean_status = reporter.PrivateSATMajorMean(major, 0.20);
  if (!sat_mean_status.ok()) {
    PrintF("Error obtaining mean: %s\n", sat_mean_status.status().message());
    PrintF(
        "Unable to get the private mean with the current "
        "privacy parameters. This is due to the small size of the dataset and "
        "random chance. Please re-run report_the_carrots to try again.\n");
  } else {
    Output mean_output = sat_mean_status.ValueOrDie();
    BoundingReport report = mean_output.error_report().bounding_report();
    double mean = GetValue<double>(mean_output);
    int lower_bound = GetValue<int>(report.lower_bound());
    int upper_bound = GetValue<int>(report.upper_bound());
    double num_inputs = report.num_inputs();
    double num_outside = report.num_outside();
    PrintF("DP average highschool SAT score for %s majors:\n%s\n", major, mean_output.DebugString());
    PrintF(
        "%.2f is the DP average highschool SAT score for %s majors. "
        "Since no bounds were set for  the DP mean algorithm, bounds on the "
        "input data were automatically determined. Most of the data fell "
        "between [%d, %d]. Thus, these bounds were used to determine clamping "
        "and global sensitivity. In addition, around %.0f input values fell "
        "inside of these bounds, and around %.0f inputs fell outside of these "
        "bounds. num_inputs and num_outside are themselves DP counts.\n",
        mean, major, lower_bound, upper_bound, num_inputs, num_outside);
  }

  PrintF("\nPrivacy budget remaining: %.2f\n", reporter.PrivacyBudget());

  PrintF(
    "\nThe students notice that the privacy budget has been depleted. To protect their own privacy, "
    "they will not answer any more of the President's questions.\n");
}

