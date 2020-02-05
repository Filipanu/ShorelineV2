#include <iostream>
#include <chrono>
#include <ctime>
#include <fstream>
#include <thread>
#include <mutex>
#include <array>

typedef unsigned long long uint64_t;

using namespace std;

#define TEST_SIZE 1000
#define THREAD_SIZE 100

std::mutex counterMutex;
std::mutex writeDurationMutex;
std::mutex writeIdsMutex;

class Node {
	uint64_t nodeId;
	uint64_t extraCounter;
	uint64_t sessionId;

public:
	Node() : nodeId(0), sessionId(0), extraCounter(0) {}
	Node(uint64_t id, int sId) : nodeId(id), sessionId(sId), extraCounter(0) {}

	uint64_t getNodeId() { return this->nodeId; }
	void setNodeId(uint64_t id) { this->nodeId = id; }

	//millisecondsSinceEpoch since epoch
	uint64_t getTimestamp() {
		uint64_t millisecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();

		return millisecondsSinceEpoch;
	}

	//10 bits nodeId + 2 bits sessionId + 11 bits extraCounter (2048 ids) + 41 bits timestamp(ms)
	uint64_t getId() {
		uint64_t GUID = 0;

		GUID |= (nodeId << 54);
		GUID |= (sessionId << 52);
		{
			//RAII style mutex
			const std::lock_guard<std::mutex> lock(counterMutex);
			GUID |= (extraCounter << 41);
			if (extraCounter == 2047)
				extraCounter = 0;
			else
				extraCounter++;
		}
		GUID |= this->getTimestamp();

		return GUID;
	}
};

void cleanup();
int getSessionId();
void decToBinary(uint64_t n);
void displayArray(uint64_t arr[], int arrSize);
void threadFunc(Node *node);

int main()
{
	//cleanup of previous files
	cleanup();
	int sessionId = getSessionId();
	std::cout << "SessionId = " << sessionId << "\n";

	Node *node = new Node(256, sessionId);
	cout << node->getNodeId() << " " << node->getTimestamp() << endl;

	thread threadArr[THREAD_SIZE];

	for (int i = 0; i < THREAD_SIZE; i++) {
		threadArr[i] = thread(threadFunc, node);
	}

	for (int i = 0; i < THREAD_SIZE; i++) {
		threadArr[i].join();
	}

	cout << "Main done!\n";

	delete node;
	return 0;
}

void threadFunc(Node *node) {
	volatile int counter = 0;
	uint64_t arr[TEST_SIZE] = { 0 };

	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < TEST_SIZE; i++) {
		arr[counter] = node->getId();
		counter++;
	}
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> d = end - start;

	{
		const std::lock_guard<std::mutex> lock(writeDurationMutex);

		std::ofstream writeFile;
		writeFile.open("durations.txt", std::ofstream::app);
		writeFile << d.count() << "\n";
		writeFile.close();
	}

	{
		const std::lock_guard<std::mutex> lock(writeIdsMutex);

		std::ofstream writeFile;
		writeFile.open("GUIDs.txt", std::ofstream::app);
		for (int i = 0; i < TEST_SIZE; i++) {
			writeFile << arr[i] << "\n";
		}
		writeFile.close();
		//cout << "Thread: "<<std::this_thread::get_id()<<"|Duration: " << d.count() << "s\n";
		//displayArray(arr, TEST_SIZE);
	}

}

void cleanup() {
	std::ofstream writeFile;
	writeFile.open("durations.txt", std::ofstream::trunc);
	writeFile.close();

	writeFile.open("GUIDs.txt", std::ofstream::trunc);
	writeFile.close();
}

int getSessionId() {
	std::ifstream readFile;
	std::ofstream writeFile;
	int sessionId = 0;

	readFile.open("sessionId.txt");
	if (readFile.is_open()) {
		readFile >> sessionId;
		writeFile.open("sessionId.txt", std::ofstream::trunc);
		
		if (sessionId >= 3)
			writeFile << 0;
		else 
			writeFile << sessionId + 1;
	}
	else {
		writeFile.open("sessionId.txt", std::ofstream::trunc);
		writeFile << sessionId + 1;
	}
	return sessionId;
}

void displayArray(uint64_t arr[], int arrSize) {

	for (int i = 0; i < arrSize; i++) {
		cout << "arr[" << i << "]= ";
		decToBinary(arr[i]);
	}
	cout << "done display" << endl;
}

// simple function to display a number in binary
void decToBinary(uint64_t n)
{
	// size of an integer is assumed to be 64 bits 
	for (int i = 63; i >= 0; i--) {
		uint64_t k = n >> i;
		if (k & 1)
			cout << "1";
		else
			cout << "0";
		if (i == 54)
			cout << " ";
		else if (i == 52)
			cout << " ";
		else if (i == 41)
			cout << " ";
		else if (i == 20)
			cout << " ";
	}
	cout << endl;
}
