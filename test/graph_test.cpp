#include "../src/Graph.hpp"

#include "gtest/gtest.h"


// The fixture for testing class Foo.
class GraphTest : public ::testing::Test {
   protected:
   // You can remove any or all of the following functions if their bodies would
   // be empty.

   GraphTest() {
      // You can do set-up work for each test here.
   }

   ~GraphTest() override {
      // You can do clean-up work that doesn't throw exceptions here.
   }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

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

// Tests that the Foo::Bar() method does Abc.
TEST_F(GraphTest, LoadFromFileSmall) {
   vector<string> test_graphs = {"line2", "line3", "triangle", "square", "clique4"};
   vector<int> test_nodes = {2, 3, 3, 4, 4};
   vector<int> test_edges = {1, 2, 3, 4, 6};

   for (int i = 0; i < test_graphs.size(); i++) {
      Graph g;
      string test_graph_path = "../test/graph/" + test_graphs[i] + ".txt";
      ifstream graph_file(test_graph_path);
      if (!graph_file.is_open() || !g.load_from_file(graph_file)) {
         cerr << "graph file " << test_graph_path << " does not exist.\n";
         FAIL();
      } else {

         EXPECT_EQ(num_vertices(g.graph_), test_nodes[i]);
         EXPECT_EQ(num_edges(g.graph_), test_edges[i]);

         graph_file.close();
      }

   }

   
}

// // Tests that Foo does Xyz.
// TEST_F(GraphTest, DoesXyz) {
//   // Exercises the Xyz feature of Foo.
// }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}