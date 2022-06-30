all: obj1 obj2 obj3 obj4 share x97

obj1:
	clang++ accelerator_runtime.cpp -I../include -o a.o
obj2:
	clang++ prepare_kernel.cpp  -I../include -o b.o
obj3:
	clang++ test_x86_compatible_device.cpp -I../include -o c.o

obj4:
	clang++ test_x97_device.cpp -I../include -o d.o

share: obj1 obj2 obj3
	clang++ a.o b.o c.o -o share_cpu_test.x

x97: obj1 obj2 obj4
	clang++ a.o b.o d.o -o x97_test.x

# 	clang++ accelerator_runtime.cpp prepare_kernel.cpp test_x86_compatible_device.cpp -I../include -o share_cpu_test.x
	
# 	clang++ accelerator_runtime.cpp -I../include -o a.o
# 	clang++ prepare_kernel.cpp  -I../include -o b.o
# 	clang++ test_x86_compatible_device.cpp -I../include -o c.o
# 	clang++ *.a -o share_cpu_test.x
clean:
	rm *.o *.x