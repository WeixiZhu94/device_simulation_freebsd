all: obj1 obj3 obj4 obj5 bp

obj1:
	clang++ accelerator_runtime.cpp prepare_kernel.cpp -c -I../include 
obj3: obj1
	clang++ *.o test_x86_compatible_device.cpp -I../include -o share_cpu_test.x
obj4: obj1 
	clang++ *.o test_x97_discrete_device.cpp -I../include -o x97_discrete_test.x
obj5: obj1 
	clang++ *.o test_x97_coherent_device.cpp -I../include -o x97_coherent_test.x

bp: 
	clang++ bp.cpp -I../include -o bp.x -lm

# 	clang++ accelerator_runtime.cpp prepare_kernel.cpp test_x86_compatible_device.cpp -I../include -o share_cpu_test.x
	
# 	clang++ accelerator_runtime.cpp -I../include -o a.o
# 	clang++ prepare_kernel.cpp  -I../include -o b.o
# 	clang++ test_x86_compatible_device.cpp -I../include -o c.o
# 	clang++ *.a -o share_cpu_test.x
clean:
	rm -rf *.o *.x