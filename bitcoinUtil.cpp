/* Michael Yitayew (myitayew@princeton.edu)
 * COS 424 Assignment 3
 * Bitcoin transaction prediction
 */

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <stack>
#include <algorithm>

using namespace std;

class edge {
public:
  int sender;
  int recv;
  int transaction;
  edge(int s = 0, int r = 0, int t = 0) {
    sender = s;
    recv = r;
    transaction = t;
  }
}; 

class test_instance {
public:
  int sender;
  int recv;
  int transacted;
  test_instance(int s = 0, int r = 0, int t = 0) {
    sender = s;
    recv = r;
    transacted = t;
  }
};

const int num_nodes     = 444075;
const int num_edges     = 3348026;
const int TRAINING_SIZE = 3348026;
const int TEST_SIZE     = 10000;

bool isReachable(int s, int r, unordered_map<int,vector<int> > &edge_set) {
  queue<int> q;
  q.push(s);
  unordered_map<int,int> visited;
  visited[s] = 1;
  while (!q.empty()) {
    int cur = q.front(); q.pop();
    vector<int> t = edge_set[cur];
    for (int i = 0; i < t.size(); i++) {
      int c = t[i];
      if (visited.find(c) == visited.end()) { q.push(c); visited[c] = 1; }
      if (c == r) return true;
    }
  }
  return false;
}

bool withinK(int s, int r, int k, unordered_map<int,vector<int> > &edge_set) {
  queue<int> q; int depth = 0;
  q.push(s);
  q.push(-1); // depth marker
  unordered_map<int, int> visited;
  visited[s] = 1;
  while (!q.empty() && depth <= k) {
    int cur = q.front(); q.pop();
    if (cur == r) return true;
    else if (cur == -1) { q.push(-1); depth++; }
    vector<int> t = edge_set[cur];
    for (int i = 0; i < t.size(); i++) {
      int c = t[i];
      if (visited.find(c) == visited.end()) { q.push(c); visited[c] = 1; }
    }
  }
  return false;
}

bool commonNeighbors(int s, int r, int n, unordered_map<int,vector<int> > &edge_set) {
  unordered_map<int, int> s_neighbors;
  vector<int> t1 = edge_set[s];
  vector<int> t2 = edge_set[r];

  for (int i = 0; i < t1.size(); i++)
    s_neighbors[t1[i]] = 1;

  int common = 0;
  for (int i = 0; i < t2.size(); i++)
    if (s_neighbors.find(t2[i]) != s_neighbors.end()) { common++; if (common >= n) return true; }

  return false;
}

bool jaccard(int s, int r, double threshold, unordered_map<int,vector<int> > &edge_set) {
  unordered_map<int, int> s_neighbors;
  vector<int> t1 = edge_set[s];
  vector<int> t2 = edge_set[r];

  for (int i = 0; i < t1.size(); i++)
    s_neighbors[t1[i]] = 1;

  int intersection_count = 0;
  int union_count = s_neighbors.size();
  for (int i = 0; i < t2.size(); i++) {
    if (s_neighbors.find(t2[i]) != s_neighbors.end()) intersection_count++;
    else union_count++;
  }

  double jaccardCoef = intersection_count; jaccardCoef /= union_count;
  return jaccardCoef >= threshold;
}


bool simpleDrugNetwork(int s, int r,unordered_map<int,int> &in, unordered_map<int,int> &out) {

  int din_s = in[s]; int dout_s = out[s];
  int din_r = in[r]; int dout_r  = out[r];
  double s_ratio = din_s; s_ratio /= dout_s;
  double r_ratio = din_r; r_ratio /= dout_r;

  if (s_ratio < 0.5 && abs(r_ratio-1) <= 0.5)
    return true;
  else if (abs(s_ratio-1) <= 0.5 && r_ratio > 1.5)
    return true;
  else if (abs(s_ratio-1) <= 0.5 && abs(r_ratio-1) <= 0.5)
    return true;
  else return false;

}

