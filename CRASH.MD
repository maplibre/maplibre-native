-------------------------------------
Translated Report (Full Report Below)
-------------------------------------
Process:             mbgl-glfw [67613]
Path:                /Users/USER/*/mbgl-glfw
Identifier:          mbgl-glfw
Version:             ???
Code Type:           ARM-64 (Native)
Role:                Foreground
Parent Process:      bash [67607]
Coalition:           com.microsoft.VSCode [2369]
Responsible Process: Electron [4953]
User ID:             501

Date/Time:           2025-09-18 21:57:56.9982 +0200
Launch Time:         2025-09-18 21:57:56.4289 +0200
Hardware Model:      MacBookPro18,2
OS Version:          macOS 26.0 (25A354)
Release Type:        User

Crash Reporter Key:  FF5D4141-568B-5C72-35D6-246897AC305D
Incident Identifier: 225D49FD-72A1-4059-93F2-387A587D69B6

Sleep/Wake UUID:       BECD6912-2E0F-405F-B474-B5D83C554401

Time Awake Since Boot: 9100 seconds

System Integrity Protection: enabled

Triggered by Thread: 0

Exception Type:    EXC_BAD_ACCESS (SIGBUS)
Exception Subtype: KERN_PROTECTION_FAILURE at 0x000000019560d030
Exception Codes:   0x0000000000000002, 0x000000019560d030

Termination Reason:  Namespace SIGNAL, Code 10, Bus error: 10
Terminating Process: exc handler [67613]


VM Region Info: 0x19560d030 is in 0x194601000-0x195cfd000;  bytes after start: 16826416  bytes before end: 7274447
      REGION TYPE                    START - END         [ VSIZE] PRT/MAX SHRMOD  REGION DETAIL
      __TEXT                      1945a5000-194601000    [  368K] r-x/r-x SM=COW  /usr/lib/libsandbox.1.dylib
--->  __TEXT                      194601000-195cfd000    [ 23.0M] r-x/r-x SM=COW  /System/Library/Frameworks/AppKit.framework/Versions/C/AppKit
      __TEXT                      195cfd000-195eb7000    [ 1768K] r-x/r-x SM=COW  /System/Library/PrivateFrameworks/UIFoundation.framework/Versions/A/UIFoundation

Thread 0 Crashed:
0   mbgl-glfw                     	       0x1048d1a08 dawn::RefCounted::AddRef() + 8
1   mbgl-glfw                     	       0x104990cb8 dawn::native::DeviceBase::APITick() + 52
2   mbgl-glfw                     	       0x1048c3760 GLFWWebGPUBackend::swap() + 364 (glfw_webgpu_backend.cpp:429)
3   mbgl-glfw                     	       0x1048347f8 GLFWView::render() + 280 (glfw_view.cpp:1183)
4   mbgl-glfw                     	       0x10485560c GLFWView::run()::$_0::operator()() const + 72 (glfw_view.cpp:1216)
5   mbgl-glfw                     	       0x1048555b8 decltype(std::declval<GLFWView::run()::$_0&>()()) std::__1::__invoke[abi:ne200100]<GLFWView::run()::$_0&>(GLFWView::run()::$_0&) + 24 (invoke.h:179)
6   mbgl-glfw                     	       0x104855594 void std::__1::__invoke_void_return_wrapper<void, true>::__call[abi:ne200100]<GLFWView::run()::$_0&>(GLFWView::run()::$_0&) + 24 (invoke.h:251)
7   mbgl-glfw                     	       0x104855570 void std::__1::__invoke_r[abi:ne200100]<void, GLFWView::run()::$_0&>(GLFWView::run()::$_0&) + 24 (invoke.h:273)
8   mbgl-glfw                     	       0x10485554c std::__1::__function::__alloc_func<GLFWView::run()::$_0, std::__1::allocator<GLFWView::run()::$_0>, void ()>::operator()[abi:ne200100]() + 24 (function.h:167)
9   mbgl-glfw                     	       0x104854e14 std::__1::__function::__func<GLFWView::run()::$_0, std::__1::allocator<GLFWView::run()::$_0>, void ()>::operator()() + 28 (function.h:319)
10  mbgl-glfw                     	       0x10484886c std::__1::__function::__value_func<void ()>::operator()[abi:ne200100]() const + 60 (function.h:436)
11  mbgl-glfw                     	       0x10482f544 std::__1::function<void ()>::operator()() const + 24 (function.h:995)
12  mbgl-glfw                     	       0x1055deb08 mbgl::util::Timer::Impl::perform(__CFRunLoopTimer*, void*) + 28 (timer.cpp:33)
13  CoreFoundation                	       0x190242290 __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__ + 32
14  CoreFoundation                	       0x190241f50 __CFRunLoopDoTimer + 980
15  CoreFoundation                	       0x190241ac8 __CFRunLoopDoTimers + 280
16  CoreFoundation                	       0x19023284c __CFRunLoopRun + 1816
17  CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
18  HIToolbox                     	       0x19cc2f730 RunCurrentEventLoopInMode + 316
19  HIToolbox                     	       0x19cc329d0 ReceiveNextEventCommon + 488
20  HIToolbox                     	       0x19cdbc1f4 _BlockUntilNextEventMatchingListInMode + 48
21  AppKit                        	       0x194b0a308 _DPSBlockUntilNextEventMatchingListInMode + 236
22  AppKit                        	       0x194620edc _DPSNextEvent + 588
23  AppKit                        	       0x195073a04 -[NSApplication(NSEventRouting) _nextEventMatchingEventMask:untilDate:inMode:dequeue:] + 688
24  AppKit                        	       0x195073710 -[NSApplication(NSEventRouting) nextEventMatchingMask:untilDate:inMode:dequeue:] + 72
25  libglfw.3.4.dylib             	       0x108443ae4 _glfwPollEventsCocoa + 140
26  mbgl-glfw                     	       0x104834e84 GLFWView::run() + 244 (glfw_view.cpp:1236)
27  mbgl-glfw                     	       0x1047f20a0 main + 12484 (main.cpp:214)
28  dyld                          	       0x18fdd5d54 start + 7184

Thread 1:
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   libdispatch.dylib             	       0x18fff415c _dispatch_mach_send_and_wait_for_reply + 548
5   libdispatch.dylib             	       0x18fff44fc dispatch_mach_send_with_result_and_wait_for_reply + 60
6   libxpc.dylib                  	       0x18fe7ba64 xpc_connection_send_message_with_reply_sync + 284
7   Metal                         	       0x19c412858 MTLXPCCompilerConnection::checkConnectionAlive() + 188
8   Metal                         	       0x19c5a7978 invocation function for block in MTLSchedulerRequest::generateXPCBlock(qos_class_t, int) + 1012
9   libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
10  libdispatch.dylib             	       0x18ffddc34 _dispatch_block_invoke_direct + 284
11  Metal                         	       0x19c56d198 invocation function for block in MTLCompilerScheduler::createBlockWithData(std::__1::shared_ptr<MTLSchedulerRequest>) + 48
12  libdispatch.dylib             	       0x18ffe84d0 _dispatch_block_async_invoke2 + 148
13  libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
14  libdispatch.dylib             	       0x18ffe14e8 _dispatch_lane_serial_drain + 740
15  libdispatch.dylib             	       0x18ffe1ff8 _dispatch_lane_invoke + 440
16  libdispatch.dylib             	       0x18ffec474 _dispatch_root_queue_drain_deferred_wlh + 292
17  libdispatch.dylib             	       0x18ffebd6c _dispatch_workloop_worker_thread + 692
18  libsystem_pthread.dylib       	       0x190190e4c _pthread_wqthread + 292
19  libsystem_pthread.dylib       	       0x19018fb9c start_wqthread + 8

Thread 2:
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   libdispatch.dylib             	       0x18fff415c _dispatch_mach_send_and_wait_for_reply + 548
5   libdispatch.dylib             	       0x18fff44fc dispatch_mach_send_with_result_and_wait_for_reply + 60
6   libxpc.dylib                  	       0x18fe7ba64 xpc_connection_send_message_with_reply_sync + 284
7   Metal                         	       0x19c412858 MTLXPCCompilerConnection::checkConnectionAlive() + 188
8   Metal                         	       0x19c5a7978 invocation function for block in MTLSchedulerRequest::generateXPCBlock(qos_class_t, int) + 1012
9   libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
10  libdispatch.dylib             	       0x18ffddc34 _dispatch_block_invoke_direct + 284
11  Metal                         	       0x19c56d198 invocation function for block in MTLCompilerScheduler::createBlockWithData(std::__1::shared_ptr<MTLSchedulerRequest>) + 48
12  libdispatch.dylib             	       0x18ffe84d0 _dispatch_block_async_invoke2 + 148
13  libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
14  libdispatch.dylib             	       0x18ffe14e8 _dispatch_lane_serial_drain + 740
15  libdispatch.dylib             	       0x18ffe1ff8 _dispatch_lane_invoke + 440
16  libdispatch.dylib             	       0x18ffec474 _dispatch_root_queue_drain_deferred_wlh + 292
17  libdispatch.dylib             	       0x18ffebd6c _dispatch_workloop_worker_thread + 692
18  libsystem_pthread.dylib       	       0x190190e4c _pthread_wqthread + 292
19  libsystem_pthread.dylib       	       0x19018fb9c start_wqthread + 8

Thread 3:
0   IOKit                         	       0x1944c7094 iokit_user_client_trap + 8
1   IOGPU                         	       0x1b455cd34 IOGPUCommandQueueSubmitCommandBuffers + 180
2   IOGPU                         	       0x1b4545b28 -[IOGPUMetalCommandQueue _submitCommandBuffers:count:] + 364
3   IOGPU                         	       0x1b4545990 -[IOGPUMetalCommandQueue submitCommandBuffers:count:] + 72
4   Metal                         	       0x19c3f7d44 -[_MTLCommandQueue _submitAvailableCommandBuffers] + 512
5   libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
6   libdispatch.dylib             	       0x18ffdd6e4 _dispatch_continuation_pop + 596
7   libdispatch.dylib             	       0x18fff0800 _dispatch_source_latch_and_call + 396
8   libdispatch.dylib             	       0x18ffef4d4 _dispatch_source_invoke + 844
9   libdispatch.dylib             	       0x18ffe1350 _dispatch_lane_serial_drain + 332
10  libdispatch.dylib             	       0x18ffe1fc4 _dispatch_lane_invoke + 388
11  libdispatch.dylib             	       0x18ffec474 _dispatch_root_queue_drain_deferred_wlh + 292
12  libdispatch.dylib             	       0x18ffebd6c _dispatch_workloop_worker_thread + 692
13  libsystem_pthread.dylib       	       0x190190e4c _pthread_wqthread + 292
14  libsystem_pthread.dylib       	       0x19018fb9c start_wqthread + 8

Thread 4:

Thread 5:: com.apple.NSEventThread
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   AppKit                        	       0x1946b0a68 _NSEventThread + 184
8   libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
9   libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 6:: org.maplibre.mbgl.Worker 1
0   libsystem_kernel.dylib        	       0x1901554f8 __psynch_cvwait + 8
1   libsystem_pthread.dylib       	       0x1901950dc _pthread_cond_wait + 984
2   libc++.1.dylib                	       0x1900c574c std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&) + 32
3   mbgl-glfw                     	       0x1054c7100 mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const + 360 (thread_pool.cpp:40)
4   mbgl-glfw                     	       0x1054c6f64 decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&) + 24 (invoke.h:179)
5   mbgl-glfw                     	       0x1054c6f40 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
6   mbgl-glfw                     	       0x1054c6d58 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*) + 84 (thread.h:214)
7   libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
8   libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 7:: org.maplibre.mbgl.Worker 2
0   libsystem_kernel.dylib        	       0x1901554f8 __psynch_cvwait + 8
1   libsystem_pthread.dylib       	       0x1901950dc _pthread_cond_wait + 984
2   libc++.1.dylib                	       0x1900c574c std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&) + 32
3   mbgl-glfw                     	       0x1054c7100 mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const + 360 (thread_pool.cpp:40)
4   mbgl-glfw                     	       0x1054c6f64 decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&) + 24 (invoke.h:179)
5   mbgl-glfw                     	       0x1054c6f40 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
6   mbgl-glfw                     	       0x1054c6d58 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*) + 84 (thread.h:214)
7   libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
8   libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 8:: org.maplibre.mbgl.Worker 3
0   libsystem_kernel.dylib        	       0x1901554f8 __psynch_cvwait + 8
1   libsystem_pthread.dylib       	       0x1901950dc _pthread_cond_wait + 984
2   libc++.1.dylib                	       0x1900c574c std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&) + 32
3   mbgl-glfw                     	       0x1054c7100 mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const + 360 (thread_pool.cpp:40)
4   mbgl-glfw                     	       0x1054c6f64 decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&) + 24 (invoke.h:179)
5   mbgl-glfw                     	       0x1054c6f40 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
6   mbgl-glfw                     	       0x1054c6d58 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*) + 84 (thread.h:214)
7   libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
8   libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 9:: org.maplibre.mbgl.Worker 4
0   libsystem_kernel.dylib        	       0x1901554f8 __psynch_cvwait + 8
1   libsystem_pthread.dylib       	       0x1901950dc _pthread_cond_wait + 984
2   libc++.1.dylib                	       0x1900c574c std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&) + 32
3   mbgl-glfw                     	       0x1054c7100 mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const + 360 (thread_pool.cpp:40)
4   mbgl-glfw                     	       0x1054c6f64 decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&) + 24 (invoke.h:179)
5   mbgl-glfw                     	       0x1054c6f40 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
6   mbgl-glfw                     	       0x1054c6d58 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*) + 84 (thread.h:214)
7   libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
8   libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 10:: org.maplibre.mbgl.OnlineFileSource
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x10567aff4 mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x10567aefc decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x10567aed8 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x10567ac00 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 11:: org.maplibre.mbgl.AssetFileSource
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x1055f98cc mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x1055f97d4 decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x1055f97b0 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x1055f94d8 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 12:: org.maplibre.mbgl.DatabaseFileSource
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x105618850 mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x105618758 decltype(std::declval<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>(std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x105618734 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>, mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x10561845c void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 13:: org.maplibre.mbgl.LocalFileSource
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x1056323b4 mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x1056322bc decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x105632298 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x105631fc0 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 14:: org.maplibre.mbgl.MBTilesFileSource
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x105609f80 mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x105609e88 decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x105609e64 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x105609b8c void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 15:: org.maplibre.mbgl.PMTilesFileSource
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x10569bf50 mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x10569be58 decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x10569be34 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x10569bb5c void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 16:: org.maplibre.mbgl.ResourceLoaderThread
0   libsystem_kernel.dylib        	       0x190151c34 mach_msg2_trap + 8
1   libsystem_kernel.dylib        	       0x190164028 mach_msg2_internal + 76
2   libsystem_kernel.dylib        	       0x19015a98c mach_msg_overwrite + 484
3   libsystem_kernel.dylib        	       0x190151fb4 mach_msg + 24
4   CoreFoundation                	       0x190233c80 __CFRunLoopServiceMachPort + 160
5   CoreFoundation                	       0x1902325d8 __CFRunLoopRun + 1188
6   CoreFoundation                	       0x1902f0898 _CFRunLoopRunSpecificWithOptions + 532
7   CoreFoundation                	       0x190289a44 CFRunLoopRun + 64
8   mbgl-glfw                     	       0x1055d9f94 mbgl::util::RunLoop::run() + 20 (run_loop.cpp:38)
9   mbgl-glfw                     	       0x1056432a0 mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()::operator()() + 196 (thread.hpp:65)
10  mbgl-glfw                     	       0x1056431a8 decltype(std::declval<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>(std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&) + 24 (invoke.h:179)
11  mbgl-glfw                     	       0x105643184 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>, mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
12  mbgl-glfw                     	       0x105642eac void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>>(void*) + 84 (thread.h:214)
13  libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
14  libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 17:: org.maplibre.mbgl.Worker 1
0   libsystem_kernel.dylib        	       0x1901554f8 __psynch_cvwait + 8
1   libsystem_pthread.dylib       	       0x1901950dc _pthread_cond_wait + 984
2   libc++.1.dylib                	       0x1900c574c std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&) + 32
3   mbgl-glfw                     	       0x1054c7100 mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const + 360 (thread_pool.cpp:40)
4   mbgl-glfw                     	       0x1054c6f64 decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&) + 24 (invoke.h:179)
5   mbgl-glfw                     	       0x1054c6f40 void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>) + 28 (thread.h:205)
6   mbgl-glfw                     	       0x1054c6d58 void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*) + 84 (thread.h:214)
7   libsystem_pthread.dylib       	       0x190194c08 _pthread_start + 136
8   libsystem_pthread.dylib       	       0x19018fba8 thread_start + 8

Thread 18:
0   libsystem_kernel.dylib        	       0x190153a60 kevent_id + 8
1   libdispatch.dylib             	       0x18fffbe7c _dispatch_kq_poll + 228
2   libdispatch.dylib             	       0x18fffb334 _dispatch_event_loop_poke + 336
3   CoreAnalytics                 	       0x199901300 CoreAnalytics::Client::sendXpcMessage_sync(applesauce::xpc::dict, XPCMessagePriority, bool, bool) + 252
4   CoreAnalytics                 	       0x19990385c invocation function for block in std::__1::decay<decltype(fp0())>::type applesauce::dispatch::v1::sync_impl<CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict const&, XPCMessagePriority, bool, bool)::$_0>(dispatch_queue_s*, CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict const&, XPCMessagePriority, bool, bool)::$_0&&, std::__1::integral_constant<bool, false>) + 112
5   libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
6   libdispatch.dylib             	       0x18ffe8940 _dispatch_lane_barrier_sync_invoke_and_complete + 56
7   CoreAnalytics                 	       0x1999004b0 CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict const&, XPCMessagePriority, bool, bool) + 152
8   CoreAnalytics                 	       0x1999001d4 CoreAnalytics::Client::sendEventAlways(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>, std::__1::function<nlohmann::basic_json<std::__1::map, std::__1::vector, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>, bool, long long, unsigned long long, double, std::__1::allocator, nlohmann::adl_serializer, std::__1::vector<unsigned char, std::__1::allocator<unsigned char>>> ()> const&, CoreAnalytics::SendEventOptions) + 768
9   CoreAnalytics                 	       0x1998ea350 AnalyticsSendEventInternal(NSString const*, NSDictionary<NSString*, NSObject*>* () block_pointer, XPCMessagePriority, EventPayloadFormat, NSDictionary<NSString*, NSObject*>*) + 644
10  Metal                         	       0x19c3e6a70 __createContextTelemetryDataWithQueueLabelAndCallstack_block_invoke + 848
11  libdispatch.dylib             	       0x18ffd8b5c _dispatch_call_block_and_release + 32
12  libdispatch.dylib             	       0x18fff2ac4 _dispatch_client_callout + 16
13  libdispatch.dylib             	       0x19000f9ec <deduplicated_symbol> + 32
14  libdispatch.dylib             	       0x18ffeb13c _dispatch_root_queue_drain + 736
15  libdispatch.dylib             	       0x18ffeb784 _dispatch_worker_thread2 + 180
16  libsystem_pthread.dylib       	       0x190190e10 _pthread_wqthread + 232
17  libsystem_pthread.dylib       	       0x19018fb9c start_wqthread + 8


Thread 0 crashed with ARM Thread State (64-bit):
    x0: 0x000000019560d028   x1: 0x000000016b6123d0   x2: 0xfffff0007fc00000   x3: 0x0000000000000001
    x4: 0x00000008c607b1c0   x5: 0x0000000000000010   x6: 0xffffffffbfc007ff   x7: 0xfffff0003ffff800
    x8: 0x000000019560d030   x9: 0x0000000000000004  x10: 0x0000000105c2a000  x11: 0x0000000000000002
   x12: 0x0000000000000000  x13: 0x0000000000000000  x14: 0x000000000000e000  x15: 0x0000000000000035
   x16: 0x0000000190152018  x17: 0x00000001fe176690  x18: 0x0000000000000000  x19: 0x000000019560d020
   x20: 0x0000000108f52c90  x21: 0x0000000108f521c0  x22: 0x00000008c75fbf30  x23: 0x00000001055deaec
   x24: 0x00000008c70cc520  x25: 0x00000000141300c9  x26: 0x00000001fe1c8cc8  x27: 0x0000003315a98896
   x28: 0x0000000000000000   fp: 0x000000016b612420   lr: 0x0000000104990cb8
    sp: 0x000000016b612370   pc: 0x00000001048d1a08 cpsr: 0x20001000
   far: 0x000000019560d030  esr: 0x9200004f (Data Abort) byte write Permission fault

Binary Images:
       0x1047e8000 -        0x105be7fff mbgl-glfw (*) <3d5e365b-b059-3651-b8ab-fc2934fd9275> */mbgl-glfw
       0x108430000 -        0x108457fff libglfw.3.4.dylib (*) <adf13a1c-2d7d-3796-af80-ee61fcffee92> /opt/homebrew/*/libglfw.3.4.dylib
       0x108478000 -        0x10849bfff libpng16.16.dylib (*) <1d486868-3c8c-321e-9727-3a5ba5cd220e> /opt/homebrew/*/libpng16.16.dylib
       0x1085e4000 -        0x1086a3fff libharfbuzz.0.dylib (*) <3d359291-b602-39fc-8112-8876e0ebaec3> /opt/homebrew/*/libharfbuzz.0.dylib
       0x1087bc000 -        0x108837fff libfreetype.6.dylib (*) <cfd169d0-526f-3a45-b5db-836906fec3d7> /opt/homebrew/*/libfreetype.6.dylib
       0x1089ac000 -        0x108aa7fff libglib-2.0.0.dylib (*) <6bee9501-347f-38e9-912c-4d8d075e5ed8> /opt/homebrew/*/libglib-2.0.0.dylib
       0x1084ac000 -        0x1084bffff libgraphite2.3.2.1.dylib (*) <1bbb20c8-b0d3-318a-a229-f81c8a8dd304> /opt/homebrew/*/libgraphite2.3.2.1.dylib
       0x1084d0000 -        0x1084f7fff libintl.8.dylib (*) <9414c162-5ccc-3b6d-8a0d-c7c743f7fec1> /opt/homebrew/*/libintl.8.dylib
       0x10885c000 -        0x1088d7fff libpcre2-8.0.dylib (*) <a9da24ef-74d4-397b-ab16-1d18e55a653b> /opt/homebrew/*/libpcre2-8.0.dylib
       0x108998000 -        0x1089a3fff libobjc-trampolines.dylib (*) <580e4b29-8304-342d-a21c-2a869364dc35> /usr/lib/libobjc-trampolines.dylib
       0x121f30000 -        0x12267bfff com.apple.AGXMetalG13X (340.26.3) <e09c4f6b-8ff6-3edf-9e21-95f237f5a392> /System/Library/Extensions/AGXMetalG13X.bundle/Contents/MacOS/AGXMetalG13X
       0x1901d4000 -        0x190720f7f com.apple.CoreFoundation (6.9) <edb39786-80b1-3bff-b6c3-e33f2e3ca867> /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
       0x19cb6e000 -        0x19ce706ff com.apple.HIToolbox (2.1.1) <f369023f-1a24-3f3f-9056-6f5248071e8c> /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/HIToolbox
       0x194601000 -        0x195cfcd9f com.apple.AppKit (6.9) <6d84c7a3-ee98-3daf-ac66-cd95a902ba84> /System/Library/Frameworks/AppKit.framework/Versions/C/AppKit
       0x18fdcd000 -        0x18fe6bf73 dyld (*) <abfd3247-50ac-3c8e-b72a-83710166e982> /usr/lib/dyld
               0x0 - 0xffffffffffffffff ??? (*) <00000000-0000-0000-0000-000000000000> ???
       0x190151000 -        0x19018d45f libsystem_kernel.dylib (*) <2eb73bf1-8c71-3e1f-a160-6da83dc82606> /usr/lib/system/libsystem_kernel.dylib
       0x18ffd7000 -        0x19001de9f libdispatch.dylib (*) <17d849c6-a785-3dbb-bfb5-8321706c4b8d> /usr/lib/system/libdispatch.dylib
       0x18fe70000 -        0x18febcb1f libxpc.dylib (*) <0cec3289-f166-35f6-8cef-de3d17eb0228> /usr/lib/system/libxpc.dylib
       0x19c3e2000 -        0x19c6c51ff com.apple.Metal (370.63.1) <1941ba9c-cd02-3c59-8f38-d54843e73f5a> /System/Library/Frameworks/Metal.framework/Versions/A/Metal
       0x19018e000 -        0x19019aabb libsystem_pthread.dylib (*) <5d31d65c-2ecf-36da-84f5-ba4caab06adb> /usr/lib/system/libsystem_pthread.dylib
       0x1944a8000 -        0x19459233f com.apple.framework.IOKit (2.0.2) <41408ce6-4525-3c70-b469-9830fb87e90a> /System/Library/Frameworks/IOKit.framework/Versions/A/IOKit
       0x1b4540000 -        0x1b4590c1f com.apple.IOGPU (128.3) <daa8e305-606c-3908-a210-3e8781a9a911> /System/Library/PrivateFrameworks/IOGPU.framework/Versions/A/IOGPU
       0x1900a3000 -        0x190135e53 libc++.1.dylib (*) <21dfdb11-8328-31d9-a35d-08c1b52621a6> /usr/lib/libc++.1.dylib
       0x1998e3000 -        0x199912cdf com.apple.analyticsd (1.0) <7568a9c7-aa6a-3f6b-91c3-02c822bf6425> /System/Library/PrivateFrameworks/CoreAnalytics.framework/Versions/A/CoreAnalytics

