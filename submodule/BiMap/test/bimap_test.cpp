#include "../BiMap.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif



using ::testing::_;
using namespace bimap;

// The fixture for testing class Foo.
class BiMapTest : public ::testing::Test {
   protected:
   // You can remove any or all of the following functions if their bodies would
   // be empty.
   string resource_dir = TEST_RESOURCE_DIR;

   BiMapTest() {
      // You can do set-up work for each test here.
   }

   ~BiMapTest() override {
      // You can do clean-up work that doesn't throw exceptions here.
   }

   void SetUp() override {
      // Code here will be called immediately after the constructor (right
      // before each test).
   }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
  }
      // Class members declared here can be used by all tests in the test suite
      // for Foo.
   };

TEST_F(BiMapTest, BasicTest) {
    BiMap mapping(10);
    for (int i = 0; i < 10; ++i) {
        mapping.update(i, i + 1);
    }
    EXPECT_EQ(mapping.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(mapping.hasKey(i), true);
        EXPECT_EQ(mapping.hasValue(i + 1), true);
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(mapping.getValueByKey(i), i + 1);
        EXPECT_EQ(mapping.getKeyByValue(i + 1), i);
    }
    for (int i = 0; i < 5; ++i) {
        mapping.eraseByKey(i);
        EXPECT_EQ(mapping.hasKey(i), false);
        EXPECT_EQ(mapping.hasValue(i + 1), false);
    }
    EXPECT_EQ(mapping.size(), 5);
    for (int i = 5; i < 10; ++i) {
        mapping.eraseByValue(i + 1);
        EXPECT_EQ(mapping.hasKey(i), false);
        EXPECT_EQ(mapping.hasValue(i + 1), false);
    }
    EXPECT_EQ(mapping.size(), 0);
}

TEST_F(BiMapTest, SwapTest) {
    BiMap mapping(10);
    for (int i = 0; i < 10; ++i) {
        mapping.update(i, i + 1);
    }
    for (int i = 0; i < 5; ++i) {
        mapping.swapValueByKey(i, i + 5);
    }
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(mapping.getValueByKey(i), i + 5 + 1);
        EXPECT_EQ(mapping.getKeyByValue(i + 5 + 1), i);
    }
    for (int i = 5; i < 10; ++i) {
        EXPECT_EQ(mapping.getValueByKey(i), i - 5 + 1);
        EXPECT_EQ(mapping.getKeyByValue(i - 5 + 1), i);
    }
}

TEST_F(BiMapTest, LoadFromFileTest) {
    
    BiMap mapping5;
    string mapping5Path = resource_dir + "5.txt";
    ifstream mapping5File(mapping5Path);
    if (!mapping5File.is_open() || !mapping5.load_from_file(mapping5File)) {
        cerr << "Cannot load file test/testcases/5.txt.\n";
        FAIL();
    }
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(mapping5.hasKey(i), true);
        EXPECT_EQ(mapping5.hasValue(i), true);
        EXPECT_EQ(mapping5.getValueByKey(i), i);
    }

    BiMap mapping8;
    string mapping8Path = resource_dir + "8.txt";
    ifstream mapping8File(mapping8Path);
    if (!mapping8File.is_open() || !mapping8.load_from_file(mapping8File)) {
        cerr << "Cannot load file test/testcases/8.txt.\n";
        FAIL();
    }
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(mapping8.hasKey(i), true);
        EXPECT_EQ(mapping8.hasValue(i), true);
        EXPECT_EQ(mapping8.getValueByKey(i), 8 - i - 1);
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}