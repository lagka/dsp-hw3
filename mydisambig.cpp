#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<map>
#include<vector>
#include<string>
#include<fstream>
#include<cstring>

using namespace std;


#include "File.h"
#include "Ngram.h"
#include "Prob.h"
#include "Vocab.h"

Vocab voc;
int main(int argc, char *argv[]){


    int ngram_order = 2;

    Ngram lm(voc, ngram_order);
    File lmFile(argv[3], "r");
    lm.read(lmFile);
    lmFile.close();

    map<string,vector<string>> vocabmap;
    File mapFile(argv[2], "r");
    ofstream outputFile;
    outputFile.open(argv[4]); 
    char *buf, *map_buf;
    while(map_buf = mapFile.getline()){
        VocabString sent1[maxWordsPerLine];
        unsigned int count = Vocab::parseWords(map_buf, &(sent1[0]), maxWordsPerLine);
        vector<string> sent_string;
        for(int i=1;i<count;i++){
            sent_string.push_back(sent1[i]);
        }
        vocabmap[sent1[0]] = sent_string;
    }

    vocabmap["<s>"].push_back("<s>");
    vocabmap["</s>"].push_back("</s>");
    mapFile.close();

    File testFile(argv[1], "r");
    vector<vector<string>> AnsPaths;
    int check = 0;
    while(buf = testFile.getline()){

        VocabString sent2[maxWordsPerLine];
        unsigned int count = Vocab::parseWords(buf, &(sent2[1]), maxWordsPerLine);
        sent2[0] = "<s>";
        sent2[count+1] = "</s>";
        count+=2;
        vector<vector<LogP>>proba(count);
        vector<vector<VocabIndex>>indexTable(count);
        vector<vector<int>>backtrack(count);

        proba[0].push_back(0);
        indexTable[0].push_back(0);
        backtrack[0].push_back(-1);
        for(int t=1;t<count;t++)
        {
            int size = 0;
            auto vocab_iter = vocabmap.find(sent2[t]);

            if(vocab_iter==vocabmap.end())
            {
                cout << "I have no idea what to do" << endl;
            }
            for(auto it=vocab_iter->second.begin();it!=vocab_iter->second.end();++it)
            {
                VocabIndex w_t2 = voc.getIndex((*it).c_str());
                //outputFile<< sent2[t] << " "<< size << " "<<w_t2 << " "<<*it << " "<< it-vocab_iter->second.begin()<<"\n";
                w_t2 = (w_t2 == Vocab_None)? voc.getIndex(Vocab_Unknown): w_t2;
                LogP max_p = LogP_Zero;
                int max_id = 0;
                for(int i=0;i<indexTable[t-1].size();i++)
                {
                    string last_word = (vocabmap.find(sent2[t-1])->second)[indexTable[t-1][i]];
                    VocabIndex w_t1 = voc.getIndex(last_word.c_str());
                    w_t1 = (w_t1 == Vocab_None)? voc.getIndex(Vocab_Unknown): w_t1;

                    VocabIndex context[] = {w_t1, Vocab_None};

                    LogP logp = lm.wordProb(w_t2, context);
                    logp+=proba[t-1][i];

                    if(logp>max_p)
                    {
                        max_p = logp;
                        max_id = i;
                    }
                }

                proba[t].push_back(max_p);
                indexTable[t].push_back(it-vocab_iter->second.begin());
                backtrack[t].push_back(max_id);
                size++;
            }               
        }

        LogP max_p = LogP_Zero;
        int max_col = -1;
        for(int j=0;j<indexTable[count-1].size();j++)
        {
            if(proba[count-1][j]>max_p)
            {
                max_p = proba[count-1][j];
                max_col = indexTable[count-1][j];
            }
        }

        vector<string> AnsPath;
        for(int i=count-1;i>0;i--)
        {
            AnsPath.push_back(vocabmap[sent2[i]][indexTable[i][max_col]]);
            max_col = backtrack[i][max_col];
        }
        AnsPath.push_back("<s>");
        AnsPaths.push_back(AnsPath);
    }
    testFile.close();

    for(auto it1=AnsPaths.begin();it1!=AnsPaths.end();++it1)
    {
        for(auto it2=(*it1).rbegin();it2!=(*it1).rend();++it2)
        {
            outputFile << *it2 << " ";
        }
        outputFile << "\n";
    }
    outputFile.close();
    
    return 0;
}



