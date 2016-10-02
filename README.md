 Project2 - Process API and SysCalls

This is the [User Programs][userprog] project in Pintos. To get started:

- Form a team of 3 members

1. Create a fork of this project2 repository (click on the "Fork" button above)
1. Give your team write permissions under Settings -> Collaborators and Teams.
1. Working on the Project on your Linux Machine:
  2. [**Clone**][ref-clone] the repository to your computer.
  2. As you work on the project, you will modify the files and [**commit**][ref-commit] changes to complete your solution.
  2. [**Push**][ref-push]/sync the changes up to your fork on GitHub.
1. Submitting your Project:
  2. [Create a **pull request**][pull-request] on the original repository to turn in the assignment.

- Tests Fail/Pass Status (Update this every time you push with what you see in make check,
please never push if you cause tests to fail that weren't failing already)

#TEST STATUS
- FAIL tests/userprog/exec-missing
From the stanford pintos project 2, exec missing is more complicated than we had thought:
Runs the executable whose name is given in cmd_line, passing any given arguments, and returns the new process's program id (pid). Must return pid -1, which otherwise should not be a valid pid, if the program cannot load or run for any reason. Thus, the parent process cannot return from the exec until it knows whether the child process successfully loaded its executable. You must use appropriate synchronization to ensure this.
- FAIL tests/userprog/no-vm/multi-oom
Ziping will look into this one.
- FAIL tests/filesys/base/syn-read
Currently syn-read and syn-write is failing because of a weird issue. The pintos loads two executables, however the test is failing 
since pintos is unable to actually run the second executable. From what I can see, the second executable is being loaded by 10 child processes, thus it may be failing to load since everyone is trying to load the executable. This is my guess, but it may not be correct.
- FAIL tests/filesys/base/syn-write
- 4 of 76 tests failed.




<!-- Links -->
[userprog]: https://web.stanford.edu/class/cs140/projects/pintos/pintos_3.html#SEC32
[forking]: https://guides.github.com/activities/forking/
[ref-clone]: http://gitref.org/creating/#clone
[ref-commit]: http://gitref.org/basic/#commit
[ref-push]: http://gitref.org/remotes/#push
[pull-request]: https://help.github.com/articles/creating-a-pull-request
[raw]: https://raw.githubusercontent.com/education/guide/master/docs/forks.md
