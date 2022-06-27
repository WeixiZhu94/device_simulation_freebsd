share:
	clang++ accelerator_runtime.cpp -I../include -o a.o
	clang++ prepare_kernel.cpp  -I../include -o b.o
	clang++ test_x86_compatible_device.cpp -I../include -o c.o
	clang++ *.a -o share_cpu_test.x

clean:
	rm *.o *.x