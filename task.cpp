#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std;

bool in_alphabet(string s)
{
    const string alphabet = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    for (int i = 0; i < alphabet.size(); i+=2)
    {
        string alpha = alphabet.substr(i,2);
        if (alpha == s) return 1;
    }
    return 0;
}

class List
{
    struct Item
    {
        string name;
        float X;
        float Y;
        string type;
        double time;

        void print(ofstream &f) const
        {
            f << name << " " << X << " " << Y << " " << type << " " << to_string(time) << endl;
        }

        float distance() const
        {
            return sqrt(X*X + Y*Y);
        }

        bool in_time(double from, double to)
        {
            return (time > from) && (time < to);
        }

        static bool compare_by_distance(const Item &first, const Item &second)
        {
            return first.distance() < second.distance();
        }

        static bool compare_by_name(const Item &first, const Item &second)
        {
            return first.name < second.name;
        }

        static bool compare_by_time(const Item &first, const Item &second)
        {
            return first.time < second.time;
        }
    };

    vector<Item> data;

public:
    List(ifstream &f)
    {
        string s;
        f >> s;
        while (!f.eof())
        {
            Item it;
            it.name = s;
            f >> s;
            it.X = stof(s);
            f >> s;
            it.Y = stof(s);
            f >> s;
            it.type = s;
            f >> s;
            it.time = stod(s);
            f >> s;

            data.push_back(it);
        }
    }

    void write_to_file(string path)
    {
        ofstream f(path);
        for (Item i : data)
        {
            i.print(f);
        }
    }

    void add_to_list()
    {
        Item it;
        while (true)
        {    
            try
            {
                string s;
                cout << "please enter the item info\n";
                cout << "name: ";
                cin >> it.name;
                cout << "X coordinate: ";
                cin >> s;
                it.X = stof(s);
                cout << "Y coordinate: ";
                cin >> s;
                it.Y = stof(s);
                cout << "type: ";
                cin >> it.type;
                cout << "time of creation: ";
                cin >> s;
                it.time = stod(s);
            }
            catch(...)
            {
                cout << "please follow the scheme: string float float string double\ntry again\n";
                continue;
            }
            break;
        }
        data.push_back(it);
        cout << "the item was added\n";
    }

    void print_groups(vector<pair<string, vector<Item>>> groups, ofstream &f)
    {
        for (auto group : groups)
        {
            if (group.second.size() == 0) continue;     //we don't print empty groups
            f << group.first << ":\n";
            for (Item it : group.second)
            {
                it.print(f);
            }
            f << endl;
        }
    }

    void grouping_by_distance()
    {
        vector<pair<string, vector<Item>>> groups(4);
        groups[0].first = "< 100";
        groups[1].first = "100 < 1000";
        groups[2].first = "1000 < 10000";
        groups[3].first = "> 10000";
        for (Item it : data)
        {
            int d = it.distance();
            if (d < 100)
            {
                groups[0].second.push_back(it);
                continue;
            }
            if (d < 1000)
            {
                groups[1].second.push_back(it);
                continue;
            }
            if (d < 10000)
            {
                groups[2].second.push_back(it);
                continue;
            }
            groups[3].second.push_back(it);
        }
        for (int i = 0; i < 4; i++)
        {
            sort(groups[i].second.begin(), groups[i].second.end(), Item::compare_by_distance);
        }
        
        string grouping = "by_distance"; 
        ofstream f(grouping);
        print_groups(groups, f);
        cout << "saved in " << grouping << endl;
    }

    void grouping_by_name()
    {
        vector<pair<string, vector<Item>>> groups;
        pair<string, vector<Item>> p;
        p.first = "#";
        groups.push_back(p);

        for (Item it : data)
        {
            string alpha = it.name.substr(0,2);
            if (!in_alphabet(alpha))
            {
                groups[0].second.push_back(it);      //put in "#"-group
                continue;
            }
            bool flag = true;   //check if there is already needed group
            for (auto& group : groups)
            {
                if (group.first == alpha)
                {
                    group.second.push_back(it);
                    flag = false;
                }
                if (!flag) break;
            }
            // have to make new group
            if (flag)
            {
                pair<string, vector<Item>> p;
                p.first = alpha;
                p.second.push_back(it);
                groups.push_back(p);
            }
        }
        for (auto &group : groups)
        {
            sort(group.second.begin(), group.second.end(), Item::compare_by_name);
        }

        string grouping = "by_name"; 
        ofstream f(grouping);
        print_groups(groups, f);
        cout << "saved in " << grouping << endl;
    }

