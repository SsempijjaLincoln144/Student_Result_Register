#include <iostream>      
#include <fstream>       
#include <string>        
#include <sstream>       
#include <stdexcept>     
#include <chrono>        
#include <random>        
#include <algorithm>     
#include <cstdlib>       
#include <iomanip>       

// ---------------------------------------------------------------------
// 1. RECORD TYPE
// ---------------------------------------------------------------------
struct StudentRecord {          
    int    id;                  
    std::string name;           
    std::string course;         
    double score;               
};

// ---------------------------------------------------------------------
// 2. PRIMARY DATA STRUCTURE (A1): DynamicArray<T>
// ---------------------------------------------------------------------
template <typename T>
class DynamicArray {
private:
    T*      data;               
    int     count;              
    int     capacity;           

    void grow() {
        int newCap = (capacity == 0) ? 1 : capacity * 2;   
        T* newData = new T[newCap];                        
        for (int i = 0; i < count; ++i)                    
            newData[i] = data[i];                          
        delete[] data;                                     
        data     = newData;                                
        capacity = newCap;                                 
    }

public:
    DynamicArray() : data(nullptr), count(0), capacity(0) {}

    DynamicArray(const DynamicArray& other)
        : data(nullptr), count(other.count), capacity(other.capacity) {
        if (capacity > 0) {
            data = new T[capacity];                        
            for (int i = 0; i < count; ++i)
                data[i] = other.data[i];                   
        }
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this == &other) return *this;                  
        T* newData = nullptr;
        if (other.capacity > 0) {
            newData = new T[other.capacity];               
            for (int i = 0; i < other.count; ++i)
                newData[i] = other.data[i];
        }
        delete[] data;                                     
        data     = newData;                                
        count    = other.count;
        capacity = other.capacity;
        return *this;
    }

    ~DynamicArray() {
        delete[] data;                                     
        data = nullptr;                                    
    }

    void push_back(const T& value) {
        if (count == capacity) grow();                     
        data[count++] = value;                             
    }

    void removeAt(int index) {
        if (index < 0 || index >= count)                   
            throw std::out_of_range("removeAt: index out of range");
        for (int i = index; i < count - 1; ++i)            
            data[i] = data[i + 1];
        --count;                                           
        if (capacity > 4 && count < capacity / 4) {
            int newCap = capacity / 2;
            T* newData = new T[newCap];
            for (int i = 0; i < count; ++i) newData[i] = data[i];
            delete[] data;
            data = newData;
            capacity = newCap;
        }
    }

    T& at(int index) {
        if (index < 0 || index >= count)
            throw std::out_of_range("at: index out of range");
        return data[index];
    }
    const T& at(int index) const {                         
        if (index < 0 || index >= count)
            throw std::out_of_range("at: index out of range");
        return data[index];
    }

    int size()     const { return count; }                 
    int getCapacity() const { return capacity; }           
    bool empty()   const { return count == 0; }            

    template <typename Pred>
    int findIndex(Pred pred) const {
        for (int i = 0; i < count; ++i)                    
            if (pred(data[i])) return i;                   
        return -1;                                         
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream in(filename);                        
        if (!in) throw std::runtime_error("cannot open input file: " + filename);
        std::string line;
        while (std::getline(in, line)) {                   
            if (line.empty()) continue;                    
            std::istringstream ss(line);                   
            std::string idStr, name, course, scoreStr;
            if (!std::getline(ss, idStr,   ',')) continue;
            if (!std::getline(ss, name,    ',')) continue;
            if (!std::getline(ss, course,  ',')) continue;
            if (!std::getline(ss, scoreStr,',')) continue;
            StudentRecord r;                               
            r.id     = std::stoi(idStr);                   
            r.name   = name;
            r.course = course;
            r.score  = std::stod(scoreStr);
            push_back(r);                                  
        }
    }

    void saveToFile(const std::string& filename) const {
        std::ofstream out(filename);                       
        if (!out) throw std::runtime_error("cannot open output file: " + filename);
        for (int i = 0; i < count; ++i) {                  
            out << data[i].id << ',' << data[i].name << ','
                << data[i].course << ',' << data[i].score << '\n';
        }
    }

    T*       raw()       { return data; }
    const T* raw() const { return data; }
};

