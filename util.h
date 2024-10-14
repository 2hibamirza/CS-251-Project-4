/*
 * File: util.h
 * Name: Hiba Mirza
 * NetID: hmirz4
 * Course: CS 251 Data Structures (21814) 2024 Summer
 * Program Overview: Implements Huffman encoding and decoding for file compression and decompression
 */

#include <iostream>
#include <fstream>
#include <map>
#include <queue>          // std::priority_queue
#include <vector>         // std::vector
#include <functional>     // std::greater
#include <string>
#include "bitstream.h"
#include "hashmap.h"
#pragma once

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

//
// *This method frees the memory allocated for the Huffman tree.
// @param node: a pointer to the root of the Huffman tree
//
void freeTree(HuffmanNode* node) {
    if (node == nullptr) {
        return;
    }

    freeTree(node->zero);
    freeTree(node->one);
    delete node;
}

//
// *This function builds the frequency map.  
//  If isFile is true, read from the file with name filename.  
//  If isFile is false, read directly from string filename.
// @param filename: the name of the file or string to read from
// @param isFile: a boolean indicating if the input is a file or a string
// @param map: the hashmap to store the frequency of each character
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
    char c;

    if (isFile) {
        ifstream is(filename);

        while (is.get(c)) {
            int charCode = c;
            if (map.containsKey(charCode)) {
                map.put(charCode, map.get(charCode) + 1);
            } else {
                map.put(charCode, 1);
            }
        }

        is.close();
    } else {
        for (char c : filename) {
            int charCode = c;
            if (map.containsKey(charCode)) {
                map.put(charCode, map.get(charCode) + 1);
            } else {
                map.put(charCode, 1);
            }
        }
    }
    map.put(PSEUDO_EOF, 1);
}

struct compareNodes {
    bool operator()(HuffmanNode* lhs, HuffmanNode* rhs) {
        return lhs->count > rhs->count;
    }
};

//
// *This function builds an encoding tree from the frequency map.
// @param map: the hashmap containing the frequency of each character
// @return: a pointer to the root of the Huffman encoding tree
//
HuffmanNode* buildEncodingTree(hashmap &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compareNodes> pq;

    for(int key : map.keys()) {
        HuffmanNode* node = new HuffmanNode{key, map.get(key), nullptr, nullptr};
        pq.push(node);
    }

    while(pq.size() > 1){
        HuffmanNode* left = pq.top();
        pq.pop();
        HuffmanNode* right = pq.top();
        pq.pop();
        HuffmanNode* com = new HuffmanNode{NOT_A_CHAR, left->count + right->count, left, right};
        pq.push(com);
    }

    return pq.top();
}

//
// *this helper function builds the encoding map from an encoding tree
// @param node: a pointer to the current node in the Huffman tree
// @param encodingMap: a reference to the map that will store character to bit string mappings
// @param path: the current path of bits (0s and 1s) to reach the node
//
void buildEncodingMapHelper(HuffmanNode* node, map<int,string>& encodingMap, string path) {
    if (node == nullptr) {
        return;
    }

    if(node->zero == nullptr && node->one == nullptr) {
        encodingMap[node->character] = path;
    } else {
        buildEncodingMapHelper(node->zero, encodingMap, path + "0");
        buildEncodingMapHelper(node->one, encodingMap, path + "1");
    }
}

//
// *This function builds the encoding map from an encoding tree.
// @param tree: a pointer to the root of the Huffman encoding tree
// @return: a map that stores character to bit string mappings
//
map<int,string> buildEncodingMap(HuffmanNode* tree) {
    map<int,string> encodingMap;

    if (tree != nullptr) {
        buildEncodingMapHelper(tree, encodingMap, "");
    }

    return encodingMap; 
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
// @param input: a reference to the input file stream
// @param encodingMap: a reference to the map that stores character to bit string mappings
// @param output: a reference to the output bit stream
// @param size: a reference to an integer that will store the number of bits written
// @param makeFile: a boolean indicating if the encoded string should be written to a file
// @return: a string representation of the encoded file
//
string encode(ifstream& input, map<int,string> &encodingMap,
              ofbitstream& output, int &size, bool makeFile) {
    char c;
    string encodedString = "";

    while(input.get(c)) {
        encodedString += encodingMap.at(c);
    }

    encodedString += encodingMap.at(PSEUDO_EOF);

    if (makeFile) {
        for (char bit : encodedString) {
            output.writeBit(bit - '0');
        }
    }

    size = encodedString.length();

    return encodedString;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
// @param input: a reference to the input bit stream
// @param encodingTree: a pointer to the root of the Huffman encoding tree
// @param output: a reference to the output file stream
// @return: a string representation of the decoded file
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    string result = "";
    HuffmanNode* currNode = encodingTree;

    while (true) {
        int bit = input.readBit();
        if (bit == 0) {
            currNode = currNode->zero;
        } else {
            currNode = currNode->one;
        }

        if (currNode->zero == nullptr && currNode->one == nullptr) {
            if (currNode->character == PSEUDO_EOF) {
                break;
            }
            output.put(currNode->character);
            result += (currNode->character);

            currNode = encodingTree;
        }
    }
    return result; 
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
// @param filename: the name of the file to be compressed
// @return: a string version of the bit pattern
//
string compress(string filename) {
    hashmap frequencyMap;
    buildFrequencyMap(filename, true, frequencyMap);
    HuffmanNode* encodingTree = buildEncodingTree(frequencyMap);
    map<int, string> encodingMap = buildEncodingMap(encodingTree);
    ofbitstream output(filename + ".huf");
    ifstream input(filename);
    output << frequencyMap;
    int size = 0;
    string encodedString = encode(input, encodingMap, output, size, true);
    output.close();
    freeTree(encodingTree);
    return encodedString;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function does.
// @param filename: the name of the compressed file to be decompressed
// @return: a string version of the uncompressed file
//
string decompress(string filename) {
    size_t hufPos = filename.find(".huf");
    string baseFilename = filename.substr(0, hufPos);
    size_t extPos = baseFilename.find_last_of('.');
    string fileExtension = baseFilename.substr(extPos);
    string baseName = baseFilename.substr(0, extPos);
    ifbitstream inputFile(filename);
    ofstream outputFile(baseName + "_unc" + fileExtension);
    hashmap frequencyMap;
    inputFile >> frequencyMap;
    HuffmanNode* huffmanTree = buildEncodingTree(frequencyMap);
    string decodedContent = decode(inputFile, huffmanTree, outputFile);
    outputFile.close();
    freeTree(huffmanTree);
    return decodedContent;
}