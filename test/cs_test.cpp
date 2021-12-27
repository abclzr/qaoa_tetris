#include "../src/GraphMatch.hpp"
#include "../src/Graph.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;

// The fixture for testing class Foo.
class CSTest : public ::testing::Test {
   protected:
   // You can remove any or all of the following functions if their bodies would
   // be empty.

   CSTest() {
      // You can do set-up work for each test here.
   }

   ~CSTest() override {
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

TEST_F(CSTest, InitCSTest) {
    Graph queryG, dataG;
    string queryGraphPath = "../../test/graph/fig1q.txt";
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryG.load_from_file(queryGraphFile)) {
        cerr << "Cannot load query graph file ../../test/graph/fig1q.txt.\n";
        FAIL();
    }
    string dataGraphPath = "../../test/graph/fig1d.txt";
    ifstream dataGraphFile(dataGraphPath);
    if (!dataGraphFile.is_open() || !dataG.load_from_file(dataGraphFile)) {
        cerr << "Cannot load data graph file ../../test/graph/fig1d.txt.\n";
        FAIL();
    }
    GraphMatch gm(queryG, dataG);
    Graph initCS(true);
    unordered_map<int, unordered_map<int, int>> uv2id;
    unordered_map<int, pair<int, int>> id2uv;
    gm.test_init_CS(initCS, uv2id, id2uv);

    auto edges = initCS.get_edges();
    // For each edge in the init cs, we check if edge(u, u') and edge(v, v')
    // exists and v is in u's candidate set as defined in the paper.
    for (auto edge : edges) {
        int id1 = edge[0], id2 = edge[1];
        int u = id2uv[id1].first, v = id2uv[id1].second;
        int u_prime = id2uv[id2].first, v_prime = id2uv[id2].second;
        // cout << u << " " << u_prime << " " << v << " " << v_prime;
        ASSERT_EQ(gm.get_query_dag().has_edge(u, u_prime), true);
        ASSERT_EQ(gm.get_data_G().has_edge(v, v_prime), true);
        auto u_candidate_set = gm.get_query_G().get_candidate_set(u, gm.get_data_G());
        ASSERT_TRUE(u_candidate_set.find(v) != u_candidate_set.end());
        auto u_prime_candidate_set = gm.get_query_G().get_candidate_set(u_prime, gm.get_data_G());
        ASSERT_TRUE(u_prime_candidate_set.find(v_prime) != u_prime_candidate_set.end());
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}