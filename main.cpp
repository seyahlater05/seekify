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
#include "Graph.h"
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Anim_GIF_Image.H>
#include <format>
#include "FL/Fl_Box.H"
#include "FL/Fl_Check_Button.H"
#include "FL/Fl_Text_Display.H"
using namespace std;
//TODO:
//replace multiline output w text display
//add timer for differnt algos
//implement actual functionality
//add selector for attributes

// Widget pointers
Fl_Select_Browser* menu_choice;
Fl_Text_Buffer* output_buffer;
Fl_Text_Display* output_box;
Fl_Text_Buffer* text_buffer;
Fl_Text_Display* text_box;
Fl_Round_Button* toggle_on;
Fl_Round_Button* toggle_off;
Fl_Anim_GIF_Image* title_gif = nullptr;
Fl_Anim_GIF_Image* head_gif = nullptr;
Fl_Box* title_box = nullptr;
Fl_Box* head_box = nullptr;
Fl_Check_Button* dance;
Fl_Check_Button* energy;
Fl_Check_Button* speech;
Fl_Check_Button* inst;
Fl_Check_Button* live;
Fl_Check_Button* valence;
Fl_Check_Button* acoust;
Fl_Check_Button* tempo;

bool gif_playing = true;
bool a = true;
bool random = true;
int rescount = 0;
vector<bool> metrics(8,0);

void check_callback(Fl_Widget*, void*) {
    metrics[0] = (dance->value() != 0);
    metrics[1] = (energy->value() != 0);
    metrics[2] = (speech->value() != 0);
    metrics[3] = (acoust->value() != 0);
    metrics[4] = (inst->value() != 0);
    metrics[5] = (live->value() != 0);
    metrics[6] = (valence->value() != 0);
    metrics[7] = (tempo->value() != 0);
}

// Custom timer callback for GIF animation
void gif_timer(void*) {
    if (gif_playing && title_gif && !title_gif->fail()) {
        // Advance animation frame WITHOUT automatic redraw
        title_gif->next();
        if(a) {
            head_gif->next();
        }
        a = !a;
        // Force redraw ONLY the GIF box area
        title_box->redraw();
        head_box->redraw();
        // Maintain animation speed (30 FPS)
        Fl::repeat_timeout(0.04, gif_timer);
    }
}
// Start button callback
void start_callback(Fl_Widget*, void*) {
    // Get selected menu item
    const int selection = menu_choice->value();
    const char* selected = menu_choice->text(selection);  // Get text directly from browser

    // Get toggle state
    const char* state = toggle_on->value() ? "Dijkstra's" : "Random Walk";  // Updated state labels
    // random = true is dijsktra
    // Update output
    std::string current = output_buffer->text() ? output_buffer->text() : "";
    current += "Selected: " + std::string(selected) + "\n";
    current += "Algorithm: " + std::string(state) + "\n";
    current += "Start button pressed!\n\n";
    output_buffer->text(current.c_str());
    ifstream file("dataset.csv");
    if (!random){
        Graph graph;
        graph.LoadGenreFromCSV(file, selected, metrics);
        vector<pair<Song*, float>> similars = graph.Dijsktra(selected);
        auto songSelected = similars.rbegin();
        string song = songSelected->first->track_name;
        current += "Songs similar to \"" + song + "\"\n\n";
        similars.pop_back();
        for (auto i : similars){
            current += "\"" + i.first->track_name + "\"\n";
        }
        current += "\n";
    }
    if (random)
    {
        Graph graph;
        graph.LoadGenreFromCSV(file, selected, metrics);
        vector<pair<Song *, float>> similars = graph.RWR(selected);
        auto songSelected = similars.rbegin();
        string song = songSelected->first->track_name;
        current += "Songs similar to \"" + song + "\"\n\n";
        similars.pop_back();
        for (auto i: similars)
        {
            current += "\"" + i.first->track_name + "\"\n";
        }
        current += "\n";
    }
    output_buffer->text(current.c_str());
}

