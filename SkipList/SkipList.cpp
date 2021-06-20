// SkipList.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "SkipList.h"
#define FILE_PATH "./store/dumpFile"

int main()
{
    SkipList<std::string, std::string>skipList(6);

    skipList.insert_element("1", "a");
    skipList.insert_element("3", "b");
    skipList.insert_element("7", "c");
    skipList.insert_element("8", "sun");
    skipList.insert_element("9", "xiu");
    skipList.insert_element("99", "yang");
    skipList.insert_element("99", "yang");
    skipList.insert_element("99", "yang");
    skipList.insert_element("99", "yanuuuuuuuuuuuuuuuuuug");

    std::cout << "skipList size:" << skipList.size() << std::endl;
    skipList.display_list();

    //skipList.dump_file();

    ////// skipList.load_file();

    skipList.search_element("9");
    skipList.search_element("18");

    skipList.get_prev_element("9");
    skipList.get_tail();


    //skipList.display_list();

    //skipList.delete_element("3");
    skipList.delete_element("99");
    skipList.get_tail();

    //std::cout << "skipList size:" << skipList.size() << std::endl;

    //skipList.display_list();
}

