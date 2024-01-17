#include <iostream>
#include <string>
#include <vector>
#include <deque>

#include "lab4.h"

#define PAGE_COUNT 6
#define MAX_COUNTER 5

using namespace std;


class pageTableItem
{
public:
    int virtualPageNumber;
    int r;
    int m;
    unsigned long counter;

    pageTableItem() : pageTableItem(-1) { };
    pageTableItem(int vpn) {
        virtualPageNumber = vpn;
        r = 0;
        m = 0;
        counter = 0;
    };

    void read() {
        r = 1;
    };

    void write() {
        r = 1;
        m = 1;
    };
};

int findPage(vector<pageTableItem>* pages, int virtualPageNumber) {
    int index = -1;
    for (int i = 0; i < (*pages).size(); i++) {
        if ((*pages)[i].virtualPageNumber == virtualPageNumber) {
            index = i;
            break;
        }
    }
    return index;
};

class replacingAlgorithm {
public:
    virtual int loadPage(vector<pageTableItem>* pages, int virtualPageNumber) = 0;
};

class secondChance : public replacingAlgorithm
{
private:
    deque<int> _loadedPages;

public: 
    int loadPage(vector<pageTableItem>* pages, int virtualPageNumber) override {

        int physicalPage = findPage(pages, -1);

        if (physicalPage != -1) {
            (*pages)[physicalPage] = pageTableItem(virtualPageNumber);
            _loadedPages.push_back(physicalPage);
            return physicalPage;
        }

        for (int i = 0; i < _loadedPages.size(); i++) {
            auto physicalPage = _loadedPages.front();
            _loadedPages.pop_front();
            if (!(*pages)[physicalPage].r) {
                (*pages)[physicalPage] = pageTableItem(virtualPageNumber);
                _loadedPages.push_back(physicalPage);

                return physicalPage;
            }
            else {
                _loadedPages.push_back(physicalPage);
            }
        }
        return -1;
    }
};

class notFrequentlyUsed : public replacingAlgorithm
{
public:
    int loadPage(vector<pageTableItem>* pages, int virtualPageNumber) override {

        int physicalPage = findPage(pages, -1);

        if (physicalPage != -1) {
            (*pages)[physicalPage] = pageTableItem(virtualPageNumber);
            return physicalPage;
        }

        vector<int> oldestPhysicalPages;
        unsigned long minAge = UINT64_MAX;
        for (int i = 0; i < (*pages).size(); i++)
        {
            if ((*pages)[i].counter == minAge) {
                oldestPhysicalPages.push_back(i);
            }
            else if ((*pages)[i].counter < minAge)
            {
                minAge = (*pages)[i].counter;
                oldestPhysicalPages.clear();
                oldestPhysicalPages.push_back(i);
            }
        }

        int index = 0;
        if (oldestPhysicalPages.size() == 0) {
            return -1;
        }
        else if (oldestPhysicalPages.size() > 1) {
            index = uniform_rnd(0, oldestPhysicalPages.size() - 1);
        }

        (*pages)[oldestPhysicalPages[index]] = pageTableItem(virtualPageNumber);
        return oldestPhysicalPages[index];
    }
};

class pageTable
{
private:
    vector<pageTableItem> _pages;
    replacingAlgorithm& _algorithm;
    int _counter;
    int _maxCounter;

    void resetRBits() {
        for (int i = 0; i < _pages.size(); i++) {
            _pages[i].counter += _pages[i].r;
            _pages[i].r = 0;
        }
        _counter = 0;
    }

    int getPhisycalPage(int pageNumber) {
        if (_counter >= _maxCounter)
            resetRBits();
        _counter++;

        int index = findPage(&_pages, pageNumber);
        if (index == -1) {
            index = _algorithm.loadPage(&_pages, pageNumber);
        }
        
        return index;
    }

public:
    pageTable(int pageCount, int maxCounter, replacingAlgorithm& algorithm) : _algorithm(algorithm)
    {
        _counter = 0;
        _maxCounter = maxCounter;
        for (int i = 0; i < pageCount; i++)
        {
            pageTableItem page;
            _pages.push_back(page);
        }
    };

    void read(int pageNumber) {
        int index = getPhisycalPage(pageNumber);
        _pages[index].read();
    };

    void write(int pageNumber) {
        int index = getPhisycalPage(pageNumber);
        _pages[index].write();
    };

    string getState()
    {
        string state = "";
        for (auto i = _pages.begin(); i != _pages.end(); ++i)
        {
            if (i->virtualPageNumber == -1)
                state.append("# ");
            else
                state.append(to_string(i->virtualPageNumber) + " ");
        }
        state = state.substr(0, state.length() - 1);

        return state;
    };
};

int main(int argc, char *argv[])
{
    short algorithm = atoi(argv[1]);

    //Создание таблицы страниц
    secondChance sc;
    notFrequentlyUsed nfu;
    replacingAlgorithm* algorithmStrategy;
    if (algorithm == 1) {
        algorithmStrategy = &sc;
    }
    else if (algorithm == 2) {
        algorithmStrategy = &nfu;
    }
    pageTable table(PAGE_COUNT, MAX_COUNTER, *algorithmStrategy);

    while (true)
    {
        // Операция
        string operationStr;
        cin >> operationStr;
        short operation = atoi(operationStr.c_str());

        // Страница
        string pageStr;
        cin >> pageStr;
        short page = atoi(pageStr.c_str());

        if (operationStr == "" || pageStr == "")
            break;

        // Выполнить операцию
        switch (operation)
        {
        case 0:
            table.read(page);
            break;
        case 1:
            table.write(page);
            break;
        default:
            break;
        }

        // Вывод состояния
        cout << table.getState() << endl;
    }

    return 0;
}
