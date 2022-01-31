#include "GraphMatch.hpp"
#include "Graph.hpp"
#include "BiMap.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using namespace subiso;
using namespace qaoagraph;
using namespace bimap;

// The fixture for testing class Foo.
class SubIsoTest : public ::testing::Test {
   protected:
   // You can remove any or all of the following functions if their bodies would
   // be empty.

   SubIsoTest() {
      // You can do set-up work for each test here.
   }

   ~SubIsoTest() override {
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

TEST_F(SubIsoTest, Figure1Test) {
    Graph queryG;
    string queryGraphPath = "../../test/graph/fig1q.txt";
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryG.load_from_file(queryGraphFile)) {
        cerr << "Cannot load query graph file ../../test/graph/fig1q.txt.\n";
        FAIL();
    }
    Graph dataG;
    string dataGraphPath = "../../test/graph/fig1d.txt";
    ifstream dataGraphFile(dataGraphPath);
    if (!dataGraphFile.is_open() || !dataG.load_from_file(dataGraphFile)) {
        cerr << "Cannot load query graph file ../../test/graph/fig1d.txt.\n";
        FAIL();
    }

    GraphMatch gm(queryG, dataG);
    vector<BiMap> result;
    result = gm.subgraph_isomorphsim();
    cout << "subgraph isomorphsim finds " << result.size() << " results" << endl;
    for (int i = 0; i < result.size(); i++) {
        auto M = result[i];
        // For each edge u u' in the queryG, we can find edge M[u], M[u'] in dataG
        for (auto edge : queryG.get_edges()) {
            int u = edge[0], u_prime = edge[1];
            EXPECT_EQ(dataG.has_edge(M.getValueByKey(u), M.getValueByKey(u_prime)), true);
        }
    }
}


TEST_F(SubIsoTest, TriangleTest) {
    Graph queryG;
    string queryGraphPath = "../../test/graph/triangle.txt";
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryG.load_from_file(queryGraphFile)) {
        cerr << "Cannot load query graph file ../../test/graph/triangle.txt.\n";
        FAIL();
    }
    Graph dataG;
    string dataGraphPath = "../../test/graph/fig1d.txt";
    ifstream dataGraphFile(dataGraphPath);
    if (!dataGraphFile.is_open() || !dataG.load_from_file(dataGraphFile)) {
        cerr << "Cannot load query graph file ../../test/graph/fig1d.txt.\n";
        FAIL();
    }

    GraphMatch gm(queryG, dataG);
    vector<BiMap> result;
    result = gm.subgraph_isomorphsim();
    cout << "subgraph isomorphsim finds " << result.size() << " results" << endl;
    for (int i = 0; i < result.size(); i++) {
        auto M = result[i];
        // For each edge u u' in the queryG, we can find edge M[u], M[u'] in dataG
        for (auto edge : queryG.get_edges()) {
            int u = edge[0], u_prime = edge[1];
            EXPECT_EQ(dataG.has_edge(M.getValueByKey(u), M.getValueByKey(u_prime)), true);
        }
    }
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}