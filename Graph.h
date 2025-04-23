#pragma once

#include <set>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct Song {
    int serial, popularity, track_length, key, time_signature;
    string track_id, artists, album, track_name, genre;
    bool potty_words, mode;
    float danceability, energy, loudness, speechiness, acousticness, liveness, valence, tempo;
    double instrumentalness;
};

class Graph {
    unordered_map<string, vector<Song>> genreMap;
    unordered_map<string, vector<vector<float>>> adjacencyMatrices;

    static vector<float> GetFeatureVector(const Song& song);

    static float EuclideanDistance(const vector<float>& a, const vector<float>& b, vector<bool> &metrics);

public:
    void LoadFromCSV(ifstream& file, vector<bool> &metrics);
    void LoadGenreFromCSV(std::ifstream& file, const std::string& target_genre, vector<bool> &metrics);
    const vector<vector<float>>& GetAdjacencyMatrix(const string& genre) const;
    const vector<Song>& GetSongsByGenre(const string& genre) const;
    vector<string> GetAllGenres() const;

    static set<string> FindGenres(ifstream& file);

    vector<pair<Song*, float>> Dijsktra(const string &genre, int k = 15);
    vector<pair<Song*, float>> RWR(const string &genre, int k =15);
};