// ---------------------------------------------------------------------
// 3. SORTING & SEARCHING (A2)
// ---------------------------------------------------------------------
template <typename T, typename Compare>
void insertionSort(T* a, int n, Compare less) {
    for (int i = 1; i < n; ++i) {                          
        T key = a[i];                                      
        int j = i - 1;
        while (j >= 0 && less(key, a[j])) {                
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;                                    
    }
}

template <typename T, typename Compare>
void mergeRuns(T* a, T* tmp, int lo, int mid, int hi, Compare less) {
    int i = lo, j = mid + 1, k = lo;                       
    while (i <= mid && j <= hi) {                          
        if (less(a[j], a[i])) tmp[k++] = a[j++];           
        else                  tmp[k++] = a[i++];           
    }
    while (i <= mid) tmp[k++] = a[i++];                    
    while (j <= hi)  tmp[k++] = a[j++];                    
    for (int t = lo; t <= hi; ++t) a[t] = tmp[t];          
}

template <typename T, typename Compare>
void mergeSortRec(T* a, T* tmp, int lo, int hi, Compare less) {
    if (lo >= hi) return;                                  
    int mid = lo + (hi - lo) / 2;                          
    mergeSortRec(a, tmp, lo, mid, less);                   
    mergeSortRec(a, tmp, mid + 1, hi, less);               
    mergeRuns(a, tmp, lo, mid, hi, less);                  
}

template <typename T, typename Compare>
void mergeSort(T* a, int n, Compare less) {
    if (n < 2) return;                                     
    T* tmp = new T[n];                                     
    mergeSortRec(a, tmp, 0, n - 1, less);                  
    delete[] tmp;                                          
}

template <typename T, typename Key, typename Compare>
int binarySearch(const T* a, int n, const Key& key, Compare less) {
    int lo = 0, hi = n - 1;                                
    while (lo <= hi) {                                     
        int mid = lo + (hi - lo) / 2;                      
        if (less(a[mid], key))      lo = mid + 1;          
        else if (less(key, a[mid])) hi = mid - 1;          
        else                        return mid;            
    }
    return -1;                                             
}

// ---------------------------------------------------------------------
// 4. COMPARATORS
// ---------------------------------------------------------------------
struct ByScore { bool operator()(const StudentRecord& x, const StudentRecord& y) const { return x.score < y.score; } };
struct ScoreCompare {
    bool operator()(const StudentRecord& a, double k) const { return a.score < k; }
    bool operator()(double k, const StudentRecord& a) const { return k < a.score; }
};

// ---------------------------------------------------------------------
// 5. MAIN
// ---------------------------------------------------------------------
int main() {
    // FIXED: Your last digit is 0 (Even) -> USE MERGE SORT
    const int REG_LAST_DIGIT = 0; 
    const bool USE_MERGE = (REG_LAST_DIGIT % 2 == 0); 

    try {
        DynamicArray<StudentRecord> reg;                    
        reg.loadFromFile("students.txt");                   
        std::cout << "Loaded " << reg.size()
                  << " records (capacity " << reg.getCapacity() << ")\n";

        insertionSort(reg.raw(), reg.size(), ByScore{});    
        std::cout << "After insertionSort (by score):\n";
        for (int i = 0; i < reg.size(); ++i)
            std::cout << "  " << reg.at(i).id << "  " << reg.at(i).name
                      << "  " << reg.at(i).score << "\n";

        double probe = 75.0;                                
        int idx = binarySearch(reg.raw(), reg.size(), probe, ScoreCompare{});
        std::cout << "binarySearch(score=" << probe << ") -> "
                  << (idx >= 0 ? "found at index " + std::to_string(idx) : "absent") << "\n";

        probe = 999.0;
        idx = binarySearch(reg.raw(), reg.size(), probe, ScoreCompare{});
        std::cout << "binarySearch(score=" << probe << ") -> "
                  << (idx >= 0 ? "found" : "absent (expected)") << "\n";

        // This will now use MERGE SORT
        std::cout << "Chosen O(n log n) sort: mergeSort\n";
        mergeSort(reg.raw(), reg.size(), ByScore{});

        reg.saveToFile("students_out.txt");
        std::cout << "Saved updated records to students_out.txt\n";

        std::cout << "\n=== A4 TIMING: insertionSort vs mergeSort ===\n";
        std::mt19937 rng(42);                               
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        std::cout << std::setw(8) << "n"
                  << std::setw(14) << "insertion(ms)"
                  << std::setw(14) << "mergeSort(ms)"
                  << std::setw(10) << "ratio\n";
        double prevIns = 0, prevFast = 0;                   
        for (int n = 1000; n <= 64000; n *= 2) {
            double insTotal = 0, fastTotal = 0;             
            for (int run = 0; run < 3; ++run) {
                DynamicArray<StudentRecord> a, b;           
                for (int i = 0; i < n; ++i) {
                    StudentRecord r{i, "S", "C", dist(rng)};
                    a.push_back(r);                         
                    b.push_back(r);                         
                }
                auto t0 = std::chrono::high_resolution_clock::now();
                insertionSort(a.raw(), a.size(), ByScore{}); 
                auto t1 = std::chrono::high_resolution_clock::now();
                
                mergeSort(b.raw(), b.size(), ByScore{}); // ALWAYS MERGE SORT
                
                auto t2 = std::chrono::high_resolution_clock::now();
                insTotal  += std::chrono::duration<double, std::milli>(t1 - t0).count();
                fastTotal += std::chrono::duration<double, std::milli>(t2 - t1).count();
            }
            double insAvg  = insTotal  / 3.0;               
            double fastAvg = fastTotal / 3.0;
            double fastRatio = prevFast > 0 ? fastAvg / prevFast : 0; 
            std::cout << std::setw(8) << n
                      << std::setw(14) << std::fixed << std::setprecision(3) << insAvg
                      << std::setw(14) << fastAvg
                      << std::setw(10) << fastRatio << "\n";
            prevIns = insAvg; prevFast = fastAvg;
        }

    } catch (const std::exception& ex) {                    
        std::cerr << "FATAL: " << ex.what() << "\n";        
        return 1;
    }
    return 0;                                               
}