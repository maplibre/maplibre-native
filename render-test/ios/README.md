# iOS RenderTestRunner App

The test instantiates ObjC object of class `IosTestRunner` from the app, the class will instantiates a C++ object from the linked library `mbgl-render-test` and calls running render test function on it. It subsequently deletes the C++ object pointer. In the end, the test will check the existence of test reports, which are html files that attach to xctest result.
