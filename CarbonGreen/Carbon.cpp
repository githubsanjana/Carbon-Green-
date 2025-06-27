#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

// Activity Base Class (for inheritance)
class Activity {
protected:
    string name;
    float co2Factor;
    float lastAmount = 0; // Track last input amount
public:
    virtual float calculateCO2(float amount) = 0;
    virtual string getSuggestion() = 0;
    string getName() const { return name; }
    float getLastAmount() const { return lastAmount; }
    void setLastAmount(float amount) { lastAmount = amount; }
};

// Derived Travel Class
class Travel : public Activity {
public:
    Travel() {
        name = "Travel";
        co2Factor = 0.21f;
    }
    
    float calculateCO2(float km) override {
        setLastAmount(km);
        return km * co2Factor;
    }
    
    string getSuggestion() override {
        return "Use public transport or carpool to reduce travel emissions";
    }
};

// Derived Electricity Class
class Electricity : public Activity {
public:
    Electricity() {
        name = "Electricity";
        co2Factor = 0.9f;
    }
    
    float calculateCO2(float units) override {
        setLastAmount(units);
        return units * co2Factor;
    }
    
    string getSuggestion() override {
        return "Switch to LED bulbs and unplug idle electronics";
    }
};

// Derived Food Class
class Food : public Activity {
public:
    Food() {
        name = "Meat Consumption";
        co2Factor = 2.5f;
    }
    
    float calculateCO2(float meals) override {
        setLastAmount(meals);
        return meals * co2Factor;
    }
    
    string getSuggestion() override {
        return "Try meatless meals 2-3 days per week";
    }
};

// Analytics Class
class Analytics {
public:
    static float getWeeklyAverage() {
        ifstream inFile("carbon_log.txt");
        float total = 0;
        int days = 0;
        string line;
        
        while (getline(inFile, line)) {
            size_t pos = line.find("CO2 Emitted: ");
            if (pos != string::npos) {
                float co2 = stof(line.substr(pos + 13));
                total += co2;
                days++;
            }
        }
        
        return days > 0 ? total/days : 0;
    }
};

// Logger Class (Singleton)
class Logger {
private:
    static Logger* instance;
    Logger() {}
    
public:
    static Logger* getInstance() {
        if (!instance) {
            instance = new Logger();
        }
        return instance;
    }
    
    void log(const string& message) {
        ofstream outFile("carbon_log.txt", ios::app);
        if (outFile.is_open()) {
            time_t now = time(0);
            tm* ltm = localtime(&now);
            
            outFile << "[" << put_time(ltm, "%Y-%m-%d %H:%M:%S") << "] " 
                   << message << endl;
        }
    }
    
    void showHistory() {
        ifstream inFile("carbon_log.txt");
        cout << "\n Emission History:\n";
        string line;
        while (getline(inFile, line)) {
            cout << line << endl;
        }
    }
};

Logger* Logger::instance = nullptr;

//  User Interface Class
class UserInterface {
public:
    static void displayWelcome() {
        cout << "🌿 CARBON CUT - Your Personal Emission Tracker\n";
        cout << "--------------------------------------------\n\n";
    }
    
    static void displayResults(float totalCO2, const vector<pair<string, float>>& breakdown) {
        cout << "\n Your Carbon Footprint Breakdown:\n";
        for (const auto& item : breakdown) {
            cout << " - " << item.first << ": " << item.second << " kg CO2\n";
        }
        
        cout << "\n Total CO2 Emitted Today: " << totalCO2 << " kg\n";
        
        float weeklyAvg = Analytics::getWeeklyAverage();
        if (weeklyAvg > 0) {
            cout << "\n Your Weekly Average: " << weeklyAvg << " kg/day\n";
            if (totalCO2 < weeklyAvg * 0.9) {
                cout << " Great job! You're doing better than your average!\n";
            }
        }
    }
    
    static void displayMenu() {
        cout << "\nMenu:\n";
        cout << "1. Calculate Today's Footprint\n";
        cout << "2. View History\n";
        cout << "3. Exit\n";
        cout << "Choice: ";
    }
    
    static void displaySuggestions(const vector<Activity*>& activities) {
        cout << "\n Suggestions:\n";
        bool allZero = true;
        
        for (Activity* activity : activities) {
            if (activity->getLastAmount() > 0) {
                cout << " - " << activity->getSuggestion() << "\n";
                allZero = false;
            }
        }
        
        if (allZero) {
            cout << "🎉 Perfect! Zero emissions today! Keep it up!\n";
        }
    }
};

int main() {
    UserInterface::displayWelcome();
    Logger* logger = Logger::getInstance();
    
    int choice;
    do {
        UserInterface::displayMenu();
        cin >> choice;
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: {
                Travel travel;
                Electricity electricity;
                Food food;
                
                vector<Activity*> activities = {&travel, &electricity, &food};
                vector<pair<string, float>> breakdown;
                float totalCO2 = 0;
                
                for (Activity* activity : activities) {
                    cout << "\nEnter " << activity->getName() << ": ";
                    float amount;
                    cin >> amount;
                    
                    float co2 = activity->calculateCO2(amount);
                    breakdown.emplace_back(activity->getName(), co2);
                    totalCO2 += co2;
                    
                    logger->log(activity->getName() + ": " + to_string(amount) + 
                              " → " + to_string(co2) + " kg CO2");
                }
                
                UserInterface::displayResults(totalCO2, breakdown);
                logger->log("TOTAL CO2: " + to_string(totalCO2) + " kg");
                UserInterface::displaySuggestions(activities);
                break;
            }
            case 2:
                logger->showHistory();
                break;
            case 3:
                cout << "\nThank you for using CarbonCut! 🌱\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 3);
    
    return 0;
}