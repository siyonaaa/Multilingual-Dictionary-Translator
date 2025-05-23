#include "hashtable.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

//function to convert a string to lowercase
string toLower(const string& str) {
    string lowerStr = str;
    for (char& c : lowerStr) {
    	//convert each character to lowercase
        c = tolower(c);
    }
    return lowerStr;
}

//constructor for Translation class
Translation::Translation(string meanings,string language) {
	//set the language of the translation
	this->language = language;
	//temporary string to store a meaning
	string cur = "";
	//add delimiter at the end to process last meaning
	meanings += ";";
	for (unsigned int i = 0; i < meanings.length(); i++) {
		if (meanings[i] != ';') {
			//build the meaning until ';' is found
			cur += meanings[i];
		}
		else {
			//if meaning is non-empty, add it
			if (!cur.empty()) {
				this->meanings.push_back(cur);
				//reset for the next meaning
				cur = "";
			}
 		}
	}
}

//add new meanings to an existing translation
void Translation::addMeaning(string newMeanings) {
	string cur = "";	
	//add delimiter to process the final meaning
    newMeanings += ";";  

    for (unsigned int i = 0; i < newMeanings.length(); i++) {
        if (newMeanings[i] != ';') {
        	//build current meaning
            cur += newMeanings[i];
        } 
		else {
            if (!cur.empty()) {
                //check if the meaning already exists (case-insensitive)
                bool exists = false;
                for (const string& meaning : meanings) {
                    if (toLower(meaning) == toLower(cur)) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                	//add new unique meaning
                    meanings.push_back(cur);
                }
                //reset for next meaning
                cur = "";
            }
        }
    }
}

//constructor for entry class
Entry::Entry(string word, string meanings,string language) {
	//set the word for the entry
	this->word = word;
	//set the deleted flag to false by default
	this->deleted = false;
	//create a Translation object with the provided meanings and language and add it to the translations list
	this->translations.push_back(Translation(meanings, language));
}

//add a new translation or update an existing one
void Entry::addTranslation(string newMeanings, string language) {
	for (unsigned int i = 0; i < translations.size(); i++) {
		//check if translation in given language exists (case-insensitive)
		if (toLower(translations[i].language) == toLower(language)) {
			//add meanings to existing translation
			translations[i].addMeaning(newMeanings);
			return;
		}
	}

	//if no existing translation found, create a new one
	translations.push_back(Translation(newMeanings, language));
}

//print the word's translations and meanings
void Entry::print() {
	//set column width for language name formatting
	const int width = 10;
	for (unsigned int i = 0; i < translations.size(); i++) { 
		//print language       
        cout << left << setw(width) << translations[i].language;
        //print colon
		cout << ": ";
       	//print meanings separated by semicolons
        for (unsigned int j = 0; j < translations[i].meanings.size(); j++) {
            cout << translations[i].meanings[j];
            if (j < translations[i].meanings.size() - 1) {
                cout << "; ";
            }
        }

        cout << endl;
    }
}

//constructor for hashtable, initializes buckets with nullptrs
HashTable::HashTable(int capacity) {
    buckets = new Entry*[capacity];		
	for(int i=0; i<capacity; i++)
		buckets[i] = nullptr;

	//store the given capacity
	this->capacity = capacity;
	//initialize number of entries to 0
	this->size = 0;
	//initialize collision count to 0
	this->collisions = 0;
}

//computes the hash code for a given word
unsigned long HashTable::hashCode(string word) {

	//cyclic shift hash method
	// word = toLower(word);
	//initialize the hash value to 0
	// unsigned long hash = 0;
	//iterate over each character in the input string
	// for (char c : word) {
		//cyclic-shift hash function
		//perform a bitwise left shift by 5 (equivalent to multiplying by 32)
		//and a right shift by 27
		// hash = (hash << 5) | (hash>>27);
		//add the ASCII value of the character to the hash
		// hash = hash + (unsigned long) c;
	// }
	//return the computed hash value
	// return hash;

	//polynomial hash method
	word = toLower(word);
	//initialize the hash value to 0
	unsigned long hash = 0;
	//choose a base value which is usually a small prime no. e.g. 31
	unsigned long base = 31;
	//iterate through each character in the word
	for (char c : word) {
		//update the hash value using the polynomial rolling formula:
    	//multiply current hash by base and add the ASCII value of the character
		hash = hash * base + (unsigned long)c;
	}
	//return the computed hash value
	return hash;

	//DJB2 hash method
	// word = toLower(word);
	//initialize the hash value with a magic number (5381) as per the DJB2 algorithm
	// unsigned long hash = 5381;
	//iterate through each character in the word
	// for (char c : word) {
		//perform the DJB2 hash calculation:
    	//hash * 33 is implemented as (hash << 5) + hash for efficiency
    	//then add the ASCII value of the current character
		// hash = ((hash << 5) + hash) + (unsigned char)c;
	// }
	//return the computed hash value
	// return hash;

}

//returns the current number of stored entries
unsigned int HashTable::getSize() {
    return size;
}

