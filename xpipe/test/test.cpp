#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestProgressListener.h>
#include <string>
#include <iostream>

int main()
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestResult controller;
    CppUnit::TextTestProgressListener progressListener;
    CppUnit::TestResultCollector result;
    controller.addListener(&result);
    controller.addListener(&progressListener);
    CppUnit::TextOutputter consoleOutputter(&result, std::cout);

    CppUnit::TestFactoryRegistry &registry =
        CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    runner.run(controller);

    consoleOutputter.write();

    return !result.wasSuccessful();
}