// Toggle buttons callback
void toggle_callback(Fl_Widget*, void*) {
    // Determine active state
    const char* state = toggle_on->value() ? "Dijkstra's" : "Random Walk";
    random = !random;
    std::string current = output_buffer->text() ? output_buffer->text() : "";
    current += "Algorithm switched to " + std::string(state) + "\n\n";
    output_buffer->text(current.c_str());
}

void genre_callback(Fl_Widget* widget, void*) {
    Fl_Select_Browser* browser = static_cast<Fl_Select_Browser*>(widget);
    const int selection = browser->value();
    if (selection >= 1) {  // FLTK browser indices start at 1
        const char* selected = browser->text(selection);
        string genre = string(selected);
        vector<string> comments = {"{}? good choice, i guess", "i never was a fan of {}", "pshhh, {}? what are you, five?",
        "you know that one song that goes \"bum bum baaah nuh\"? classic", "booooo!", "not a big fan of {} personally", "ehhh {} is ok imo",
        "now {} is just bad taste", "ooh i like {}", "boooooooring", "*sighs smugly*", "need to come up with a new catchphrase"};

        string snarky_comment = vformat(comments[rescount], make_format_args(genre));
        rescount++;
        if(rescount >= comments.size()) {
            rescount = 0;
        }
        text_buffer->text(snarky_comment.c_str());  // Replace content
        // Optional: Append instead of replace
        // text_buffer->append("\nSelected: ");
        // text_buffer->append(selected);
    }
}


