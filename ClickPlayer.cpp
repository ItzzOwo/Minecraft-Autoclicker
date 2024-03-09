#include <Windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <random>
#include <vector>

bool shouldClick = true;
std::vector<int> clickDelays;

std::vector<int> readIntervalsFromFile(const std::string& filePath) {
    std::vector<int> intervals;
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << "\n";
        return intervals;
    }

    std::string interval;
    while (std::getline(inputFile, interval, ',')) {
        intervals.push_back(std::stoi(interval));
    }

    inputFile.close();
    return intervals;
}

void bubbleSort(std::vector<int>& vec) {
    int n = vec.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (vec[j] > vec[j + 1]) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
}

double kernelDensityEstimation(const std::vector<int>& samples, double x, double bandwidth) {
    double sum = 0.0;
    for (int sample : samples) {
        sum += exp(-0.5 * pow((x - sample) / bandwidth, 2));
    }
    return sum / (samples.size() * sqrt(2 * 3.14159265359) * bandwidth);
}

int sampleFromKDE(const std::vector<int>& samples, double bandwidth) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(0.0, 1.0);

    while (true) {
        double x = dis(gen) * (samples.back() - samples.front()) + samples.front();
        double y = dis(gen);

        if (y < kernelDensityEstimation(samples, x, bandwidth)) {
            return static_cast<int>(x);
        }
    }
}

void playClicks(const std::string& intervalsFilePath) {
    std::vector<int> intervals = readIntervalsFromFile(intervalsFilePath);
    if (intervals.empty()) {
        std::cerr << "No intervals found in file.\n";
        return;
    }
    std::cout << "Intervals loaded.\n";

    bubbleSort(intervals);

    double bandwidth = 20.0;

    while (true) {
        if (GetAsyncKeyState(VK_LBUTTON) && shouldClick) {
            HWND window = GetForegroundWindow();
            if (FindWindowA(("LWJGL"), nullptr) == GetForegroundWindow()) {
                int clickInterval = sampleFromKDE(intervals, bandwidth);
                std::cout << "Interval: " << clickInterval << "ms\n";
                Sleep(clickInterval);
                SendMessageW(GetForegroundWindow(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
                SendMessageW(GetForegroundWindow(), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
            }
        }
        if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
            shouldClick = false;
        }
        else {
            shouldClick = true;
        }
    }
}

void saveClickDelaysToFile(const std::string& filePath) {
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << "\n";
        return;
    }

    for (size_t i = 0; i < clickDelays.size(); ++i) {
        outputFile << clickDelays[i];
        if (i < clickDelays.size() - 1) {
            outputFile << ",";
        }
    }

    outputFile.close();
}


void clickRecorder() {
    std::cout << "Click anywhere to start recording. Press left shift to save recording.\n";

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER firstClickTime = {0};
    LARGE_INTEGER secondClickTime = {0};

    bool recording = false;
    while (!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (!recording) {
                std::cout << "Recording...\n";
                QueryPerformanceCounter(&firstClickTime);
                recording = true;
            } else {
                QueryPerformanceCounter(&secondClickTime);
                float delay = static_cast<float>((secondClickTime.QuadPart - firstClickTime.QuadPart) * 1000.0 / frequency.QuadPart);
                if (delay <= 150.0f) {
                    clickDelays.push_back(delay);
                    std::cout << "Recorded delay: " << delay << "ms\n";
                } else {
                    std::cout << "Delay too long (" << delay << "ms). Ignoring.\n";
                }
                firstClickTime = secondClickTime;
            }

            while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {}
        }
    }

    std::ofstream outputFile("click_delays.txt");
    if (outputFile.is_open()) {
        for (size_t i = 0; i < clickDelays.size(); ++i) {
            outputFile << clickDelays[i];
            if (i != clickDelays.size() - 1) {
                outputFile << ",";
            }
        }
        outputFile.close();
        std::cout << "Recording saved to click_delays.txt.\n";
    } else {
        std::cerr << "Unable to open file for writing.\n";
    }
}
int main() {
    SetConsoleTitle("Owo Clicker");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 0x0B);
    std::cout << " _____           \n";
    std::cout << "|     |_ _ _ ___ \n";
    std::cout << "|  |  | | | | . |\n";
    std::cout << "|_____|_____|___|\n";
    std::cout << "\n";

    int choice;
    std::cout << "1. Click Player\n";
    std::cout << "2. Click Recorder\n";
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    std::cin.ignore();

    if (choice == 1) {
        system("cls");
        std::string intervalsFilePath;
        std::cout << "Enter intervals file path: ";
        std::getline(std::cin, intervalsFilePath);
        system("cls");
        playClicks(intervalsFilePath);
    }
    else if (choice == 2) {
        system("cls");
        clickRecorder();
    }
    else {
        std::cout << "Invalid choice.\n";
    }

    return 0;
}
