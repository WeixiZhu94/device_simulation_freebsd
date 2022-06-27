share:
	clang++ accelerator_runtime.cpp prepare_kernel.cpp -I../include -o library.o
	clang++ test_x86_compatible_device.cpp -o test_x86.o
	clang++ library.o test_x86.o -o share_cpu_test.x