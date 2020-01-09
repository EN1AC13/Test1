#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <map>

using namespace std;

typedef struct TreeNode
{
    int data;
    struct TreeNode *left;
    struct TreeNode *right;  
}TreeNode;


int deep(TreeNode *root) {
    if(root == nullptr) 
        return 0;
    if(root->left == nullptr) 
        return 1+deep(root->right);
    if(root->right == nullptr)
        return 1+deep(root->left);
    int left_deep = deep(root->left) + 1;
    int right_deep = deep(root->right) +1 ;
    return left_deep < right_deep ? left_deep:right_deep;
}

int evalRPN(vector<string> &token) {
    stack<int> num;
    for(auto tmp : token) {
        if(tmp=="+"||tmp=="-"||tmp=="*"||tmp=="/") {
            if(num.size()<2) return 0;
            int a = num.top();
            num.pop();
            int b = num.top();
            num.pop();
            int sum = 0;
            if(tmp=="+")
                sum = a+b;
            if(tmp=="-")
                sum = b-a;
            if(tmp=="*")
                sum = a*b;
            if(tmp=="/"){
                if(a == 0)
                    return 0;
                sum = b / a;
            }
            num.push(sum);
        }
        else {
            num.push(atoi(tmp.c_str()));
        }
    }
    return num.top();
}


int main()
{
    vector<string> token;
    token.push_back("2");
    token.push_back("1");
    token.push_back("+");
    token.push_back("3");
    token.push_back("*");
    cout<<evalRPN(token)<<endl;
    map<double,int> mp;
}