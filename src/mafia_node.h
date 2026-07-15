#ifndef MAFIA_NODE_H
#define MAFIA_NODE_H

#include <string>

using namespace std;

struct MafiaNode {
    int id;
    string name;
    string last_name;
    char gender; 
    int age;
    int id_boss;
    
    bool is_dead;
    bool in_jail;
    bool was_boss;
    bool is_boss;

    MafiaNode* left;
    MafiaNode* right;
};

MafiaNode* createMafiaNode(int id, string name, string last_name, char gender, int age, int id_boss, bool is_dead, bool in_jail, bool was_boss, bool is_boss) {
    MafiaNode* newNode = new MafiaNode();
    
    newNode->id = id;
    newNode->name = name;
    newNode->last_name = last_name;
    newNode->gender = gender;
    newNode->age = age;
    newNode->id_boss = id_boss;
    newNode->is_dead = is_dead;
    newNode->in_jail = in_jail;
    newNode->was_boss = was_boss;
    newNode->is_boss = is_boss;
    
    newNode->left = nullptr;
    newNode->right = nullptr;
    
    return newNode;
}

#endif