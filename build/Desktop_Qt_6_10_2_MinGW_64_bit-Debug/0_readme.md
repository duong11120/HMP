## Testing instructions

You can use these test files in the graphical user interface using: 
- read <file.txt> 
- testread <file-in.txt> <file-out-txt>

You can also run the tests from command line but first you need to build the executable.
Run the "build.sh" in bash. This will build tiraka26.exe in the project root-directory: 

> ./build.sh

Now you can test your code running this in bash:

> ../tiraka26 A-ALL-testsuite.txt

The files named A-ALL-testsuite.txt run all available tests for level A-methods, 
files marked "AB" and "ABC" have more methods.


# Updating tests 

You can add test files and make changes to these files LOCALLY, but you cannot push your changes
to the "tests" - repository (submodule). If you want to store your own test-files, make a new folder in the root of your course-gitlab-repo (for ex. /mytests/), put your test files in it and git add/commit/push normally. 