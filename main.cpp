#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <bitset>
#include <string>
#include <algorithm>
using namespace std;

//variables
//everything will be maintained in an array of bitsets, with each bitset representing the allowed letters of a certain space
std::array<std::bitset<26>,5> allowed_at_pos;
std::bitset<26> allowed; //represents all allowed and disallowed letters
std::bitset<26> mustAppear; //represents words we've found that must be in the word

//the letter scores represent how common the letters are in the english language, used to rank how good each word is as a guess
std::array<int,26> letterScores = {8,2,4,3,11,2,2,3,7,0,1,5,3,7,7,3,0,7,6,7,4,1,1,0,2,0};
constexpr int WORD_LENGTH = 5; //I don't think I'll ever change this, but I could make it so this is derived from the passed in word list idk


vector<string> loadWords(const char* filename) {
    ifstream inFile(filename);
    if (!inFile) {
        cout << "Could not open file " << filename << endl;
        //return an empty array just so we exit sooner
        return vector<string>();
    }
    vector<string> words;
    string word;
    while (getline(inFile, word)) {
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    return words;
}

//this is for debug purposes only
void printList(vector<string> words) {
    for (string word : words) {
        cout << word << endl;
    }
}

//this is how we will check individual words, check them against the bitset
bool violatesConstraints(string word) {
    //implement later
    //check every letter in the word against the bitsets
    for (int i=0;i<WORD_LENGTH;i++) {
        //get the letter as an index to compare to our bitsets
        const int letter = word[i] - 'a';
        //letter must be allowed in the word as a whole and at the position given,
        if (!allowed[letter] or !allowed_at_pos[i][letter]) {
            return true;
        }
    }
    //now check against required letters
    for (int j = 0; j < 26; j++) {
        if (mustAppear[j]) {
            char c = 'a' + j;
            if (word.find(c) == string::npos) {
                return true;
            }
        }
    }
    return false;
}

bool colorCodeIsBad(string colorCode) {
    //check if the color code letters are bad
    //if the color code is not equal to
    if (colorCode.size() != WORD_LENGTH){
        return true;
    }
    std::string colors = "gby";
    //check if each color in the code is valid
    for (int i=0;i<WORD_LENGTH;i++) {
        if (colors.find(colorCode[i]) == string::npos) {
            return true;
        }
    }
    //if we get here we're good
    return false;
}

bool wordIsBad(string word) {
    //very similar to above function
    if (word.size() != WORD_LENGTH){
        return true;
    }
    std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
    //check that the word only consists of valid english letters
    for (int i=0;i<WORD_LENGTH;i++) {
        if (alphabet.find(word[i]) == string::npos) {
            return true;
        }
    }
    //if we get here we're good
    return false;
}

bool takeResults(string word, string colorCode) {
    //if we're given bad parameters return false
    if (colorCodeIsBad(colorCode) || wordIsBad(word)) {
        return false;
    }
    //need to make a pretty big change here to solve for broken logic, we'll do a 2 pass system and do some fancy counts
    array<int, 26> guessCount = {0};
    array<int, 26> greenYellowCount = {0};

    //count the number of occurrences of each letter in the word
    for (char c : word) {
        guessCount[c - 'a']++;
    }

    //count the number of occurrences of those letters that were good
    for (int i = 0; i < WORD_LENGTH; i++) {
        char c = word[i];
        if (colorCode[i] == 'g' || colorCode[i] == 'y') {
            greenYellowCount[c - 'a']++;
        }
    }

    //now continue like we were before and handle logic on each letter
    for (int i=0;i<WORD_LENGTH;i++) {
        if (colorCode[i] == 'g') {
            //code is green, set all bits to 0, then set the correct letter to 1
            int letter = word[i] - 'a';
            allowed_at_pos[i].reset();
            allowed_at_pos[i].set(letter);
            allowed.set(letter);
            mustAppear.set(letter);
        }
        if (colorCode[i] == 'y') {
            //code is yellow, set the current position to not allow that letter, but yes in the overall
            int letter = word[i] - 'a';
            allowed_at_pos[i].reset(letter);
            allowed.set(letter);
            mustAppear.set(letter);
        }
        if (colorCode[i] == 'b') {
            //here's where things mess up, the greens/yellows influence behavior of black tiles
            int letter = word[i] - 'a';
            if (greenYellowCount[letter] == 0) {
                //this letter is not in the word at all, treat it like a normal black tile
                allowed.reset(letter);
                for (int j=0;j<WORD_LENGTH;j++){
                    allowed_at_pos[j].reset(letter);
                }
            }
            else {
                // the letter also appeared somewhere else in the word, so we only know it's not in this space
                allowed_at_pos[i].reset(letter);
            }

        }
    }
    return true;
}

vector<string> getValidWords(vector<string> allWords){
    //walk through every word in the mega list, check if it violates constraints, pass out a vector of all the valid ones
    vector<string> validWords;
    for (string word : allWords) {
        //now realizing the naming convention of that function makes this one slightly less readable, oh well
        if (!violatesConstraints(word)) {
            validWords.push_back(word);
        }
    }
    return validWords;
}

int scoreWord(string word) {
    bool seen[26] = {};
    int score = 0;
    bool uniqueLetters = true;
    for (char c : word) {
        score += letterScores[c - 'a'];
        //if we've already seen this letter before, no unique letter bonus for you >:(
        if (seen[c - 'a']) {
            uniqueLetters = false;
        }
        seen[c - 'a'] = true;
    }
    if (uniqueLetters) {
        //bonus for all letters being distinct
        score*=2;
    }
    return score;

}

vector<string> giveAnswer(vector<string> words, int topN=3) {
    //walk through all valid words, score them, return the best ones
    //by default returns the best 3 but I can choose to return however many I want
    struct scoredWord {
        string word;
        int score = 0;
    };
    vector<scoredWord> scored;

    for (string word : words) {
        scoredWord newScored;
        newScored.word = word;
        newScored.score = scoreWord(word);
        scored.push_back(newScored);
    }

    //this will sort all of the words by their scores
    //wtf why is clion grammar checking my comments lmao
    std::sort(scored.begin(), scored.end(),
          [](const scoredWord& a, const scoredWord& b) {
              return a.score > b.score;   // highest score first
    });

    //have to include the topN as a limit instead, I think this is why it was crashing sometimes
    int limit = min(topN, (int)scored.size());
    vector<string> answers;
    for (int i = 0; i < limit; i++) {
        answers.push_back(scored[i].word);
    }

    return answers;
}

void printAnswers(vector<string> answers) {
    cout << "Best answers:" << endl;
    for (string word : answers) {
        cout << word << endl;
    }
}

int main() {
    allowed.set();
    mustAppear.reset();
    for (auto &bs : allowed_at_pos) {
        bs.set(); // all letters allowed in all positions
    }

    vector<string> words = loadWords("nyt-wordle-words.txt");
    string guess;
    string colorCode;
    bool solved = false;
    bool failed = false;

    //always give the same starting recommendation
    cout << "Best answer(s): crane, slate, trace" << endl;
    for (int i=0;i<6;i++) {
        //take the answer from the user and make sure its valid
        do {
            cout << "Enter your last guess: " << endl;
            cin >> guess;
            cout << "Enter the color code of your last guess: " << endl;
            cin >> colorCode;
        } while (!takeResults(guess, colorCode));
        //if the color code is all greens, break out and print yippee or something
        if (colorCode == "ggggg") {
            cout << "Wordle solved in " << i+1 << " attempts." << endl;
            solved = true;
            break;
        }
        //produce the best results, also update the word list
        words = getValidWords(words);
        if (words.empty()) {
            cout << "No answers found." << endl;
            failed = true;
            break;
        }
        vector <string> answers = giveAnswer(words);
        printAnswers(answers);
    }
    if (failed) {
        cout << "WordleBot could not solve the Wordle." << endl;
        cout << "WordleBot has failed you. WordleBot is sorry :(" << endl;
    }
    if (solved) {
        cout << "Beep Boop WordleBot has helped you solve the Wordle :)" << endl;
    }

    return 0;
}