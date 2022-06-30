all: obj1 obj2 obj3 obj4

obj1:
	clang++ accelerator_runtime.cpp -I../include -o a.o
obj2:
	clang++ prepare_kernel.cpp  -I../include -o b.o

obj3: obj1 obj2
	clang++ *.o test_x86_compatible_device.cpp -I../include -o share_cpu_test.x
obj4: obj1 obj2 
	clang++ *.o test_x97_device.cpp -I../include -o x97_test.x


# 	clang++ accelerator_runtime.cpp prepare_kernel.cpp test_x86_compatible_device.cpp -I../include -o share_cpu_test.x
	
# 	clang++ accelerator_runtime.cpp -I../include -o a.o
# 	clang++ prepare_kernel.cpp  -I../include -o b.o
# 	clang++ test_x86_compatible_device.cpp -I../include -o c.o
# 	clang++ *.a -o share_cpu_test.x
clean:
	rm -rf *.o *.x