//returns the number of collisions that occurred during insertions
unsigned int HashTable::getCollisions() {
    return collisions;
}

//global flag to prevent insert error message from being printed when calling import
bool insertCalledFromImport = false;

//imports entries from a file at the given path
void HashTable::import(string path) {

	//check if user didn't enter file name
	if (path.empty()) {
		cout << "Please provide the file name you wish to import from." << endl;
		return;
	}

	ifstream file(path);
	//check if file opened successfully 
	if (!file.is_open()) {
		cout << "Could not open the file." << endl;
		return;
	}

	//disable errors in insert
	insertCalledFromImport = true;

	string line;
	string language;

	//read the first line to get the language
	if (getline(file, line)) {
		language = line;
	}
	else {
		cout << "Could not find the language." << endl;
		return;
	}

	int linesProcessed = 0;
	string word;
	string meanings;

	//process each remaining line
	while (getline(file, line)) {
		if (line.empty()) {
			continue;
		}
		//find the position of the colon separator
		size_t colonidx = line.find(':');
		if (colonidx == string::npos) continue;
		//extract the word and its meanings
		word = line.substr(0, colonidx);
		meanings = line.substr(colonidx + 1);
		//insert the entry into the table
		insert(word, meanings, language);
		linesProcessed++;
	}
	//confirmation message
	cout << linesProcessed << " " << language << " words have been imported successfully." << endl;
	//re-enable error messages in insert
	insertCalledFromImport = false;
	//close the file
	file.close();
}

//inserts a new word with its meanings and language into the hashtable
void HashTable::insert(string word, string meanings,string language) {

	//validate user input
	if (!insertCalledFromImport && (word.empty() || meanings.empty() || language.empty())) {
    cout << "Error: Please provide a word, at least one meaning, and the language." << endl;
    return;
	}
	
	//compute the initial index in the table
	unsigned long hashIndex = hashCode(toLower(word)) % capacity;
	//probing attempt counter
	unsigned int i = 0;
	//final index after probing
	unsigned int index = hashIndex;
	//quadratic probing to find an empty slot or the word if it already exists
	while (buckets[index] != nullptr && (toLower(buckets[index]->word)) != toLower(word) && !buckets[index]->deleted) {
		//count each collision
		collisions++;
		//increase the probe count
		i++;
		//quadratic probing formula
		index = (hashIndex + i*i) % capacity;	
	}

	//check if slot is empty or was marked as deleted. If either, then insert a new Entry
	if (buckets[index] == nullptr || buckets[index]->deleted) {
		buckets[index] = new Entry(word, meanings, language);
		//increase total number of stored entries
		size++;
		return;
	}	
	//if the word already exists, add translation to the entry
	Entry* entry = buckets[index];
	entry->addTranslation(meanings, language);
}

//delete a word entirely from the dictionary
void HashTable::delWord(string word) {

	//validate user input
	if (word.empty()) {
		cout << "Please provide the word you wish to delete." << endl;
		return;
	}

	//compute hash index using quadratic probing
	unsigned long hashIndex = hashCode(toLower(word)) % capacity;
	for (unsigned int i = 0; i < capacity; i++) {
        unsigned int index = (hashIndex + i * i) % capacity;
		
		//if empty slot is found, word does not exist
		if (buckets[index] == nullptr) {
			cout << word << " not found in the Dictionary." << endl;
			return;
		}
		//check for matching word that is not deleted
		if (!buckets[index]->deleted && (toLower(buckets[index]->word)) == toLower(word)) {
			buckets[index]->deleted = true;
			//decrement size
			size--;
			//confirmation message
			cout << word << " has been successfully deleted from the Dictionary." << endl;
			return;
		}
	}

	cout << word << " not found in the Dictionary." << endl;
}

//delete a translation of a word in a specific language
void HashTable::delTranslation(string word, string language) {
	//check if either word or language input is empty
	if (word.empty() || language.empty()) {
		cout << "Please provide the word and the language of its translation you wish to delete." << endl;
		return;
	}

	//compute the hash index using the lowercased word
	unsigned long hashIndex = hashCode(toLower(word)) % capacity;
	//perform quadratic probing to resolve collisions
	for (unsigned int i = 0; i < capacity; i++) {
        unsigned int index = (hashIndex + i * i) % capacity;
        //if a null bucket is reached, the word does not exist in the table
		if (buckets[index] == nullptr) {
			cout << word << " not found in the Dictionary." << endl;
			return;
		}

		//if the current bucket is not marked deleted and matches the word
		if (!buckets[index]->deleted && (toLower(buckets[index]->word)) == toLower(word)) {

			//access the list of translations for this word
			vector<Translation>& translations = buckets[index]->translations;

			//iterate through all translations to find the one matching the given language
			for (vector<Translation>::iterator p = translations.begin(); p != translations.end(); ++p) {
				//compare language case-insensitively
				if (toLower(p->language) == toLower(language)) {
					//remove the translation from the list
					translations.erase(p);
					cout << "Translation has been successfully deleted from the Dictionary." << endl;

					//if the word now has no translations, mark the bucket as deleted
					if (translations.empty()) {
						buckets[index]->deleted = true;
						size--;						
					}
					return;
				}
			}
			//if loop completes without match, the translation was not found
			cout << "Translation not found in the Dictionary." << endl;
			return;
		}
	}
	//if the word was never found in the probing sequence
	cout << word << " not found in the Dictionary." << endl;
}