int main(int argc, char** argv) {
    ifstream file("dataset.csv");
    if (!file.is_open()) {
        cerr << "Could not open dataset.csv\n";
        return 1;
    }
    string genre;
    Graph graph;
    set<string> genre_list = graph.FindGenres(file);

    // Create main window
    int w = 700;
    int h = 900;
    const auto window = new Fl_Window(w, h, "Control Panel");
    window->begin();
    window->color(FL_LIGHT1);
    Fl::visual(FL_DOUBLE|FL_RGB);

    title_gif = new Fl_Anim_GIF_Image("superseekify.gif");
    head_gif = new Fl_Anim_GIF_Image("head.gif");
    if (title_gif && !title_gif->fail()) {
        title_box = new Fl_Box(360, -40, 350, 250);
        title_box->box(FL_NO_BOX);
        title_box->align(FL_ALIGN_CLIP);
        title_box->color(FL_LIGHT1);
        title_box->image(title_gif);
        title_gif->resize(1.75);
        title_gif->start();  // Required to initialize frames
        title_gif->stop();   // Immediately stop auto-play
        head_box = new Fl_Box(360, 640, 250, 300);
        head_box->box(FL_NO_BOX);
        head_box->align(FL_ALIGN_CLIP);
        head_box->color(FL_LIGHT1);
        head_box->image(head_gif);
        head_gif->resize(0.8);
        head_gif->start();  // Required to initialize frames
        head_gif->stop();   // Immediately stop auto-play


        // Start custom animation timer
        Fl::add_timeout(0.04, gif_timer, title_gif);  // 30 FPS
    }

    // Create scrollable dropdown (Fl_Select_Browser)
    menu_choice = new Fl_Select_Browser(20, 20, 200, 200, "Choose a Genre:");  // X, Y, W, H
    menu_choice->callback(genre_callback);
    menu_choice->has_scrollbar(Fl_Browser_::VERTICAL_ALWAYS);  // Force vertical scrollbar
    menu_choice->textsize(12);  // Set font size
    menu_choice->align(FL_ALIGN_TOP_LEFT);  // Label alignment

    // Populate with genres
    for(const string& s : genre_list) {
        menu_choice->add(s.c_str());
    }
    if(!genre_list.empty()) {
        menu_choice->select(1);
        menu_choice->topline(1);
    }
    // Set visible items (calculated based on height and text size)
    const int item_height = menu_choice->textsize() + 8;  // Text size + padding
    const int visible_items = 200 / item_height;  // 200 is the browser height
    menu_choice->set_visible_focus();  // Enable keyboard navigation

    // Create start button
    auto* start_btn = new Fl_Button(20, 775, 80, 30, "Start");
    start_btn->callback(start_callback);

    // Create algorithm toggle buttons
    Fl_Box* alg_box = new Fl_Box(230, 25, 120, 80, "Algorithm:");
    alg_box->align(FL_ALIGN_TOP_LEFT);
    alg_box->box(FL_UP_BOX);
    toggle_on = new Fl_Round_Button(235, 30, 120, 30, "Dijkstra's");
    toggle_off = new Fl_Round_Button(235, 60, 120, 30, "Random Walk");
    toggle_on->type(FL_RADIO_BUTTON);
    toggle_off->type(FL_RADIO_BUTTON);
    toggle_off->value(1);  // Default to Random Walk
    toggle_on->callback(toggle_callback);
    toggle_off->callback(toggle_callback);

    // Create output text box
    output_box = new Fl_Text_Display(20, 250, 300, 500);  // Adjusted position
    output_buffer = new Fl_Text_Buffer();
    output_box->buffer(output_buffer);
    output_box->textfont(FL_COURIER);
    output_box->textsize(12);
    output_buffer->text("System Ready\n\n");

    text_box = new Fl_Text_Display(360, 625, 250, 50);
    text_buffer = new Fl_Text_Buffer();
    text_box->buffer(text_buffer);

    Fl_Box* check_box = new Fl_Box(360, 280, 180, 205, "Choose songs with similar:");
    check_box->box(FL_UP_BOX);
    check_box->align(FL_ALIGN_TOP_LEFT);
    dance = new Fl_Check_Button(380, 300, 20,20, "Danceability");
    energy = new Fl_Check_Button(380, 320, 20,20, "Energy");
    speech = new Fl_Check_Button(380, 340, 20,20, "Speechiness");
    acoust = new Fl_Check_Button(380, 360, 20,20, "Acousticness");
    inst = new Fl_Check_Button(380, 380, 20,20, "Instrumentalness");
    live = new Fl_Check_Button(380, 400, 20,20, "Liveliness");
    valence = new Fl_Check_Button(380, 420, 20,20, "Valence");
    tempo = new Fl_Check_Button(380, 440, 20,20, "Tempo");
    dance->callback(check_callback);
    energy->callback(check_callback);
    speech->callback(check_callback);
    acoust->callback(check_callback);
    inst->callback(check_callback);
    live->callback(check_callback);
    valence->callback(check_callback);
    tempo->callback(check_callback);


    window->end();
    window->show();

    return Fl::run();
    //
    // cout << "Enter a genre: ";
    // getline(cin, genre);
    //
    //
    //
    // Graph graph;
    // graph.LoadGenreFromCSV(file, genre); // Only loads songs from that genre
    //
    // const auto& songs = graph.GetSongsByGenre(genre);
    // const auto& matrix = graph.GetAdjacencyMatrix(genre);
    //
    // if (songs.size() < 10) {
    //     cout << "Not enough songs in this genre to find 10 closest.\n";
    //     return 1;
    // }
    //
    // // Pick a random song
    // random_device rd;
    // mt19937 gen(rd());
    // uniform_int_distribution<> dist(0, songs.size() - 1);
    // int startIndex = dist(gen);
    //
    // cout << "\nStarting song: " << songs[startIndex].track_name << " by " << songs[startIndex].artists << "\n\n";
    //
    // // Set to track unique recommendations by name+artist
    // set<string> seen;
    // seen.insert(songs[startIndex].track_name + songs[startIndex].artists);
    //
    // vector<pair<float, int>> distances;
    // for (size_t i = 0; i < matrix[startIndex].size(); ++i) {
    //     if (i != static_cast<size_t>(startIndex)) {
    //         distances.emplace_back(matrix[startIndex][i], i);
    //     }
    // }
    //
    // sort(distances.begin(), distances.end());
    //
    // cout << "9 closest songs:\n";
    // int count = 0;
    // for (const auto& [dist, index] : distances) {
    //     string key = songs[index].track_name + songs[index].artists;
    //     if (seen.count(key)) continue;
    //
    //     seen.insert(key);
    //     cout << " - " << songs[index].track_name << " by " << songs[index].artists << "\n";
    //     if (++count == 9) break;
    // }
    //
    // return 0;
}