#PROJECTS AND TEST STATUS
## Project 3: Virtual Memory
- Read about the project [here](https://github.com/ZipingL/EE461S_Projects/wiki/Project-3:-Intro-to-Virtual-Memory).
- View the TODO List for Project3 [here](https://github.com/ZipingL/EE461S_Projects/projects/2).
- Suggested Order of Implementation:
	- Frame table (without swapping). Once complete, all project 2 test cases should still be passing
	- Supplemental Page Table
	- Page fault handler. Kernel should pass all projecct 2 test cases, but only some of the robustness cases (?)
	- Stack growth
	- Mapped files
	- Page reclamation on process exit (the last three can be done in parallel)
	- Eviction
- I would focus on trying the last couple of problems on the practice exam, because it really does give you a good idea of how paging works and so would really clear up your understanding before you just start writing code.
### Passed Tests
- All tests failing. Edit this if you fixed some tests, e.g., add any tests that you pass to this list.

## Project 2: User Programs
- Writing the wiki page to document the project. Still need to finish [implementation details](https://github.com/ZipingL/EE461S_Projects/wiki/Project-2:-Implementation-Details).
### Passed Tests
- All tests passing. The protected Branch for the finished project2 code can be viewed [here](https://github.com/ZipingL/EE461S_Projects/tree/project2_userprograms).



<!-- Links -->
[userprog]: https://web.stanford.edu/class/cs140/projects/pintos/pintos_3.html#SEC32
[forking]: https://guides.github.com/activities/forking/
[ref-clone]: http://gitref.org/creating/#clone
[ref-commit]: http://gitref.org/basic/#commit
[ref-push]: http://gitref.org/remotes/#push
[pull-request]: https://help.github.com/articles/creating-a-pull-request
[raw]: https://raw.githubusercontent.com/education/guide/master/docs/forks.md