//delete a specific meaning of a word in a certain language
void HashTable::delMeaning(string word, string meaning, string language) {

	//check if any of the input fields are empty
	if (word.empty() || language.empty() || meaning.empty()) {
		cout << "Please provide the word, its meaning, and its language that you wish to delete." << endl;
		return;
	}

	//compute initial index using the lowercase of the word
	unsigned long hashIndex = hashCode(toLower(word)) % capacity;

	//probe through the table using quadratic probing
	for (unsigned int i = 0; i < capacity; i++) {
        unsigned int index = (hashIndex + i * i) % capacity;

        //if a null slot is found, the word doesn't exist in the dictionary
		if (buckets[index] == nullptr) {
			cout << word << " not found in the Dictionary." << endl;
			return;
		}

		//if the slot is not marked deleted and the word matches
		if (!buckets[index]->deleted && (toLower(buckets[index]->word)) == toLower(word)) {
			//access the list of translations for this word
			vector<Translation>& translations = buckets[index]->translations;

			//iterate through each translation to find matching language
			for (unsigned int j = 0; j < translations.size(); j++) {
				if (toLower(translations[j].language) == toLower(language)) {
					//access the list of meanings for this translation
					vector<string>& meanings = translations[j].meanings;

					//search for the specific meaning to delete
					for (vector<string>::iterator p = meanings.begin(); p != meanings.end(); ++p) {
						if (toLower(*p) == toLower(meaning)) {

							//remove the meaning
							meanings.erase(p);

							//confirmation message
							cout << "Meaning has been successfully deleted from the Translation." << endl;

							//if no meanings remain, remove the entire translation
							if (meanings.empty()) {
								translations.erase(translations.begin() + j);
							}

							//if no translations remain, mark the bucket as deleted
							if (translations.empty()) {
								buckets[index]->deleted = true;
								size--;
							}							

							return;
						}
					}
					
					//meaning was not found in the translation
					cout << "Meaning not found in the Dictionary." << endl;
					return;
				}
			}
			//language was not found for the word
			cout << "Language not found in the Dictionary." << endl;
			return;
		}
	}
	//word was not found in the dictionary
	cout << word << " not found in the Dictionary." << endl;
}

//export all words of a given language to a file
void HashTable::exportData(string language, string filePath) {

	//attempt to open output file
	ofstream outFile(filePath);
	if (!outFile.is_open()) {
		cout << "Could not open the required file for writing." << endl;
		return;
	}
	//write the language as the first line in the file
	outFile << language << endl;
	int cnt = 0;
	//traverse all entries in the table
	for (unsigned int i = 0; i < capacity; i++) {
		Entry* entry = buckets[i];

		//skip null or deleted entries
		if (entry != nullptr && !entry->deleted) {
			//check each translation for the target language
			for (const Translation& T : entry->translations) {
				if (toLower(T.language) == toLower(language)) {
					//write the word followed by a colon
					outFile << entry->word << ":";

					//write all meanings separated by semicolons
					for (unsigned int j = 0; j < T.meanings.size(); j++) {
						outFile << T.meanings[j];
						if (j < T.meanings.size() - 1) {
							outFile << ";";
						}
					}

					outFile << endl;
					cnt++;
				}
			}
		}
	}
	//close the file after writing all the records
	outFile.close();
	//confirmation message
	cout << cnt << " records have been successfully exported to " << filePath << endl;
}

//searches for a word in the dictionary and prints its translations
void HashTable::find(string word) {

	//check if the input is empty
	if (word.empty()) {
		cout << "Please provide the word you wish to find." << endl;
		return;
	}

	//compute initial index using hash function
	unsigned long hashIndex = hashCode(toLower(word)) % capacity;
	int comparisons = 0;

	//probe the table using quadratic probing
	for (unsigned int i = 0; i < capacity; i++) {
        unsigned int index = (hashIndex + i * i) % capacity;
		comparisons++;

		//if a null slot is found, the word doesn't exist
		if (buckets[index] == nullptr) {
			cout << word << " not found in the Dictionary." << endl;
			return;
		}

		//if the slot is not deleted and the word matches
		if (!buckets[index]->deleted && (toLower(buckets[index]->word)) == toLower(word)) {
			cout << word << " found in the Dictionary after " << comparisons << " comparisons."<< endl;
			//print all translations and meanings
			buckets[index]->print();
			return;
		}
	}
	//word was not found in the dictionary after full probing
	cout << word << " not found in the Dictionary." << endl;
}

//destructor for hashtable, frees memory used by entries
HashTable::~HashTable() {

	//delete each entry in the hastable
	for (unsigned int i=0; i < capacity; i++) {
		if (buckets[i] != nullptr) {
			delete buckets[i];
		}
	}
	//delete the entire bucket array
	delete[] buckets;
}