bool drugNetwork2(int s, int r, unordered_map<int,int> &in, unordered_map<int,int> &out, int n, unordered_map<int,vector<int> > &edge_set) {
  if (simpleDrugNetwork(s,r,in,out) || commonNeighbors(s,r,n, edge_set))
    return true;
  return false;
}


int main() {

  /* unordered_map<int, int> mp_send;
   * unordered_map<int, int> mp_recv;
   * int unique_sender_count = 0;
   * int unique_receiver_count = 0; */

  edge* edge_list = new edge[TRAINING_SIZE];
  test_instance* test_list = new test_instance[TEST_SIZE];  
  unordered_map<int,vector<int> > edge_set;
  unordered_map<int, int> indegree;
  unordered_map<int, int> outdegree;
 
  unordered_map<int, int> involume;
  unordered_map<int, int> outvolume;


  for (int i = 0; i < TRAINING_SIZE; i++) {
    int s, r, t;
    cin >> s >> r >> t;
    vector<int> temp;
    if (edge_set.find(s) == edge_set.end()) { temp.push_back(r); edge_set[s] = temp; }
    else edge_set[s].push_back(r);
    temp.clear();
    if (edge_set.find(r) == edge_set.end()) { temp.push_back(s); edge_set[r] = temp; }
    else edge_set[r].push_back(s);
 
    if (outdegree.find(s) == outdegree.end()) { outdegree[s] = 1; outvolume[s] = t; }
    else { outdegree[s]++; outvolume[s] += t; }

    if (indegree.find(r) == indegree.end()) { indegree[r] = 1; involume[r] = t; }
    else { indegree[r]++; involume[r] += t; }

    // if (mp_send.find(s) == mp_send.end()) { mp_send[s] = 1; unique_sender_count++; }
    // if (mp_recv.find(r) == mp_recv.end()) { mp_recv[r] = 1; unique_receiver_count++; }

    edge e(s, r, t);
    edge_list[i] = e;
  }

  //cout<<"unique_senders: "<<unique_sender_count<<endl;
  //cout<<"unique_receivers: "<<unique_receiver_count<<endl;

  for (int i = 0; i < TEST_SIZE; i++) {
    int s, r, t;
    cin >> s >> r >> t;
    test_instance ti(s,r,t);
    test_list[i] = ti;
  }

  // check if graph is connected (in the undirected-sense) or find the largest connected component
  /*  int count = 0;
  unordered_map<int,int> visited;
  unordered_map<int,int> CC;
  int cur = 1;
  queue<int> q; q.push(cur);
  visited[cur] = 1; count++;
  while (!q.empty()) {
    cur = q.front(); q.pop();
    CC[cur] = 1;
    vector<int> t = edge_set[cur];
    for (int i = 0; i < t.size(); i++) {
      int c = t[i];
      if (visited.find(c) == visited.end()) {
	count++;
	visited[c] = 1;
	q.push(c);
      }
    }
  }
  */
  //  cout<<"Largest connected component size: "<<count<<endl;
 
  /* Simplest prediction scheme: given pair (s,r) and query "did they have a transaction"?
   * answer true if they are in the same connected component in the current graph, false otherwise */
  /*  int correct_count1 = 0;
  for (int i = 0; i < TEST_SIZE; i++) {
    int prediction = 0;
    int s = test_list[i].sender; int r = test_list[i].recv; int label = test_list[i].transacted;

    if (CC.find(s) != CC.end() && CC.find(r) != CC.end()) prediction = 1;
    else if (CC.find(s) != CC.end() && CC.find(r) == CC.end()) prediction = 0;
    else if (CC.find(s) == CC.end() && CC.find(r) != CC.end()) prediction = 0;
    else prediction = isReachable(s,r,edge_set);

    if (prediction == label) correct_count1++;
  }
  double accuracy1 = correct_count1; accuracy1 /= TEST_SIZE;
  cout<<"Accuracy: "<<accuracy1<<endl; */

  /* k-hops prediction: given pair (s,r) and query "did they have a transaction"?
   * answer true if there is distance <= k between them in the current graph, false otherwise */
  /*   int correct_count2 = 0; int K = 2;
   for (int i = 0; i < TEST_SIZE; i++) {
    int prediction = 0;
    int s = test_list[i].sender; int r = test_list[i].recv; int label = test_list[i].transacted;
    if (withinK(s,r,K, edge_set)) prediction = 1;
    cout<<s<<" "<<r<<" "<<prediction<<endl; 
    if (prediction == label) correct_count2++; 
    }*/
  
   //double accuracy2 = correct_count2; accuracy2 /= 100;
   //cout<<"Accuracy: "<<accuracy2<<endl;
 
  /* Common Neighbors: given pair (s,r) and query "did they have a transaction"? 
   * answer true if the intersection of their neighbor sets has size >= n in the current graph, false otherwise */
  /*  int correct_count3 = 0; int n = 2;
  for (int i = 0; i < TEST_SIZE; i++) {                                                         
    int prediction = 0;
    int s = test_list[i].sender; int r = test_list[i].recv; int label = test_list[i].transacted;
    if (commonNeighbors(s,r,n, edge_set)) prediction = 1;
    if (prediction == label) correct_count3++; 
    cout<<s<<" "<<r<<" "<<prediction<<endl;
    } */ 
  // double accuracy3 = correct_count3; accuracy3 /= TEST_SIZE;
  //cout<<"Accuracy: "<<accuracy3<<endl;


  /* Jaccard similarity: given pair (s,r) and query "did they have a transaction"?  
   * answer true if their jaccard coefficient >= threshold, false otherwise */
  /*  int correct_count4 = 0; double threshold = 0.00001;  // sparsity*alpha
  for (int i = 0; i < TEST_SIZE; i++) {
    int prediction = 0;
    int s = test_list[i].sender; int r = test_list[i].recv; int label = test_list[i].transacted;
    if (jaccard(s,r,threshold, edge_set)) prediction = 1;
    if (prediction == label) correct_count4++;  
    cout<<s<<" "<<r<<" "<<prediction<<endl;
  }
  */
  //  double accuracy4 = correct_count4; accuracy4 /= TEST_SIZE;
  // cout<<"Accuracy: "<<accuracy4<<endl;
  
  /* Simple Drug network: given pair (s,r) and query "did they have a transaction"? 
   * answer true if s and r are transacting elements in our simple drug network model */
  
  int correct_count5 = 0;
  for (int i = 0; i < TEST_SIZE; i++) {
    int prediction = 0;
    int s = test_list[i].sender; int r = test_list[i].recv; int label = test_list[i].transacted;
    if (simpleDrugNetwork(s,r,involume,outvolume)) prediction = 1;
    if (prediction == label) correct_count5++; 
    //cout<<s<<" "<<r<<" "<<prediction<<endl;
  }
  
  double accuracy5 = correct_count5; accuracy5 /= TEST_SIZE;
  cout<<"Accuracy: "<<accuracy5<<endl;
 

  /* Drug network + common neighbors: 
   *   */
  /*int correct_count6 = 0;
  for (int i = 0; i < TEST_SIZE; i++) {
    int prediction = 0;
    int s = test_list[i].sender; int r = test_list[i].recv; int label = test_list[i].transacted;
    if (drugNetwork2(s,r,involume,outvolume,3,edge_set)) prediction = 1;
    if (prediction == label) correct_count6++; 
    cout<<s<<" "<<r<<" "<<prediction<<endl;
    } 
  */
  //double accuracy6 = correct_count6; accuracy6 /= 1000;
  //cout<<"Accuracy: "<<accuracy6<<endl;

  return 0;
}