    void grouping_by_time()
    {
        vector<pair<string, vector<Item>>> groups(7);
        groups[0].first = "В будещем";
        groups[1].first = "Сегодня";
        groups[2].first = "Вчера";
        groups[3].first = "На этой неделе";
        groups[4].first = "В этом месяце";
        groups[5].first = "В этом году";
        groups[6].first = "Ранее";
        
        time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
        auto tm = localtime(&now);
        time_t today = now - tm->tm_hour*60*60 - tm->tm_min*60 - tm->tm_sec;    //midnight
        time_t yesterday = today - 24*60*60;    //midnight of yesterday
        auto wday = (tm->tm_wday + 6)%7;        //to start a week from monday, not sunday
        time_t week = today - wday*24*60*60;    //start of the week
        time_t month = today - (tm->tm_mday-1)*24*60*60;     //start of the month
        time_t year = today - tm->tm_yday*24*60*60;          //start of the year

        for (Item it : data)
        {
            if (it.time > now)
            {
                groups[0].second.push_back(it);
                continue;
            }
            if (it.in_time(today, now))
            {
                groups[1].second.push_back(it);
                continue;
            }
            if (it.in_time(yesterday, today))
            {
                groups[2].second.push_back(it);
                continue;
            }
            if (it.in_time(week, now))
            {
                groups[3].second.push_back(it);
                continue;
            }
            if (it.in_time(month, now))
            {
                groups[4].second.push_back(it);
                continue;
            }
            if (it.in_time(year, now))
            {
                groups[5].second.push_back(it);
                continue;
            }
            groups[6].second.push_back(it);
        }

        for (auto &group : groups)
        {
            sort(group.second.begin(), group.second.end(), Item::compare_by_time);
        }
        
        string grouping = "by_time"; 
        ofstream f(grouping);
        print_groups(groups, f);
        cout << "saved in " << grouping << endl;
    }

    void grouping_by_type(int N)
    {
        vector<pair<string, vector<Item>>> groups(1);
        groups[0].first = "Разное";

        for (Item it : data)
        {
            bool flag = true;   //check if there is already needed group
            for (auto& group : groups)
            {
                if (group.first == it.type)
                {
                    group.second.push_back(it);
                    flag = false;
                }
                if (!flag) break;
            }
            // have to make new group
            if (flag)
            {
                pair<string, vector<Item>> p;
                p.first = it.type;
                p.second.push_back(it);
                groups.push_back(p);
            }
        }
        //  if the group is too small, put the content in common group
        for (auto group = groups.begin() + 1 ; group != groups.end(); )
        {
            if (int((*group).second.size()) < N)
            {
                for (auto elem : (*group).second)
                {
                    groups[0].second.push_back(elem);
                }
                groups.erase(group);
            }
            else group++;
        }
        for (auto &group : groups)
        {
            sort(group.second.begin(), group.second.end(), Item::compare_by_name);
        }
        
        string grouping = "by_type"; 
        ofstream f(grouping);
        print_groups(groups, f);
        cout << "saved in " << grouping << endl;
    }

};

void show_info()
{
    cout << "info\na\tadd an item to the list\n"<<
                  "s\tsave existing list in \"output\"\n"
                  "g\tgrouping the list\n"<<
                  "q\tquit\n"<<
                  "h\tshow this info\n";
}

struct Menu
{
    static char get_char_from_list(string allowed, string prompt, string on_error)
    {
        cout << prompt;

        while (true)
        {
            string s;
            cin >> s;
            
            if (s.length() == 1 && allowed.find(s[0]) != string::npos)
            {
                return s[0];
            }
            else
            {
                cout << on_error;
            }
        }
    }
    
    static int get_valid_int(string prompt, string on_error)
    {
        cout << prompt;
        int n;
        
        while (true)
        {
            string s;
            cin >> s;
            
            try
            {
                n = stoi(s);
                break;
            }
            catch (...)
            {
                cout << on_error;
            }
        }
        
        return n;
    }
};

void group_menu(List &list)
{
    cout << "you want to group by:\n"<<
                "1\tdistance\n"<<
                "2\tname\n"<<
                "3\ttime of creation\n"<<
                "4\ttype\n"<<
                "b\tback to main menu\n"<<
                "q\tquit\n";
    
    char choice = Menu::get_char_from_list("1234bq", "your choice: ", "enter a number 1-4: ");
    
    switch (choice)
    {
        case '1':
            list.grouping_by_distance();
            break;
        case '2':
            list.grouping_by_name();
            break;
        case '3':
            list.grouping_by_time();
            break;
        case '4':
            list.grouping_by_type(Menu::get_valid_int("minimum number of items in every group: ", "enter an integer: "));
            break;
        case 'b':
            return;
        case 'q':
            exit(0);
    }
}

int main()
{
    string out_path = "output";
    ifstream f;
    
    while(1)
    {
        string in_path;
        cout << "enter file name: ";
        cin >> in_path;

        if (in_path == "q") return 0;

        f.open(in_path);
        if (!f.is_open())
        {
            cout << "could not open: \"" << in_path << "\"\ntry again\n";
        }
        else break;
    }

    List list(f);

    while(1)
    {
        show_info();

        char command = Menu::get_char_from_list("asgqh", "what to do?  ", "enter one of the following symbols: a s g q h: ");
        
        switch (command)
        {
            case 'q': return 0;
            case 'h':
                show_info();
                break;
            case 'a': 
                list.add_to_list();
                break;
            case 's':
                list.write_to_file(out_path);
                cout << "saved in " << out_path << endl;
                break;
            case 'g':
                group_menu(list);
                break;
        }
    }

    return 0;
}
