#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <set>
#include "Graph.h"
#include "FL/Fl.h"
#include "FL/Fl_Window.H"
#include "FL/Fl.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Input.H"
#include "FL/Fl_Output.H"
#include <string>
#include <algorithm>

// Callback function to reverse the input text
void reverse_input_cb(Fl_Widget* w, void* data) {
    auto* input = dynamic_cast<Fl_Input*>(w);
    auto* output = static_cast<Fl_Output*>(data);

    std::string text = input->value();
    ranges::reverse(text);
    output->value(text.c_str());
}
using namespace std;

int main(int argc, char** argv) {
    auto window = new Fl_Window(400, 150, "Reverse Text");

    // Input field
    auto* input = new Fl_Input(100, 30, 200, 30, "Input:");
    input->when(FL_WHEN_ENTER_KEY_ALWAYS); // Trigger callback on Enter key

    // Output field
    auto* output = new Fl_Output(100, 80, 200, 30, "Reversed:");

    // Set the callback function
    input->callback(reverse_input_cb, output);

    window->end();
    window->show(argc, argv);

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

    sort(distances.begin(), distances.end());

    cout << "9 closest songs:\n";
    int count = 0;
    for (const auto& [dist, index] : distances) {
        string key = songs[index].track_name + songs[index].artists;
        if (seen.contains(key)) continue;

        seen.insert(key);
        cout << " - " << songs[index].track_name << " by " << songs[index].artists << "\n";
        if (++count == 9) break;
    }

    return 0;
}