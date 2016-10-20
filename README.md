This is our group's repository for our EE461S class.

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

### TEST STATUS

	pass tests/filesys/base/syn-write
	pass tests/userprog/args-none
	pass tests/userprog/args-single
	pass tests/userprog/args-multiple
	pass tests/userprog/args-many
	pass tests/userprog/args-dbl-space
	pass tests/userprog/sc-bad-sp
	pass tests/userprog/sc-bad-arg
	pass tests/userprog/sc-boundary
	pass tests/userprog/sc-boundary-2
	pass tests/userprog/halt
	pass tests/userprog/exit
	pass tests/userprog/create-normal
	pass tests/userprog/create-empty
	pass tests/userprog/create-null
	pass tests/userprog/create-bad-ptr
	pass tests/userprog/create-long
	pass tests/userprog/create-exists
	pass tests/userprog/create-bound
	pass tests/userprog/open-normal
	pass tests/userprog/open-missing
	pass tests/userprog/open-boundary
	pass tests/userprog/open-empty
	pass tests/userprog/open-null
	pass tests/userprog/open-bad-ptr
	pass tests/userprog/open-twice
	pass tests/userprog/close-normal
	pass tests/userprog/close-twice
	pass tests/userprog/close-stdin
	pass tests/userprog/close-stdout
	pass tests/userprog/close-bad-fd
	pass tests/userprog/read-normal
	pass tests/userprog/read-bad-ptr
	pass tests/userprog/read-boundary
	pass tests/userprog/read-zero
	pass tests/userprog/read-stdout
	pass tests/userprog/read-bad-fd
	pass tests/userprog/write-normal
	pass tests/userprog/write-bad-ptr
	pass tests/userprog/write-boundary
	pass tests/userprog/write-zero
	pass tests/userprog/write-stdin
	pass tests/userprog/write-bad-fd
	pass tests/userprog/exec-once
	pass tests/userprog/exec-arg
	pass tests/userprog/exec-multiple
	pass tests/userprog/exec-missing
	pass tests/userprog/exec-bad-ptr
	pass tests/userprog/wait-simple
	pass tests/userprog/wait-twice
	pass tests/userprog/wait-killed
	pass tests/userprog/wait-bad-pid
	pass tests/userprog/multi-recurse
	pass tests/userprog/multi-child-fd
	pass tests/userprog/rox-simple
	pass tests/userprog/rox-child
	pass tests/userprog/rox-multichild
	pass tests/userprog/bad-read
	pass tests/userprog/bad-write
	pass tests/userprog/bad-read2
	pass tests/userprog/bad-write2
	pass tests/userprog/bad-jump
	pass tests/userprog/bad-jump2
	FAIL tests/vm/pt-grow-stack
	FAIL tests/vm/pt-grow-pusha
	pass tests/vm/pt-grow-bad
	FAIL tests/vm/pt-big-stk-obj
	pass tests/vm/pt-bad-addr
	pass tests/vm/pt-bad-read
	pass tests/vm/pt-write-code
	pass tests/vm/pt-write-code2
	FAIL tests/vm/pt-grow-stk-sc
	FAIL tests/vm/page-linear
	pass tests/vm/page-parallel
	FAIL tests/vm/page-merge-seq
	FAIL tests/vm/page-merge-par
	FAIL tests/vm/page-merge-stk
	FAIL tests/vm/page-merge-mm
	pass tests/vm/page-shuffle
	FAIL tests/vm/mmap-read
	FAIL tests/vm/mmap-close
	pass tests/vm/mmap-unmap
	FAIL tests/vm/mmap-overlap
	FAIL tests/vm/mmap-twice
	FAIL tests/vm/mmap-write
	FAIL tests/vm/mmap-exit
	FAIL tests/vm/mmap-shuffle
	FAIL tests/vm/mmap-bad-fd
	FAIL tests/vm/mmap-clean
	FAIL tests/vm/mmap-inherit
	FAIL tests/vm/mmap-misalign
	FAIL tests/vm/mmap-null
	FAIL tests/vm/mmap-over-code
	FAIL tests/vm/mmap-over-data
	FAIL tests/vm/mmap-over-stk
	FAIL tests/vm/mmap-remove
	FAIL tests/vm/mmap-zero
	pass tests/filesys/base/lg-create
	pass tests/filesys/base/lg-full
	pass tests/filesys/base/lg-random
	pass tests/filesys/base/lg-seq-block
	pass tests/filesys/base/lg-seq-random
	pass tests/filesys/base/sm-create
	pass tests/filesys/base/sm-full
	pass tests/filesys/base/sm-random
	pass tests/filesys/base/sm-seq-block
	pass tests/filesys/base/sm-seq-random
	pass tests/filesys/base/syn-read
	pass tests/filesys/base/syn-remove
	pass tests/filesys/base/syn-write
	26 of 109 tests failed.

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


