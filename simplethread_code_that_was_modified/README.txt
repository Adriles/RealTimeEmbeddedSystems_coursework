The files in this folder are included since the submission instructions say to include any files that were modified.

These files were modified by changing the pthread creation call to include &rt_sched_attr[i] to pass the thread parameters to them.

Otherwise the files are unchanged from Siewert's examples.

Not included, but one of the Makefiles was updated to include the compilation flag -lrt, which caused compilation errors when missing.