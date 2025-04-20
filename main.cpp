#include <algorithm>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <random>
#include <ranges>

#include "Graph.h"
using namespace std;

// Widget pointers
Fl_Choice* menu_choice;
Fl_Multiline_Output* output_box;
Fl_Round_Button* toggle_on;
Fl_Round_Button* toggle_off;

// Start button callback
void start_callback(Fl_Widget*, void*) {
    // Get selected menu item
    const int selection = menu_choice->value();
    const Fl_Menu_Item* menu = menu_choice->menu();
    const char* selected = menu[selection].label();

    // Get toggle state
    auto state = "OFF";
    if (toggle_on->value()) state = "ON";

    // Update output
    std::string current = output_box->value() ? output_box->value() : "";
    current += "Selected: " + std::string(selected) + "\n";
    current += "Toggle state: " + std::string(state) + "\n";
    current += "Start button pressed!\n\n";
    output_box->value(current.c_str());
}

// Toggle buttons callback
void toggle_callback(Fl_Widget*, void*) {
    // Determine active state
    auto state = "OFF";
    if (toggle_on->value()) state = "ON";

    std::string current = output_box->value() ? output_box->value() : "";
    current += "Toggle switched to " + std::string(state) + "\n\n";
    output_box->value(current.c_str());
}



int main(int argc, char** argv) {
    // Create main window
    auto window = new Fl_Window(400, 300, "Control Panel");
    window->color(FL_WHITE);

    // Create drop-down menu
    menu_choice = new Fl_Choice(20, 20, 150, 30, "Options:");
    menu_choice->add("Option 1");
    menu_choice->add("Option 2");
    menu_choice->add("Option 3");
    menu_choice->value(0);  // Set default selection

    // Create start button
    auto start_btn = new Fl_Button(200, 20, 80, 30, "Start");
    start_btn->callback(start_callback);

    // Create toggle buttons (radio group)
    toggle_on = new Fl_Round_Button(300, 20, 80, 30, "ON");
    toggle_off = new Fl_Round_Button(300, 60, 80, 30, "OFF");

    // Configure as radio buttons
    toggle_on->type(FL_RADIO_BUTTON);
    toggle_off->type(FL_RADIO_BUTTON);
    toggle_off->value(1);  // Default to OFF position

    // Set common callback
    toggle_on->callback(toggle_callback);
    toggle_off->callback(toggle_callback);

    // Create output text box
    output_box = new Fl_Multiline_Output(20, 100, 360, 170);
    output_box->textfont(FL_COURIER);
    output_box->textsize(12);
    output_box->value("System Ready\n\n");  // Initial message

    window->end();
    window->show();
    return Fl::run();

    string genre;
    cout << "Enter a genre: ";
    getline(cin, genre);

    ifstream file("dataset.csv");
    if (!file) {
        cerr << "Could not open dataset.csv\n";
        return 1;
    }

    Graph graph;
    graph.LoadGenreFromCSV(file, genre); // Only loads songs from that genre

    const auto& songs = graph.GetSongsByGenre(genre);
    const auto& matrix = graph.GetAdjacencyMatrix(genre);

    if (songs.size() < 10) {
        cout << "Not enough songs in this genre to find 10 closest.\n";
        return 1;
    }

    // Pick a random song
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, songs.size() - 1);
    int startIndex = dist(gen);

    cout << "\nStarting song: " << songs[startIndex].track_name << " by " << songs[startIndex].artists << "\n\n";

    // Set to track unique recommendations by name+artist
    set<string> seen;
    seen.insert(songs[startIndex].track_name + songs[startIndex].artists);

    vector<pair<float, int>> distances;
    for (size_t i = 0; i < matrix[startIndex].size(); ++i) {
        if (i != static_cast<size_t>(startIndex)) {
            distances.emplace_back(matrix[startIndex][i], i);
        }
    }

    ranges::sort(distances);

    cout << "9 closest songs:\n";
    int count = 0;
    for (int index : distances | std::views::values) {
        string key = songs[index].track_name + songs[index].artists;
        if (seen.contains(key)) continue;

        seen.insert(key);
        cout << " - " << songs[index].track_name << " by " << songs[index].artists << "\n";
        if (++count == 9) break;
    }

    return 0;
}