#include "GraphMatch.hpp"
#include "Graph.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using namespace subiso;

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

    Graph generate_fig5(bool directed = false) {
        Graph g(9, directed);
        g.add_edge(0, 1);
        g.add_edge(0, 2);
        g.add_edge(0, 3);
        g.add_edge(1, 4);
        g.add_edge(1, 5);
        g.add_edge(2, 6);
        g.add_edge(3, 7);
        g.add_edge(5, 6);
        g.add_edge(6, 8);

        return g;
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
    queryGraphFile.close();
    string dataGraphPath = "../../test/graph/fig1d.txt";
    ifstream dataGraphFile(dataGraphPath);
    if (!dataGraphFile.is_open() || !dataG.load_from_file(dataGraphFile)) {
        cerr << "Cannot load data graph file ../../test/graph/fig1d.txt.\n";
        FAIL();
    }
    dataGraphFile.close();
    GraphMatch gm(queryG, dataG);
    Graph initCS(true);
    vector<unordered_map<int, int>> uv2id(queryG.num_nodes());;
    unordered_map<int, pair<int, int>> id2uv;
    gm.build_init_CS(initCS, uv2id, id2uv);

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

TEST_F(CSTest, RefineCSTest) {
    Graph queryDAG;
    string queryGraphPath = "../../test/graph/fig1q.txt";
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryDAG.load_from_file(queryGraphFile, true)) {
        cerr << "Cannot load query graph file ../../test/graph/fig1q.txt.\n";
        FAIL();
    }
    queryGraphFile.close();
    
    GraphMatch gm;
    Graph initCS(true);
    vector<unordered_map<int, int>> uv2id(queryDAG.num_nodes());
    unordered_map<int, pair<int, int>> id2uv;
    // Build initCS
    initCS = Graph(11, true);
    set<vector<int>> edges = {
        {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {2, 8}, {2, 9}, 
        {3, 10}, {3, 5}, {4, 10}, {4, 5}, {8, 11}, {8, 6}, {8, 7},
        {5, 10}, {6, 10}, {7, 10}, {9, 11}
    };
    for (auto edge : edges) {
        initCS.add_edge(edge[0] - 1, edge[1] - 1);
    }
    vector<vector<int>> u2v = {
        {1, 2}, {3, 4, 8}, {5, 6, 7, 9}, {10, 11}
    };
    for (int u = 0; u < u2v.size(); u++) {
        for (int v : u2v[u]) {
            // In this example id = v
            uv2id[u][v - 1] = v - 1;
            id2uv[v - 1] = make_pair(u, v - 1);
        }
    }
    EXPECT_EQ(initCS.num_nodes(), 11);
    EXPECT_EQ(initCS.num_edges(), 18);
    bool changed = false;
    edges = set<vector<int>>();
    for (auto edge : initCS.get_edges()) {
        edges.insert(edge);
    }

    changed = gm.refine_CS_wrapper(initCS, uv2id, id2uv, queryDAG, 1);
    EXPECT_EQ(changed, true);
    EXPECT_EQ(initCS.num_edges(), 15);
    edges.erase({1, 8});
    edges.erase({7, 10});
    edges.erase({8, 10});
    EXPECT_THAT(initCS.get_edges(), testing::UnorderedElementsAreArray(edges));

    changed = gm.refine_CS_wrapper(initCS, uv2id, id2uv, queryDAG, 0);
    EXPECT_EQ(changed, true);
    EXPECT_EQ(initCS.num_edges(), 12);
    edges.erase({1, 7});
    edges.erase({7, 5});
    edges.erase({7, 6});
    EXPECT_THAT(initCS.get_edges(), testing::UnorderedElementsAreArray(edges));

    changed = gm.refine_CS_wrapper(initCS, uv2id, id2uv, queryDAG, 1);
    EXPECT_EQ(changed, true);
    EXPECT_EQ(initCS.num_edges(), 8);
    edges.erase({0, 5});
    edges.erase({0, 6});
    edges.erase({5, 9});
    edges.erase({6, 9});
    EXPECT_THAT(initCS.get_edges(), testing::UnorderedElementsAreArray(edges));

    changed = gm.refine_CS_wrapper(initCS, uv2id, id2uv, queryDAG, 0);
    EXPECT_EQ(changed, false);
}


TEST_F(CSTest, BuildWeightArrayTest) {
    Graph queryDAG = generate_fig5(true);
    Graph initCS(true);
    vector<unordered_map<int, int>> uv2id(queryDAG.num_nodes());;
    unordered_map<int, pair<int, int>> id2uv;
    // Build CS in Figure 5b
    Graph CS(23, true);
    // Add edges
    vector<vector<int>> edges = {
        {1, 2, 3, 4, 5, 6}, {7, 8, 9}, {7, 8, 10}, {15}, {16}, {17},
        {11, 12, 13, 14}, {}, {}, {15, 16}, {17}, {}, {}, {}, {},
        {18, 19}, {18, 19, 20, 21, 22}, {20}, {}, {}, {}, {}, {}
    };
    for (int u = 0; u < 23; u++) {
        for (auto v : edges[u]) {
            CS.add_edge(u, v);
        }
    }
    EXPECT_EQ(CS.num_edges(), 30);
    // Build uv2id
    vector<vector<int>> u2id = {
        {0}, {1, 2}, {3, 4, 5}, {6},
        {7, 8}, {9, 10}, {15, 16, 17}, 
        {11, 12, 13, 14}, {18, 19, 20, 21, 22}
    };
    vector<vector<int>> u2v = {
        {0}, {1, 2}, {3, 4, 5}, {1},
        {0, 6}, {7, 8}, {9, 10, 11}, 
        {11, 12, 13, 14}, {15, 16, 17, 18, 19}
    };
    int id = 0;
    for (int u = 0; u < u2v.size(); u++) {
        for (int j = 0; j < u2v[u].size(); j++) {
            int v = u2v[u][j], id = u2id[u][j];
            uv2id[u][v] = id;
            id2uv[id] = make_pair(u, v);
            id++;
        }
    }
    
    GraphMatch gm;
    vector<int> weightArray(23, 0);
    gm.build_weight_array(weightArray, queryDAG, CS, uv2id, id2uv);
    vector<int> correctWeightArray(23, 0); // Figure 5c
    for (int u = 0; u < 23; u++) correctWeightArray[u] = 1;
    correctWeightArray[0] = 2;
    correctWeightArray[6] = 4;
    correctWeightArray[15] = 2;
    correctWeightArray[16] = 5;

    for (int u = 0; u < 23; u++) {
        EXPECT_EQ(weightArray[u], correctWeightArray[u]);
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}