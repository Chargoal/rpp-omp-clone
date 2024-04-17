#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm> // For std::merge
#include <omp.h>

using namespace std;
using namespace std::chrono;

void bubbleSort(vector<int>& arr, int startIndex, int endIndex) {
    int n = endIndex - startIndex;
    for (int i = 0; i < n - 1; ++i) {
        for (int j = startIndex; j < endIndex - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}


int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Usage: ./ompSort <number of processes> <input file name>\n";
        return 1;
    }

    int numProcesses = atoi(argv[1]);
    string filename = argv[2];

    auto start = chrono::high_resolution_clock::now();
    ifstream inputFile(filename);
    if (!inputFile) {
        cout << "Error: Unable to open file " << filename << endl;
        return 1;
    }

    vector<int> numbers;
    int number;

    while (inputFile >> number) {
        numbers.push_back(number);
    }

    inputFile.close();

    int numbersSize = numbers.size();
    if (numbersSize == 0) {
        cout << "Error: No data found in file " << filename << endl;
        return 1;
    }

    // Divide data into chunks for each process
    int chunkSize = numbersSize / numProcesses;
    // Temporary vectors to store sorted chunks
    vector<vector<int>> sortedChunks(numProcesses);

    // Parallel sorting of chunks
    #pragma omp parallel num_threads(numProcesses)
    {
        // Compute the start and end index for each thread's chunk
        int tid = omp_get_thread_num();
        int startIndex = tid * chunkSize;
        int endIndex = min((tid + 1) * chunkSize, numbersSize); // Ensure endIndex does not exceed vector size

        // Sort the chunk using bubble sort
        bubbleSort(numbers, startIndex, endIndex);

        // Store the sorted chunk in the temporary vector
        sortedChunks[tid] = vector<int>(numbers.begin() + startIndex, numbers.begin() + endIndex);
    }

    // Merge sorted chunks
    for (int stride = 1; stride < numProcesses; stride *= 2) {
        int mergeStep = 2 * stride;
        #pragma omp parallel for num_threads(numProcesses)
        for (int i = 0; i < numProcesses; i += mergeStep) {
            int startIndex = i * chunkSize;
            int middleIndex = (i + stride) * chunkSize;
            int endIndex = min((i + mergeStep) * chunkSize, numbersSize);
            merge(sortedChunks[i].begin(), sortedChunks[i].begin() + middleIndex - startIndex,
                sortedChunks[i + stride].begin(), sortedChunks[i + stride].begin() + endIndex - middleIndex,
                numbers.begin() + startIndex);
        }
    }


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Sorting duration: " << duration.count() << " milliseconds" << endl;

    ofstream outputFile("output.txt");
    if (!outputFile) {
        cout << "Error: Unable to create output file." << endl;
    } else {
        for (int num : numbers) {
            outputFile << num << " ";
        }
        outputFile.close();
    }

    return 0;
}