External Modification Summary:
  Calls made by other processes targeting this process:
    task_for_pid: 0
    thread_create: 0
    thread_set_state: 0
  Calls made by this process:
    task_for_pid: 0
    thread_create: 0
    thread_set_state: 0
  Calls made by all processes on this machine:
    task_for_pid: 0
    thread_create: 0
    thread_set_state: 0

-----------
Full Report
-----------

{"app_name":"mbgl-glfw","timestamp":"2025-09-18 21:58:00.00 +0200","app_version":"","slice_uuid":"3d5e365b-b059-3651-b8ab-fc2934fd9275","build_version":"","platform":1,"share_with_app_devs":0,"is_first_party":1,"bug_type":"309","os_version":"macOS 26.0 (25A354)","roots_installed":0,"incident_id":"225D49FD-72A1-4059-93F2-387A587D69B6","name":"mbgl-glfw"}
{
  "uptime" : 9100,
  "procRole" : "Foreground",
  "version" : 2,
  "userID" : 501,
  "deployVersion" : 210,
  "modelCode" : "MacBookPro18,2",
  "coalitionID" : 2369,
  "osVersion" : {
    "train" : "macOS 26.0",
    "build" : "25A354",
    "releaseType" : "User"
  },
  "captureTime" : "2025-09-18 21:57:56.9982 +0200",
  "codeSigningMonitor" : 1,
  "incident" : "225D49FD-72A1-4059-93F2-387A587D69B6",
  "pid" : 67613,
  "translated" : false,
  "cpuType" : "ARM-64",
  "roots_installed" : 0,
  "bug_type" : "309",
  "procLaunch" : "2025-09-18 21:57:56.4289 +0200",
  "procStartAbsTime" : 219401903595,
  "procExitAbsTime" : 219415355429,
  "procName" : "mbgl-glfw",
  "procPath" : "\/Users\/USER\/*\/mbgl-glfw",
  "parentProc" : "bash",
  "parentPid" : 67607,
  "coalitionName" : "com.microsoft.VSCode",
  "crashReporterKey" : "FF5D4141-568B-5C72-35D6-246897AC305D",
  "developerMode" : 1,
  "responsiblePid" : 4953,
  "responsibleProc" : "Electron",
  "codeSigningID" : "mbgl-glfw",
  "codeSigningTeamID" : "",
  "codeSigningFlags" : 570556929,
  "codeSigningValidationCategory" : 10,
  "codeSigningTrustLevel" : 4294967295,
  "codeSigningAuxiliaryInfo" : 0,
  "instructionByteStream" : {"beforePC":"CAVAkigRKPgABUCSwANf1gggAJEIMSH4AAVAksADX9YIIACRiQCAUg==","atPC":"CAEp+MADX9YIIACRaQCAkggBafgfHQDxqAAAVL85A9UIAED5AQlA+Q=="},
  "bootSessionUUID" : "42538590-54A5-48FE-983A-9148D9FD2505",
  "sleepWakeUUID" : "BECD6912-2E0F-405F-B474-B5D83C554401",
  "sip" : "enabled",
  "vmRegionInfo" : "0x19560d030 is in 0x194601000-0x195cfd000;  bytes after start: 16826416  bytes before end: 7274447\n      REGION TYPE                    START - END         [ VSIZE] PRT\/MAX SHRMOD  REGION DETAIL\n      __TEXT                      1945a5000-194601000    [  368K] r-x\/r-x SM=COW  \/usr\/lib\/libsandbox.1.dylib\n--->  __TEXT                      194601000-195cfd000    [ 23.0M] r-x\/r-x SM=COW  \/System\/Library\/Frameworks\/AppKit.framework\/Versions\/C\/AppKit\n      __TEXT                      195cfd000-195eb7000    [ 1768K] r-x\/r-x SM=COW  \/System\/Library\/PrivateFrameworks\/UIFoundation.framework\/Versions\/A\/UIFoundation",
  "exception" : {"codes":"0x0000000000000002, 0x000000019560d030","rawCodes":[2,6801117232],"type":"EXC_BAD_ACCESS","signal":"SIGBUS","subtype":"KERN_PROTECTION_FAILURE at 0x000000019560d030"},
  "termination" : {"flags":1024,"code":10,"namespace":"SIGNAL","indicator":"Bus error: 10","byProc":"exc handler","byPid":67613},
  "vmregioninfo" : "0x19560d030 is in 0x194601000-0x195cfd000;  bytes after start: 16826416  bytes before end: 7274447\n      REGION TYPE                    START - END         [ VSIZE] PRT\/MAX SHRMOD  REGION DETAIL\n      __TEXT                      1945a5000-194601000    [  368K] r-x\/r-x SM=COW  \/usr\/lib\/libsandbox.1.dylib\n--->  __TEXT                      194601000-195cfd000    [ 23.0M] r-x\/r-x SM=COW  \/System\/Library\/Frameworks\/AppKit.framework\/Versions\/C\/AppKit\n      __TEXT                      195cfd000-195eb7000    [ 1768K] r-x\/r-x SM=COW  \/System\/Library\/PrivateFrameworks\/UIFoundation.framework\/Versions\/A\/UIFoundation",
  "extMods" : {"caller":{"thread_create":0,"thread_set_state":0,"task_for_pid":0},"system":{"thread_create":0,"thread_set_state":0,"task_for_pid":0},"targeted":{"thread_create":0,"thread_set_state":0,"task_for_pid":0},"warnings":0},
  "faultingThread" : 0,
  "threads" : [{"triggered":true,"id":338002,"threadState":{"x":[{"value":6801117224,"symbolLocation":8,"symbol":"+[NSView(NSDeclarativeConstraintCreation) keyPathsForValuesAffectingAlignmentRectInsets]"},{"value":6096495568},{"value":18446726483666796544},{"value":1},{"value":37682131392},{"value":16},{"value":18446744072631617535},{"value":18446726482597246976},{"value":6801117232,"symbolLocation":4,"symbol":"NSViewFramesApplyAncestorScalingDefaultValueFunction"},{"value":4},{"value":4391608320,"symbolLocation":336,"symbol":"boost::detail::crc_table_t<32, 8, 79764919ul, true>::get_table()::table"},{"value":2},{"value":0},{"value":0},{"value":57344},{"value":53},{"value":6712270872,"symbolLocation":0,"symbol":"mach_continuous_time"},{"value":8557913744},{"value":0},{"value":6801117216,"symbolLocation":0,"symbol":"+[NSView(NSDeclarativeConstraintCreation) keyPathsForValuesAffectingAlignmentRectInsets]"},{"value":4445252752},{"value":4445249984},{"value":37704679216},{"sourceLine":33,"value":4385008364,"sourceFile":"timer.cpp","symbol":"mbgl::util::Timer::Impl::perform(__CFRunLoopTimer*, void*)","symbolLocation":0},{"value":37699241248},{"value":336789705},{"value":8558251208,"symbolLocation":0,"symbol":"kCFTypeArrayCallBacks"},{"value":219406764182},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":4372106424},"cpsr":{"value":536875008},"fp":{"value":6096495648},"sp":{"value":6096495472},"esr":{"value":2449473615,"description":"(Data Abort) byte write Permission fault"},"pc":{"value":4371323400,"matchesCrashFrame":1},"far":{"value":6801117232}},"frames":[{"imageOffset":956936,"symbol":"dawn::RefCounted::AddRef()","symbolLocation":8,"imageIndex":0},{"imageOffset":1739960,"symbol":"dawn::native::DeviceBase::APITick()","symbolLocation":52,"imageIndex":0},{"imageOffset":898912,"sourceLine":429,"sourceFile":"glfw_webgpu_backend.cpp","symbol":"GLFWWebGPUBackend::swap()","imageIndex":0,"symbolLocation":364},{"imageOffset":313336,"sourceLine":1183,"sourceFile":"glfw_view.cpp","symbol":"GLFWView::render()","imageIndex":0,"symbolLocation":280},{"imageOffset":448012,"sourceLine":1216,"sourceFile":"glfw_view.cpp","symbol":"GLFWView::run()::$_0::operator()() const","imageIndex":0,"symbolLocation":72},{"imageOffset":447928,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<GLFWView::run()::$_0&>()()) std::__1::__invoke[abi:ne200100]<GLFWView::run()::$_0&>(GLFWView::run()::$_0&)","imageIndex":0,"symbolLocation":24},{"imageOffset":447892,"sourceLine":251,"sourceFile":"invoke.h","symbol":"void std::__1::__invoke_void_return_wrapper<void, true>::__call[abi:ne200100]<GLFWView::run()::$_0&>(GLFWView::run()::$_0&)","imageIndex":0,"symbolLocation":24},{"imageOffset":447856,"sourceLine":273,"sourceFile":"invoke.h","symbol":"void std::__1::__invoke_r[abi:ne200100]<void, GLFWView::run()::$_0&>(GLFWView::run()::$_0&)","imageIndex":0,"symbolLocation":24},{"imageOffset":447820,"sourceLine":167,"sourceFile":"function.h","symbol":"std::__1::__function::__alloc_func<GLFWView::run()::$_0, std::__1::allocator<GLFWView::run()::$_0>, void ()>::operator()[abi:ne200100]()","imageIndex":0,"symbolLocation":24},{"imageOffset":445972,"sourceLine":319,"sourceFile":"function.h","symbol":"std::__1::__function::__func<GLFWView::run()::$_0, std::__1::allocator<GLFWView::run()::$_0>, void ()>::operator()()","imageIndex":0,"symbolLocation":28},{"imageOffset":395372,"sourceLine":436,"sourceFile":"function.h","symbol":"std::__1::__function::__value_func<void ()>::operator()[abi:ne200100]() const","imageIndex":0,"symbolLocation":60},{"imageOffset":292164,"sourceLine":995,"sourceFile":"function.h","symbol":"std::__1::function<void ()>::operator()() const","imageIndex":0,"symbolLocation":24},{"imageOffset":14641928,"sourceLine":33,"sourceFile":"timer.cpp","symbol":"mbgl::util::Timer::Impl::perform(__CFRunLoopTimer*, void*)","imageIndex":0,"symbolLocation":28},{"imageOffset":451216,"symbol":"__CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__","symbolLocation":32,"imageIndex":11},{"imageOffset":450384,"symbol":"__CFRunLoopDoTimer","symbolLocation":980,"imageIndex":11},{"imageOffset":449224,"symbol":"__CFRunLoopDoTimers","symbolLocation":280,"imageIndex":11},{"imageOffset":387148,"symbol":"__CFRunLoopRun","symbolLocation":1816,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":792368,"symbol":"RunCurrentEventLoopInMode","symbolLocation":316,"imageIndex":12},{"imageOffset":805328,"symbol":"ReceiveNextEventCommon","symbolLocation":488,"imageIndex":12},{"imageOffset":2417140,"symbol":"_BlockUntilNextEventMatchingListInMode","symbolLocation":48,"imageIndex":12},{"imageOffset":5280520,"symbol":"_DPSBlockUntilNextEventMatchingListInMode","symbolLocation":236,"imageIndex":13},{"imageOffset":130780,"symbol":"_DPSNextEvent","symbolLocation":588,"imageIndex":13},{"imageOffset":10955268,"symbol":"-[NSApplication(NSEventRouting) _nextEventMatchingEventMask:untilDate:inMode:dequeue:]","symbolLocation":688,"imageIndex":13},{"imageOffset":10954512,"symbol":"-[NSApplication(NSEventRouting) nextEventMatchingMask:untilDate:inMode:dequeue:]","symbolLocation":72,"imageIndex":13},{"imageOffset":80612,"symbol":"_glfwPollEventsCocoa","symbolLocation":140,"imageIndex":1},{"imageOffset":315012,"sourceLine":1236,"sourceFile":"glfw_view.cpp","symbol":"GLFWView::run()","imageIndex":0,"symbolLocation":244},{"imageOffset":41120,"sourceLine":214,"sourceFile":"main.cpp","symbol":"main","imageIndex":0,"symbolLocation":12484},{"imageOffset":36180,"symbol":"start","symbolLocation":7184,"imageIndex":14}]},{"id":338006,"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":119132,"symbol":"_dispatch_mach_send_and_wait_for_reply","symbolLocation":548,"imageIndex":17},{"imageOffset":120060,"symbol":"dispatch_mach_send_with_result_and_wait_for_reply","symbolLocation":60,"imageIndex":17},{"imageOffset":47716,"symbol":"xpc_connection_send_message_with_reply_sync","symbolLocation":284,"imageIndex":18},{"imageOffset":198744,"symbol":"MTLXPCCompilerConnection::checkConnectionAlive()","symbolLocation":188,"imageIndex":19},{"imageOffset":1857912,"symbol":"invocation function for block in MTLSchedulerRequest::generateXPCBlock(qos_class_t, int)","symbolLocation":1012,"imageIndex":19},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":27700,"symbol":"_dispatch_block_invoke_direct","symbolLocation":284,"imageIndex":17},{"imageOffset":1618328,"symbol":"invocation function for block in MTLCompilerScheduler::createBlockWithData(std::__1::shared_ptr<MTLSchedulerRequest>)","symbolLocation":48,"imageIndex":19},{"imageOffset":70864,"symbol":"_dispatch_block_async_invoke2","symbolLocation":148,"imageIndex":17},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":42216,"symbol":"_dispatch_lane_serial_drain","symbolLocation":740,"imageIndex":17},{"imageOffset":45048,"symbol":"_dispatch_lane_invoke","symbolLocation":440,"imageIndex":17},{"imageOffset":87156,"symbol":"_dispatch_root_queue_drain_deferred_wlh","symbolLocation":292,"imageIndex":17},{"imageOffset":85356,"symbol":"_dispatch_workloop_worker_thread","symbolLocation":692,"imageIndex":17},{"imageOffset":11852,"symbol":"_pthread_wqthread","symbolLocation":292,"imageIndex":20},{"imageOffset":7068,"symbol":"start_wqthread","symbolLocation":8,"imageIndex":20}],"threadState":{"x":[{"value":0},{"value":17297326606},{"value":0},{"value":77059},{"value":0},{"value":41794326757376},{"value":16384},{"value":0},{"value":0},{"value":17179869184},{"value":16384},{"value":0},{"value":0},{"value":0},{"value":9731},{"value":1},{"value":18446744073709551569},{"value":8557913872},{"value":0},{"value":0},{"value":16384},{"value":41794326757376},{"value":0},{"value":77059},{"value":6097626192},{"value":0},{"value":17297326606},{"value":18446744073709550527},{"value":117457422}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6097625856},"sp":{"value":6097625776},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}}},{"id":338007,"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":119132,"symbol":"_dispatch_mach_send_and_wait_for_reply","symbolLocation":548,"imageIndex":17},{"imageOffset":120060,"symbol":"dispatch_mach_send_with_result_and_wait_for_reply","symbolLocation":60,"imageIndex":17},{"imageOffset":47716,"symbol":"xpc_connection_send_message_with_reply_sync","symbolLocation":284,"imageIndex":18},{"imageOffset":198744,"symbol":"MTLXPCCompilerConnection::checkConnectionAlive()","symbolLocation":188,"imageIndex":19},{"imageOffset":1857912,"symbol":"invocation function for block in MTLSchedulerRequest::generateXPCBlock(qos_class_t, int)","symbolLocation":1012,"imageIndex":19},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":27700,"symbol":"_dispatch_block_invoke_direct","symbolLocation":284,"imageIndex":17},{"imageOffset":1618328,"symbol":"invocation function for block in MTLCompilerScheduler::createBlockWithData(std::__1::shared_ptr<MTLSchedulerRequest>)","symbolLocation":48,"imageIndex":19},{"imageOffset":70864,"symbol":"_dispatch_block_async_invoke2","symbolLocation":148,"imageIndex":17},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":42216,"symbol":"_dispatch_lane_serial_drain","symbolLocation":740,"imageIndex":17},{"imageOffset":45048,"symbol":"_dispatch_lane_invoke","symbolLocation":440,"imageIndex":17},{"imageOffset":87156,"symbol":"_dispatch_root_queue_drain_deferred_wlh","symbolLocation":292,"imageIndex":17},{"imageOffset":85356,"symbol":"_dispatch_workloop_worker_thread","symbolLocation":692,"imageIndex":17},{"imageOffset":11852,"symbol":"_pthread_wqthread","symbolLocation":292,"imageIndex":20},{"imageOffset":7068,"symbol":"start_wqthread","symbolLocation":8,"imageIndex":20}],"threadState":{"x":[{"value":0},{"value":17297326606},{"value":0},{"value":78851},{"value":0},{"value":83678847827968},{"value":16384},{"value":0},{"value":0},{"value":17179869184},{"value":16384},{"value":0},{"value":0},{"value":0},{"value":19483},{"value":1},{"value":18446744073709551569},{"value":8557913872},{"value":0},{"value":0},{"value":16384},{"value":83678847827968},{"value":0},{"value":78851},{"value":6097052752},{"value":0},{"value":17297326606},{"value":18446744073709550527},{"value":117457422}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6097052416},"sp":{"value":6097052336},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}}},{"id":338022,"frames":[{"imageOffset":127124,"symbol":"iokit_user_client_trap","symbolLocation":8,"imageIndex":21},{"imageOffset":118068,"symbol":"IOGPUCommandQueueSubmitCommandBuffers","symbolLocation":180,"imageIndex":22},{"imageOffset":23336,"symbol":"-[IOGPUMetalCommandQueue _submitCommandBuffers:count:]","symbolLocation":364,"imageIndex":22},{"imageOffset":22928,"symbol":"-[IOGPUMetalCommandQueue submitCommandBuffers:count:]","symbolLocation":72,"imageIndex":22},{"imageOffset":89412,"symbol":"-[_MTLCommandQueue _submitAvailableCommandBuffers]","symbolLocation":512,"imageIndex":19},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":26340,"symbol":"_dispatch_continuation_pop","symbolLocation":596,"imageIndex":17},{"imageOffset":104448,"symbol":"_dispatch_source_latch_and_call","symbolLocation":396,"imageIndex":17},{"imageOffset":99540,"symbol":"_dispatch_source_invoke","symbolLocation":844,"imageIndex":17},{"imageOffset":41808,"symbol":"_dispatch_lane_serial_drain","symbolLocation":332,"imageIndex":17},{"imageOffset":44996,"symbol":"_dispatch_lane_invoke","symbolLocation":388,"imageIndex":17},{"imageOffset":87156,"symbol":"_dispatch_root_queue_drain_deferred_wlh","symbolLocation":292,"imageIndex":17},{"imageOffset":85356,"symbol":"_dispatch_workloop_worker_thread","symbolLocation":692,"imageIndex":17},{"imageOffset":11852,"symbol":"_pthread_wqthread","symbolLocation":292,"imageIndex":20},{"imageOffset":7068,"symbol":"start_wqthread","symbolLocation":8,"imageIndex":20}],"threadState":{"x":[{"value":0},{"value":0},{"value":2},{"value":64},{"value":6098215296},{"value":6098215428},{"value":0},{"value":0},{"value":37691261968},{"value":18446744073709551615},{"value":4445233840},{"value":8534768112,"symbolLocation":0,"symbol":"__CFRuntimeClassTables"},{"value":20},{"value":0},{"value":6918396941,"symbolLocation":141,"symbol":"_OBJC_$_INSTANCE_METHODS__MTLCommandBuffer"},{"value":8535410744,"symbolLocation":0,"symbol":"OBJC_CLASS_$__MTLCommandBuffer"},{"value":18446744073709551516},{"value":8557297248},{"value":0},{"value":37692183408},{"value":6098215428},{"value":6098215296},{"value":2},{"value":64},{"value":3758097090},{"value":1},{"value":0},{"value":112},{"value":32}],"flavor":"ARM_THREAD_STATE64","lr":{"value":7320489268},"cpsr":{"value":1610616832},"fp":{"value":6098215280},"sp":{"value":6098215136},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6783004820},"far":{"value":0}}},{"id":338023,"frames":[],"threadState":{"x":[{"value":6098792448},{"value":130307},{"value":6098255872},{"value":6098791296},{"value":5193730},{"value":1},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":0},"cpsr":{"value":4096},"fp":{"value":0},"sp":{"value":6098791280},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712523668},"far":{"value":0}}},{"id":338024,"name":"com.apple.NSEventThread","threadState":{"x":[{"value":0},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":131954280235008},{"value":0},{"value":131954280235008},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":30723},{"value":0},{"value":18446744073709551569},{"value":8557913872},{"value":0},{"value":4294967295},{"value":2},{"value":131954280235008},{"value":0},{"value":131954280235008},{"value":6099361928},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6099361776},"sp":{"value":6099361696},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":719464,"symbol":"_NSEventThread","symbolLocation":184,"imageIndex":13},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338059,"name":"org.maplibre.mbgl.Worker 1","threadState":{"x":[{"value":4},{"value":0},{"value":0},{"value":37705495672},{"value":2199023256064},{"value":4256},{"value":0},{"value":0},{"value":6099938488},{"value":512},{"value":512},{"value":1099511628290},{"value":1099511628290},{"value":512},{"value":256},{"value":2199023256064},{"value":305},{"value":8557912000},{"value":0},{"value":37705495672},{"value":37705495624},{"value":6099939552},{"value":0},{"value":0},{"value":0},{"value":0},{"value":512},{"value":0},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712545500},"cpsr":{"value":1610616832},"fp":{"value":6099938608},"sp":{"value":6099938464},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712284408},"far":{"value":0}},"frames":[{"imageOffset":17656,"symbol":"__psynch_cvwait","symbolLocation":8,"imageIndex":16},{"imageOffset":28892,"symbol":"_pthread_cond_wait","symbolLocation":984,"imageIndex":20},{"imageOffset":141132,"symbol":"std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&)","symbolLocation":32,"imageIndex":23},{"imageOffset":13496576,"sourceLine":40,"sourceFile":"thread_pool.cpp","symbol":"mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const","imageIndex":0,"symbolLocation":360},{"imageOffset":13496164,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":13496128,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":13495640,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338060,"name":"org.maplibre.mbgl.Worker 2","threadState":{"x":[{"value":4},{"value":0},{"value":0},{"value":37705495672},{"value":1099511628288},{"value":4256},{"value":0},{"value":0},{"value":6100511928},{"value":256},{"value":512},{"value":514},{"value":514},{"value":256},{"value":512},{"value":1099511628288},{"value":305},{"value":8557912000},{"value":0},{"value":37705495672},{"value":37705495624},{"value":6100512992},{"value":0},{"value":0},{"value":0},{"value":1},{"value":256},{"value":0},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712545500},"cpsr":{"value":1610616832},"fp":{"value":6100512048},"sp":{"value":6100511904},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712284408},"far":{"value":0}},"frames":[{"imageOffset":17656,"symbol":"__psynch_cvwait","symbolLocation":8,"imageIndex":16},{"imageOffset":28892,"symbol":"_pthread_cond_wait","symbolLocation":984,"imageIndex":20},{"imageOffset":141132,"symbol":"std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&)","symbolLocation":32,"imageIndex":23},{"imageOffset":13496576,"sourceLine":40,"sourceFile":"thread_pool.cpp","symbol":"mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const","imageIndex":0,"symbolLocation":360},{"imageOffset":13496164,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":13496128,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":13495640,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338061,"name":"org.maplibre.mbgl.Worker 3","threadState":{"x":[{"value":260},{"value":0},{"value":0},{"value":0},{"value":0},{"value":160},{"value":0},{"value":0},{"value":6101085368},{"value":0},{"value":512},{"value":2199023256066},{"value":2199023256066},{"value":512},{"value":0},{"value":2199023256064},{"value":305},{"value":8557912000},{"value":0},{"value":37705495672},{"value":37705495624},{"value":6101086432},{"value":0},{"value":0},{"value":0},{"value":0},{"value":768},{"value":0},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712545500},"cpsr":{"value":1610616832},"fp":{"value":6101085488},"sp":{"value":6101085344},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712284408},"far":{"value":0}},"frames":[{"imageOffset":17656,"symbol":"__psynch_cvwait","symbolLocation":8,"imageIndex":16},{"imageOffset":28892,"symbol":"_pthread_cond_wait","symbolLocation":984,"imageIndex":20},{"imageOffset":141132,"symbol":"std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&)","symbolLocation":32,"imageIndex":23},{"imageOffset":13496576,"sourceLine":40,"sourceFile":"thread_pool.cpp","symbol":"mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const","imageIndex":0,"symbolLocation":360},{"imageOffset":13496164,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":13496128,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":13495640,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338062,"name":"org.maplibre.mbgl.Worker 4","threadState":{"x":[{"value":4},{"value":0},{"value":0},{"value":0},{"value":0},{"value":160},{"value":0},{"value":0},{"value":6101658808},{"value":0},{"value":512},{"value":2199023256066},{"value":2199023256066},{"value":512},{"value":0},{"value":2199023256064},{"value":305},{"value":8557912000},{"value":0},{"value":37705495672},{"value":37705495624},{"value":6101659872},{"value":0},{"value":0},{"value":0},{"value":0},{"value":1024},{"value":0},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712545500},"cpsr":{"value":1610616832},"fp":{"value":6101658928},"sp":{"value":6101658784},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712284408},"far":{"value":0}},"frames":[{"imageOffset":17656,"symbol":"__psynch_cvwait","symbolLocation":8,"imageIndex":16},{"imageOffset":28892,"symbol":"_pthread_cond_wait","symbolLocation":984,"imageIndex":20},{"imageOffset":141132,"symbol":"std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&)","symbolLocation":32,"imageIndex":23},{"imageOffset":13496576,"sourceLine":40,"sourceFile":"thread_pool.cpp","symbol":"mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const","imageIndex":0,"symbolLocation":360},{"imageOffset":13496164,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":13496128,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":13495640,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338064,"name":"org.maplibre.mbgl.OnlineFileSource","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":269393233707008},{"value":0},{"value":269393233707008},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":62723},{"value":0},{"value":18446744073709551569},{"value":8557913872},{"value":0},{"value":4294967295},{"value":2},{"value":269393233707008},{"value":0},{"value":269393233707008},{"value":6102228584},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6102228432},"sp":{"value":6102228352},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":15282164,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":15281916,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":15281880,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":15281152,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::OnlineFileSourceThread>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338068,"name":"org.maplibre.mbgl.AssetFileSource","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":264995187195904},{"value":0},{"value":264995187195904},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":61699},{"value":37705695232},{"value":18446744073709551569},{"value":18446744072367376383},{"value":0},{"value":4294967295},{"value":2},{"value":264995187195904},{"value":0},{"value":264995187195904},{"value":6102802024},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6102801872},"sp":{"value":6102801792},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":14751948,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":14751700,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":14751664,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":14750936,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::AssetFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338069,"name":"org.maplibre.mbgl.DatabaseFileSource","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":261696652312576},{"value":0},{"value":261696652312576},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":60931},{"value":0},{"value":18446744073709551569},{"value":8557913872},{"value":0},{"value":4294967295},{"value":2},{"value":261696652312576},{"value":0},{"value":261696652312576},{"value":6103375464},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6103375312},"sp":{"value":6103375232},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":14878800,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":14878552,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>(std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":14878516,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>, mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":14877788,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::DatabaseFileSourceThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338070,"name":"org.maplibre.mbgl.LocalFileSource","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":207820582551552},{"value":0},{"value":207820582551552},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":48387},{"value":37705695232},{"value":18446744073709551569},{"value":18446744072367376383},{"value":0},{"value":4294967295},{"value":2},{"value":207820582551552},{"value":0},{"value":207820582551552},{"value":6103948904},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6103948752},"sp":{"value":6103948672},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":14984116,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":14983868,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":14983832,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":14983104,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::LocalFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338071,"name":"org.maplibre.mbgl.MBTilesFileSource","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":212218629062656},{"value":0},{"value":212218629062656},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":49411},{"value":37705695232},{"value":18446744073709551569},{"value":18446744072367376383},{"value":0},{"value":4294967295},{"value":2},{"value":212218629062656},{"value":0},{"value":212218629062656},{"value":6104522344},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6104522192},"sp":{"value":6104522112},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":14819200,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":14818952,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":14818916,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":14818188,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MBTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338072,"name":"org.maplibre.mbgl.PMTilesFileSource","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":255099582545920},{"value":0},{"value":255099582545920},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":59395},{"value":37705695232},{"value":18446744073709551569},{"value":18446744072367376383},{"value":0},{"value":4294967295},{"value":2},{"value":255099582545920},{"value":0},{"value":255099582545920},{"value":6105095784},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6105095632},"sp":{"value":6105095552},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":15417168,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":15416920,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":15416884,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>, mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":15416156,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::PMTilesFileSource::Impl>::Thread<std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<mbgl::ResourceOptions, mbgl::ClientOptions>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338073,"name":"org.maplibre.mbgl.ResourceLoaderThread","threadState":{"x":[{"value":268451845},{"value":21592279046},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":251801047662592},{"value":0},{"value":251801047662592},{"value":2},{"value":4294967295},{"value":0},{"value":17179869184},{"value":0},{"value":2},{"value":0},{"value":0},{"value":58627},{"value":0},{"value":18446744073709551569},{"value":8557913872},{"value":0},{"value":4294967295},{"value":2},{"value":251801047662592},{"value":0},{"value":251801047662592},{"value":6105669224},{"value":8589934592,"symbolLocation":56,"symbol":"__block_descriptor_48_e8_32r40r_e20_v24\u0001?0Q8\u0001\"NSError\"16l"},{"value":21592279046},{"value":18446744073709550527},{"value":4412409862}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712344616},"cpsr":{"value":4096},"fp":{"value":6105669072},"sp":{"value":6105668992},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712269876},"far":{"value":0}},"frames":[{"imageOffset":3124,"symbol":"mach_msg2_trap","symbolLocation":8,"imageIndex":16},{"imageOffset":77864,"symbol":"mach_msg2_internal","symbolLocation":76,"imageIndex":16},{"imageOffset":39308,"symbol":"mach_msg_overwrite","symbolLocation":484,"imageIndex":16},{"imageOffset":4020,"symbol":"mach_msg","symbolLocation":24,"imageIndex":16},{"imageOffset":392320,"symbol":"__CFRunLoopServiceMachPort","symbolLocation":160,"imageIndex":11},{"imageOffset":386520,"symbol":"__CFRunLoopRun","symbolLocation":1188,"imageIndex":11},{"imageOffset":1165464,"symbol":"_CFRunLoopRunSpecificWithOptions","symbolLocation":532,"imageIndex":11},{"imageOffset":744004,"symbol":"CFRunLoopRun","symbolLocation":64,"imageIndex":11},{"imageOffset":14622612,"sourceLine":38,"sourceFile":"run_loop.cpp","symbol":"mbgl::util::RunLoop::run()","imageIndex":0,"symbolLocation":20},{"imageOffset":15053472,"sourceLine":65,"sourceFile":"thread.hpp","symbol":"mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()::operator()()","imageIndex":0,"symbolLocation":196},{"imageOffset":15053224,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>()()) std::__1::__invoke[abi:ne200100]<mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>(std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":15053188,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>(std::__1::tuple<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>, mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":15052460,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::util::Thread<mbgl::MainResourceLoaderThread>::Thread<std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>>(std::__1::function<void ()>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::tuple<std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>, std::__1::shared_ptr<mbgl::FileSource>>&&)::'lambda'()>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338090,"name":"org.maplibre.mbgl.Worker 1","threadState":{"x":[{"value":260},{"value":0},{"value":768},{"value":0},{"value":0},{"value":160},{"value":0},{"value":0},{"value":6106246328},{"value":0},{"value":0},{"value":2},{"value":2},{"value":0},{"value":0},{"value":0},{"value":305},{"value":8557912000},{"value":0},{"value":37693948536},{"value":37693948488},{"value":6106247392},{"value":0},{"value":0},{"value":768},{"value":769},{"value":1024},{"value":0},{"value":0}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6712545500},"cpsr":{"value":1610616832},"fp":{"value":6106246448},"sp":{"value":6106246304},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712284408},"far":{"value":0}},"frames":[{"imageOffset":17656,"symbol":"__psynch_cvwait","symbolLocation":8,"imageIndex":16},{"imageOffset":28892,"symbol":"_pthread_cond_wait","symbolLocation":984,"imageIndex":20},{"imageOffset":141132,"symbol":"std::__1::condition_variable::wait(std::__1::unique_lock<std::__1::mutex>&)","symbolLocation":32,"imageIndex":23},{"imageOffset":13496576,"sourceLine":40,"sourceFile":"thread_pool.cpp","symbol":"mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0::operator()() const","imageIndex":0,"symbolLocation":360},{"imageOffset":13496164,"sourceLine":179,"sourceFile":"invoke.h","symbol":"decltype(std::declval<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>()()) std::__1::__invoke[abi:ne200100]<mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0&&)","imageIndex":0,"symbolLocation":24},{"imageOffset":13496128,"sourceLine":205,"sourceFile":"thread.h","symbol":"void std::__1::__thread_execute[abi:ne200100]<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>(std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>&, std::__1::__tuple_indices<>)","imageIndex":0,"symbolLocation":28},{"imageOffset":13495640,"sourceLine":214,"sourceFile":"thread.h","symbol":"void* std::__1::__thread_proxy[abi:ne200100]<std::__1::tuple<std::__1::unique_ptr<std::__1::__thread_struct, std::__1::default_delete<std::__1::__thread_struct>>, mbgl::ThreadedSchedulerBase::makeSchedulerThread(unsigned long)::$_0>>(void*)","imageIndex":0,"symbolLocation":84},{"imageOffset":27656,"symbol":"_pthread_start","symbolLocation":136,"imageIndex":20},{"imageOffset":7080,"symbol":"thread_start","symbolLocation":8,"imageIndex":20}]},{"id":338109,"frames":[{"imageOffset":10848,"symbol":"kevent_id","symbolLocation":8,"imageIndex":16},{"imageOffset":151164,"symbol":"_dispatch_kq_poll","symbolLocation":228,"imageIndex":17},{"imageOffset":148276,"symbol":"_dispatch_event_loop_poke","symbolLocation":336,"imageIndex":17},{"imageOffset":123648,"symbol":"CoreAnalytics::Client::sendXpcMessage_sync(applesauce::xpc::dict, XPCMessagePriority, bool, bool)","symbolLocation":252,"imageIndex":24},{"imageOffset":133212,"symbol":"invocation function for block in std::__1::decay<decltype(fp0())>::type applesauce::dispatch::v1::sync_impl<CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict const&, XPCMessagePriority, bool, bool)::$_0>(dispatch_queue_s*, CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict const&, XPCMessagePriority, bool, bool)::$_0&&, std::__1::integral_constant<bool, false>)","symbolLocation":112,"imageIndex":24},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":72000,"symbol":"_dispatch_lane_barrier_sync_invoke_and_complete","symbolLocation":56,"imageIndex":17},{"imageOffset":119984,"symbol":"CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict const&, XPCMessagePriority, bool, bool)","symbolLocation":152,"imageIndex":24},{"imageOffset":119252,"symbol":"CoreAnalytics::Client::sendEventAlways(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>, std::__1::function<nlohmann::basic_json<std::__1::map, std::__1::vector, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>, bool, long long, unsigned long long, double, std::__1::allocator, nlohmann::adl_serializer, std::__1::vector<unsigned char, std::__1::allocator<unsigned char>>> ()> const&, CoreAnalytics::SendEventOptions)","symbolLocation":768,"imageIndex":24},{"imageOffset":29520,"symbol":"AnalyticsSendEventInternal(NSString const*, NSDictionary<NSString*, NSObject*>* () block_pointer, XPCMessagePriority, EventPayloadFormat, NSDictionary<NSString*, NSObject*>*)","symbolLocation":644,"imageIndex":24},{"imageOffset":19056,"symbol":"__createContextTelemetryDataWithQueueLabelAndCallstack_block_invoke","symbolLocation":848,"imageIndex":19},{"imageOffset":7004,"symbol":"_dispatch_call_block_and_release","symbolLocation":32,"imageIndex":17},{"imageOffset":113348,"symbol":"_dispatch_client_callout","symbolLocation":16,"imageIndex":17},{"imageOffset":231916,"symbol":"<deduplicated_symbol>","symbolLocation":32,"imageIndex":17},{"imageOffset":82236,"symbol":"_dispatch_root_queue_drain","symbolLocation":736,"imageIndex":17},{"imageOffset":83844,"symbol":"_dispatch_worker_thread2","symbolLocation":180,"imageIndex":17},{"imageOffset":11792,"symbol":"_pthread_wqthread","symbolLocation":232,"imageIndex":20},{"imageOffset":7068,"symbol":"start_wqthread","symbolLocation":8,"imageIndex":20}],"threadState":{"x":[{"value":0},{"value":0},{"value":1},{"value":6106818104},{"value":1},{"value":0},{"value":0},{"value":1027},{"value":1024},{"value":8534643136,"symbolLocation":0,"symbol":"_dispatch_mgr_q"},{"value":1024},{"value":273},{"value":37702636984},{"value":65519},{"value":9005137670438912},{"value":9005137670438912},{"value":375},{"value":8557913552},{"value":0},{"value":1027},{"value":0},{"value":0},{"value":1},{"value":6106818104},{"value":1},{"value":6106818104},{"value":37702636928},{"value":132096},{"value":8534643136,"symbolLocation":0,"symbol":"_dispatch_mgr_q"}],"flavor":"ARM_THREAD_STATE64","lr":{"value":6710869628},"cpsr":{"value":4096},"fp":{"value":6106818080},"sp":{"value":6106817984},"esr":{"value":1442840704,"description":"(Syscall)"},"pc":{"value":6712277600},"far":{"value":0}}}],
  "usedImages" : [
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4370366464,
    "size" : 20971520,
    "uuid" : "3d5e365b-b059-3651-b8ab-fc2934fd9275",
    "path" : "*\/mbgl-glfw",
    "name" : "mbgl-glfw"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4433575936,
    "size" : 163840,
    "uuid" : "adf13a1c-2d7d-3796-af80-ee61fcffee92",
    "path" : "\/opt\/homebrew\/*\/libglfw.3.4.dylib",
    "name" : "libglfw.3.4.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4433870848,
    "size" : 147456,
    "uuid" : "1d486868-3c8c-321e-9727-3a5ba5cd220e",
    "path" : "\/opt\/homebrew\/*\/libpng16.16.dylib",
    "name" : "libpng16.16.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4435361792,
    "size" : 786432,
    "uuid" : "3d359291-b602-39fc-8112-8876e0ebaec3",
    "path" : "\/opt\/homebrew\/*\/libharfbuzz.0.dylib",
    "name" : "libharfbuzz.0.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4437295104,
    "size" : 507904,
    "uuid" : "cfd169d0-526f-3a45-b5db-836906fec3d7",
    "path" : "\/opt\/homebrew\/*\/libfreetype.6.dylib",
    "name" : "libfreetype.6.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4439326720,
    "size" : 1032192,
    "uuid" : "6bee9501-347f-38e9-912c-4d8d075e5ed8",
    "path" : "\/opt\/homebrew\/*\/libglib-2.0.0.dylib",
    "name" : "libglib-2.0.0.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4434083840,
    "size" : 81920,
    "uuid" : "1bbb20c8-b0d3-318a-a229-f81c8a8dd304",
    "path" : "\/opt\/homebrew\/*\/libgraphite2.3.2.1.dylib",
    "name" : "libgraphite2.3.2.1.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4434231296,
    "size" : 163840,
    "uuid" : "9414c162-5ccc-3b6d-8a0d-c7c743f7fec1",
    "path" : "\/opt\/homebrew\/*\/libintl.8.dylib",
    "name" : "libintl.8.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64",
    "base" : 4437950464,
    "size" : 507904,
    "uuid" : "a9da24ef-74d4-397b-ab16-1d18e55a653b",
    "path" : "\/opt\/homebrew\/*\/libpcre2-8.0.dylib",
    "name" : "libpcre2-8.0.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 4439244800,
    "size" : 49152,
    "uuid" : "580e4b29-8304-342d-a21c-2a869364dc35",
    "path" : "\/usr\/lib\/libobjc-trampolines.dylib",
    "name" : "libobjc-trampolines.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 4864540672,
    "CFBundleShortVersionString" : "340.26.3",
    "CFBundleIdentifier" : "com.apple.AGXMetalG13X",
    "size" : 7651328,
    "uuid" : "e09c4f6b-8ff6-3edf-9e21-95f237f5a392",
    "path" : "\/System\/Library\/Extensions\/AGXMetalG13X.bundle\/Contents\/MacOS\/AGXMetalG13X",
    "name" : "AGXMetalG13X",
    "CFBundleVersion" : "340.26.3"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6712803328,
    "CFBundleShortVersionString" : "6.9",
    "CFBundleIdentifier" : "com.apple.CoreFoundation",
    "size" : 5558144,
    "uuid" : "edb39786-80b1-3bff-b6c3-e33f2e3ca867",
    "path" : "\/System\/Library\/Frameworks\/CoreFoundation.framework\/Versions\/A\/CoreFoundation",
    "name" : "CoreFoundation",
    "CFBundleVersion" : "4040.1.401"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6924197888,
    "CFBundleShortVersionString" : "2.1.1",
    "CFBundleIdentifier" : "com.apple.HIToolbox",
    "size" : 3155712,
    "uuid" : "f369023f-1a24-3f3f-9056-6f5248071e8c",
    "path" : "\/System\/Library\/Frameworks\/Carbon.framework\/Versions\/A\/Frameworks\/HIToolbox.framework\/Versions\/A\/HIToolbox",
    "name" : "HIToolbox"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6784290816,
    "CFBundleShortVersionString" : "6.9",
    "CFBundleIdentifier" : "com.apple.AppKit",
    "size" : 24100256,
    "uuid" : "6d84c7a3-ee98-3daf-ac66-cd95a902ba84",
    "path" : "\/System\/Library\/Frameworks\/AppKit.framework\/Versions\/C\/AppKit",
    "name" : "AppKit",
    "CFBundleVersion" : "2685.10.108"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6708580352,
    "size" : 651124,
    "uuid" : "abfd3247-50ac-3c8e-b72a-83710166e982",
    "path" : "\/usr\/lib\/dyld",
    "name" : "dyld"
  },
  {
    "size" : 0,
    "source" : "A",
    "base" : 0,
    "uuid" : "00000000-0000-0000-0000-000000000000"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6712266752,
    "size" : 246880,
    "uuid" : "2eb73bf1-8c71-3e1f-a160-6da83dc82606",
    "path" : "\/usr\/lib\/system\/libsystem_kernel.dylib",
    "name" : "libsystem_kernel.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6710718464,
    "size" : 290464,
    "uuid" : "17d849c6-a785-3dbb-bfb5-8321706c4b8d",
    "path" : "\/usr\/lib\/system\/libdispatch.dylib",
    "name" : "libdispatch.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6709248000,
    "size" : 314144,
    "uuid" : "0cec3289-f166-35f6-8cef-de3d17eb0228",
    "path" : "\/usr\/lib\/system\/libxpc.dylib",
    "name" : "libxpc.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6916284416,
    "CFBundleShortVersionString" : "370.63.1",
    "CFBundleIdentifier" : "com.apple.Metal",
    "size" : 3027456,
    "uuid" : "1941ba9c-cd02-3c59-8f38-d54843e73f5a",
    "path" : "\/System\/Library\/Frameworks\/Metal.framework\/Versions\/A\/Metal",
    "name" : "Metal",
    "CFBundleVersion" : "370.63.1"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6712516608,
    "size" : 51900,
    "uuid" : "5d31d65c-2ecf-36da-84f5-ba4caab06adb",
    "path" : "\/usr\/lib\/system\/libsystem_pthread.dylib",
    "name" : "libsystem_pthread.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6782877696,
    "CFBundleShortVersionString" : "2.0.2",
    "CFBundleIdentifier" : "com.apple.framework.IOKit",
    "size" : 959296,
    "uuid" : "41408ce6-4525-3c70-b469-9830fb87e90a",
    "path" : "\/System\/Library\/Frameworks\/IOKit.framework\/Versions\/A\/IOKit",
    "name" : "IOKit"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 7320371200,
    "CFBundleShortVersionString" : "128.3",
    "CFBundleIdentifier" : "com.apple.IOGPU",
    "size" : 330784,
    "uuid" : "daa8e305-606c-3908-a210-3e8781a9a911",
    "path" : "\/System\/Library\/PrivateFrameworks\/IOGPU.framework\/Versions\/A\/IOGPU",
    "name" : "IOGPU",
    "CFBundleVersion" : "128.3"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6711554048,
    "size" : 601684,
    "uuid" : "21dfdb11-8328-31d9-a35d-08c1b52621a6",
    "path" : "\/usr\/lib\/libc++.1.dylib",
    "name" : "libc++.1.dylib"
  },
  {
    "source" : "P",
    "arch" : "arm64e",
    "base" : 6871199744,
    "CFBundleShortVersionString" : "1.0",
    "CFBundleIdentifier" : "com.apple.analyticsd",
    "size" : 195808,
    "uuid" : "7568a9c7-aa6a-3f6b-91c3-02c822bf6425",
    "path" : "\/System\/Library\/PrivateFrameworks\/CoreAnalytics.framework\/Versions\/A\/CoreAnalytics",
    "name" : "CoreAnalytics",
    "CFBundleVersion" : "1"
  }
],
  "sharedCache" : {
  "base" : 6707511296,
  "size" : 5557583872,
  "uuid" : "52e4eec0-379e-31c7-ac62-bfed14d90f52"
},
  "legacyInfo" : {
  "threadTriggered" : {

  }
},
  "logWritingSignature" : "ec4458d9b92490b62840eb4bf3c04b18ac40b9e6",
  "trialInfo" : {
  "rollouts" : [
    {
      "rolloutId" : "67d07cd6a7affa169ae21f45",
      "factorPackIds" : [

      ],
      "deploymentId" : 240000002
    },
    {
      "rolloutId" : "6246d6a916a70b047e454124",
      "factorPackIds" : [

      ],
      "deploymentId" : 240000010
    }
  ],
  "experiments" : [

  ]
}
}

