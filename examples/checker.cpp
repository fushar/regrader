
/**
 * Special problem checker template for problems used in Regrader
 *
 * Checker's executable will be run with command:
 *
 * ./[checker_executable] [testcase_input] [judges_output]
 *
 * and the contestant's output which will be checked is redirected using
 * the standard input (stdio).
 *
 * To read [testcase_input] or [judges_output], use:
 * > ifstream (C++ style)
 * > fopen (C style)
 *
 * [testcase_input] will be in argv[1] and
 * [judges_output] will be in argv[2]
 *
 * Checker will determine the final verdict of contestant's solution
 * by printing [OK] or [NOT OK] to standard output (stdout).
 *
 * [OK] will be counted as an Accepted, and
 * [NOT OK] will be counted as an Wrong Answer.
 * 
 * @author Pusaka Kaleb Setyabudi <sokokaleb@gmail.com>
 */

#include <bits/stdc++.h>
using namespace std;

#define OK "[OK]"
#define NOT_OK "[NOT OK]"

ifstream testcase_input, judges_output;
bool correct;

int main(int argc, char** argv)
{

	// prepare the input for official testcase files
	
	testcase_input.open(argv[1]);
	judges_output.open(argv[2]);



	// read contestant's output for this test case
	
	// .....

	

	// checker's logic starts here
	
	// .....



	// If correct, print [OK], otherwise [NOT OK]
	if (correct) cout << OK << endl;
	else cout << NOT_OK << endl;

	return 0;
}
