// doctest.h - the lightest feature-rich C++ single-header testing framework for unit tests and TDD
// 
// Copyright (c) 2016-2023 Viktor Kirilov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
// The documentation can be found at the library's page:
// https://github.com/doctest/doctest/blob/master/doc/markdown/readme.md

#ifndef DOCTEST_LIBRARY_INCLUDED
#define DOCTEST_LIBRARY_INCLUDED

// Minimal doctest implementation for ESP32 testing
#define DOCTEST_VERSION_MAJOR 2
#define DOCTEST_VERSION_MINOR 4
#define DOCTEST_VERSION_PATCH 11

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>

namespace doctest {
    namespace detail {
        struct TestCase {
            std::string name;
            std::function<void()> test_func;
            bool passed = true;
            std::string failure_reason;
        };
        
        static std::vector<TestCase>& getTests() {
            static std::vector<TestCase> tests;
            return tests;
        }
        
        static TestCase* current_test = nullptr;
        
        class TestCaseRegistrar {
        public:
            TestCaseRegistrar(const char* name, std::function<void()> func) {
                getTests().push_back({name, func, true, ""});
            }
        };
        
        template<typename T>
        class Expression {
        public:
            Expression(bool result, const char* expr, T val) 
                : m_result(result), m_expr(expr), m_value(val) {}
            
            ~Expression() {
                if (!m_result && current_test) {
                    current_test->passed = false;
                    std::stringstream ss;
                    ss << "CHECK failed: " << m_expr << " (value: " << m_value << ")";
                    current_test->failure_reason = ss.str();
                }
            }
            
        private:
            bool m_result;
            const char* m_expr;
            T m_value;
        };
    }
    
    struct Context {
        static int run() {
            int failures = 0;
            auto& tests = detail::getTests();
            
            std::cout << "[doctest] run with " << tests.size() << " test cases\n";
            
            for (auto& test : tests) {
                detail::current_test = &test;
                std::cout << "TEST CASE: " << test.name << std::endl;
                
                try {
                    test.test_func();
                    if (test.passed) {
                        std::cout << "  ✓ PASSED\n";
                    } else {
                        std::cout << "  ✗ FAILED: " << test.failure_reason << "\n";
                        failures++;
                    }
                } catch (const std::exception& e) {
                    std::cout << "  ✗ FAILED: Exception: " << e.what() << "\n";
                    failures++;
                } catch (...) {
                    std::cout << "  ✗ FAILED: Unknown exception\n";
                    failures++;
                }
            }
            
            std::cout << "\n[doctest] test cases: " << tests.size() 
                     << " | " << (tests.size() - failures) << " passed | " 
                     << failures << " failed\n";
            
            return failures;
        }
    };
}

#define TEST_CASE(name) \
    static void test_##__LINE__(); \
    static doctest::detail::TestCaseRegistrar reg_##__LINE__(name, test_##__LINE__); \
    static void test_##__LINE__()

#define CHECK(expr) \
    do { \
        auto result = static_cast<bool>(expr); \
        doctest::detail::Expression<bool> temp(result, #expr, result); \
    } while(false)

#define CHECK_EQ(lhs, rhs) \
    do { \
        auto lval = (lhs); \
        auto rval = (rhs); \
        auto result = (lval == rval); \
        if (!result) { \
            std::cout << "  CHECK_EQ failed: " << #lhs << " == " << #rhs \
                     << " (" << lval << " == " << rval << ")\n"; \
            if (doctest::detail::current_test) { \
                doctest::detail::current_test->passed = false; \
            } \
        } \
    } while(false)

#define CHECK_NE(lhs, rhs) \
    do { \
        auto lval = (lhs); \
        auto rval = (rhs); \
        auto result = (lval != rval); \
        if (!result) { \
            std::cout << "  CHECK_NE failed: " << #lhs << " != " << #rhs \
                     << " (" << lval << " != " << rval << ")\n"; \
            if (doctest::detail::current_test) { \
                doctest::detail::current_test->passed = false; \
            } \
        } \
    } while(false)

#define CHECK_LT(lhs, rhs) \
    do { \
        auto lval = (lhs); \
        auto rval = (rhs); \
        auto result = (lval < rval); \
        if (!result) { \
            std::cout << "  CHECK_LT failed: " << #lhs << " < " << #rhs \
                     << " (" << lval << " < " << rval << ")\n"; \
            if (doctest::detail::current_test) { \
                doctest::detail::current_test->passed = false; \
            } \
        } \
    } while(false)

#define CHECK_GT(lhs, rhs) \
    do { \
        auto lval = (lhs); \
        auto rval = (rhs); \
        auto result = (lval > rval); \
        if (!result) { \
            std::cout << "  CHECK_GT failed: " << #lhs << " > " << #rhs \
                     << " (" << lval << " > " << rval << ")\n"; \
            if (doctest::detail::current_test) { \
                doctest::detail::current_test->passed = false; \
            } \
        } \
    } while(false)

#define DOCTEST_IMPLEMENT_FIXTURE_DERIVED(base, derived, name) \
    namespace { \
        struct derived : public base { \
            void f(); \
        }; \
        static void test_##derived() { \
            derived v; \
            v.f(); \
        } \
        static doctest::detail::TestCaseRegistrar reg_##derived(name, test_##derived); \
    } \
    inline void derived::f()

#endif // DOCTEST_LIBRARY_INCLUDED