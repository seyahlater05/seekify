#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <set>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Anim_GIF_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>

#include "Graph.h"

using namespace std;

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
size_t rescount = 0;
vector<bool> metrics(8, false);

void check_callback(Fl_Widget*, void*) {
    metrics[0] = dance->value();
    metrics[1] = energy->value();
    metrics[2] = speech->value();
    metrics[3] = acoust->value();
    metrics[4] = inst->value();
    metrics[5] = live->value();
    metrics[6] = valence->value();
    metrics[7] = tempo->value();
}

void gif_timer(void*) {
    if (gif_playing && title_gif && !title_gif->fail()) {
        title_gif->next();
        if (a) head_gif->next();
        a = !a;
        title_box->redraw();
        head_box->redraw();
        Fl::repeat_timeout(0.06, gif_timer);
    }
}

void start_callback(Fl_Widget*, void*) {
    int selection = menu_choice->value();
    const char* selected = menu_choice->text(selection);
    const char* state = toggle_on->value() ? "Dijkstra's" : "Random Walk";

    string current = output_buffer->text() ? output_buffer->text() : "";
    current += "Selected: " + string(selected) + "\n";
    current += "Algorithm: " + string(state) + "\nStart button pressed!\n\n";

    ifstream file("dataset.csv");
    Graph graph;
    graph.LoadGenreFromCSV(file, selected, metrics);

    vector<pair<Song*, float>> similars = random ? graph.RWR(selected) : graph.Dijsktra(selected);
    auto songSelected = similars.rbegin();
    current += "Songs similar to \"" + songSelected->first->track_name + "\" by " + songSelected->first->artists + "\n\n";
    similars.pop_back();
    for (auto key : similars | views::keys) {
        current += "\"" + key->track_name + "\" by " + key->artists + "\n";
    }
    current += "\n";
    output_buffer->text(current.c_str());
}

void toggle_callback(Fl_Widget*, void*) {
    random = !random;
    const char* state = toggle_on->value() ? "Dijkstra's" : "Random Walk";
    string current = output_buffer->text() ? output_buffer->text() : "";
    current += "Algorithm switched to " + string(state) + "\n\n";
    output_buffer->text(current.c_str());
}

void genre_callback(Fl_Widget* widget, void*) {
    const auto* browser = dynamic_cast<Fl_Select_Browser*>(widget);
    if (const int selection = browser->value(); selection >= 1) {
        const char* selected = browser->text(selection);
        string genre = selected;
        const vector<string> comments = {
            "{}? good choice, i guess", "i never was a fan of {}", "pshhh, {}? what are you, five?",
            "you know that one song that goes \n\"bum bum baaah nuh\"? classic!", "booooo!",
            "not a big fan of {} personally", "ehhh {} is ok imo", "now {} is just bad taste",
            "ooh i like {}", "boooooooring", "*sighs smugly*", "need to come up with a new\ncatchphrase"
        };
        const string snarky_comment = vformat(comments[rescount], make_format_args(genre));
        rescount = (rescount + 1) % comments.size();
        text_buffer->text(snarky_comment.c_str());
    }
}

int main() {
    ifstream file("dataset.csv");
    if (!file.is_open()) {
        cerr << "Could not open dataset.csv\n";
        return 1;
    }
    const set<string> genre_list = Graph::FindGenres(file);

    constexpr int w = 700, h = 900;
    auto* window = new Fl_Window(w, h, "Seekify");
    window->begin();
    window->color(FL_LIGHT1);
    Fl::visual(FL_DOUBLE | FL_RGB);

    title_gif = new Fl_Anim_GIF_Image("superseekify.gif");
    head_gif = new Fl_Anim_GIF_Image("head.gif");
    if (title_gif && !title_gif->fail()) {
        title_box = new Fl_Box(350, -40, 350, 250);
        title_box->box(FL_NO_BOX);
        title_box->align(FL_ALIGN_CLIP);
        title_box->color(FL_LIGHT1);
        title_box->image(title_gif);
        title_gif->resize(1.75);
        title_gif->start();
        title_gif->stop();

        head_box = new Fl_Box(360, 650, 250, 300);
        head_box->box(FL_NO_BOX);
        head_box->align(FL_ALIGN_CLIP);
        head_box->color(FL_LIGHT1);
        head_box->image(head_gif);
        head_gif->resize(0.8);
        head_gif->start();
        head_gif->stop();

        Fl::add_timeout(0.06, gif_timer, title_gif);
    }

    menu_choice = new Fl_Select_Browser(20, 20, 200, 200, "Choose a Genre:");
    menu_choice->callback(genre_callback);
    menu_choice->has_scrollbar(Fl_Browser_::VERTICAL_ALWAYS);
    menu_choice->textsize(12);
    menu_choice->align(FL_ALIGN_TOP_LEFT);
    for (const auto& s : genre_list) menu_choice->add(s.c_str());
    if (!genre_list.empty()) menu_choice->select(1);

    auto* start_btn = new Fl_Button(20, 775, 80, 30, "Start");
    start_btn->callback(start_callback);

    auto* alg_box = new Fl_Box(230, 25, 120, 80, "Algorithm:");
    alg_box->align(FL_ALIGN_TOP_LEFT);
    alg_box->box(FL_UP_BOX);
    toggle_on = new Fl_Round_Button(235, 30, 120, 30, "Dijkstra's");
    toggle_off = new Fl_Round_Button(235, 60, 120, 30, "Random Walk");
    toggle_on->type(FL_RADIO_BUTTON);
    toggle_off->type(FL_RADIO_BUTTON);
    toggle_off->value(1);
    toggle_on->callback(toggle_callback);
    toggle_off->callback(toggle_callback);

    output_box = new Fl_Text_Display(20, 250, 300, 500, "Recommendations:");
    output_box->align(FL_ALIGN_TOP_LEFT);
    output_buffer = new Fl_Text_Buffer();
    output_box->buffer(output_buffer);
    output_box->textfont(FL_COURIER);
    output_box->textsize(12);
    output_buffer->text("System Ready\n\n");

    auto* text_up = new Fl_Box(350, 615, 270, 70, "Official Music Opinions:");
    text_up->align(FL_ALIGN_TOP_LEFT);
    text_up->box(FL_UP_BOX);
    text_box = new Fl_Text_Display(360, 625, 250, 50);
    text_buffer = new Fl_Text_Buffer();
    text_box->buffer(text_buffer);

    auto* check_box = new Fl_Box(360, 280, 180, 205, "Choose songs with similar:");
    check_box->box(FL_UP_BOX);
    check_box->align(FL_ALIGN_TOP_LEFT);

    dance = new Fl_Check_Button(380, 300, 20, 20, "Danceability");
    energy = new Fl_Check_Button(380, 320, 20, 20, "Energy");
    speech = new Fl_Check_Button(380, 340, 20, 20, "Speechiness");
    acoust = new Fl_Check_Button(380, 360, 20, 20, "Acousticness");
    inst = new Fl_Check_Button(380, 380, 20, 20, "Instrumentalness");
    live = new Fl_Check_Button(380, 400, 20, 20, "Liveliness");
    valence = new Fl_Check_Button(380, 420, 20, 20, "Valence");
    tempo = new Fl_Check_Button(380, 440, 20, 20, "Tempo");

    for (auto* btn : {dance, energy, speech, acoust, inst, live, valence, tempo}) {
        btn->callback(check_callback);
    }

    window->end();
    window->show();
    return Fl::run();
}