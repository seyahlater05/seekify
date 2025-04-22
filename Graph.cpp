//
// Created by sam51 on 4/19/2025.
//

#include "Graph.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ranges>
#include <set>
#include <queue>
#include <random>
#include <unordered_set>

vector<float> Graph::GetFeatureVector(const Song& song) {
    return {
        song.danceability, song.energy, song.speechiness, song.acousticness,
        static_cast<float>(song.instrumentalness), song.liveness,
        song.valence, song.tempo
    };
}

float Graph::EuclideanDistance(const vector<float>& a, const vector<float>& b, vector<bool>& metrics) {
    float sum = 0.0f;
    const bool no_selection = ranges::all_of(metrics, [](const bool selected) { return !selected; });

    for (size_t i = 0; i < a.size(); ++i) {
        if (metrics[i] || no_selection) {
            const float diff = a[i] - b[i];
            sum += diff * diff;
        }
    }
    return sqrt(sum);
}

const vector<vector<float>>& Graph::GetAdjacencyMatrix(const string& genre) const {
    return adjacencyMatrices.at(genre);
}

const vector<Song>& Graph::GetSongsByGenre(const string& genre) const {
    return genreMap.at(genre);
}

vector<string> Graph::GetAllGenres() const {
    vector<string> genres;
    for (const auto& key : genreMap | std::views::keys) {
        genres.push_back(key);
    }
    return genres;
}


void Graph::LoadFromCSV(ifstream& file, vector<bool>& metrics) {
    string header;
    getline(file, header);

    while (file && file.peek() != EOF) {
        string temp, line;
        getline(file, temp, ',');
        getline(file, line);

        vector<string> fields;
        string field;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); ++i) {
            if (char c = line[i]; c == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i; // skip the escaped quote
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        fields.push_back(field);

        if (fields.size() < 20) continue;

        Song song;
        song.track_id = fields[0];
        song.artists = fields[1];
        song.album = fields[2];
        song.track_name = fields[3];
        song.popularity = stoi(fields[4]);
        song.track_length = stoi(fields[5]);

        string potty = fields[6];
        ranges::transform(potty, potty.begin(), ::tolower);
        istringstream(potty) >> boolalpha >> song.potty_words;

        song.danceability     = stof(fields[7]);
        song.energy           = stof(fields[8]);
        song.key              = stoi(fields[9]);
        song.loudness         = stof(fields[10]);
        song.mode             = stoi(fields[11]);
        song.speechiness      = stof(fields[12]);
        song.acousticness     = stof(fields[13]);
        song.instrumentalness = stod(fields[14]);
        song.liveness         = stof(fields[15]);
        song.valence          = stof(fields[16]);
        song.tempo            = stof(fields[17]);
        song.time_signature   = stoi(fields[18]);
        song.genre            = fields[19];

        string& genre = song.genre;
        auto& songs = genreMap[genre];
        size_t n = songs.size();

        vector<float> newRow(n + 1, 0.0f);
        for (size_t i = 0; i < n; ++i) {
            float dist = EuclideanDistance(GetFeatureVector(song), GetFeatureVector(songs[i]), metrics);
            adjacencyMatrices[genre][i].push_back(dist);
            newRow[i] = dist;
        }

        adjacencyMatrices[genre].push_back(newRow);
        songs.push_back(song);
    }
}

void Graph::LoadGenreFromCSV(ifstream& file, const string& target_genre, vector<bool>& metrics) {
    string header;
    getline(file, header);

    string line;
    while (getline(file, line)) {
        size_t firstComma = line.find(',');
        if (firstComma == string::npos) continue;

        string rest = line.substr(firstComma + 1);
        vector<string> fields;
        string current;
        bool inQuotes = false;

        for (size_t i = 0; i < rest.size(); ++i) {
            if (const char c = rest[i]; c == '"') {
                if (inQuotes && i + 1 < rest.size() && rest[i + 1] == '"') {
                    current += '"';
                    ++i;  // Skip the next character, which is the escaped quote
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        fields.push_back(current);

        if (fields.size() < 20 || fields[19] != target_genre) continue;

        Song song;
        song.track_id         = fields[0];
        song.artists          = fields[1];
        song.album            = fields[2];
        song.track_name       = fields[3];
        song.popularity       = stoi(fields[4]);
        song.track_length     = stoi(fields[5]);
        string potty          = fields[6];
        ranges::transform(potty, potty.begin(), ::tolower);
        istringstream(potty) >> boolalpha >> song.potty_words;

        song.danceability     = stof(fields[7]);
        song.energy           = stof(fields[8]);
        song.key              = stoi(fields[9]);
        song.loudness         = stof(fields[10]);
        song.mode             = stoi(fields[11]);
        song.speechiness      = stof(fields[12]);
        song.acousticness     = stof(fields[13]);
        song.instrumentalness = stod(fields[14]);
        song.liveness         = stof(fields[15]);
        song.valence          = stof(fields[16]);
        song.tempo            = stof(fields[17]);
        song.time_signature   = stoi(fields[18]);
        song.genre            = target_genre;

        genreMap[target_genre].push_back(song);
    }

    auto& songs = genreMap[target_genre];
    size_t n = songs.size();

    vector<vector<float>> matrix(n, vector<float>(n, 0.0f));
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            matrix[i][j] = i == j ? 1e9f : EuclideanDistance(GetFeatureVector(songs[i]), GetFeatureVector(songs[j]), metrics);

    adjacencyMatrices[target_genre] = std::move(matrix);
}

set<string> Graph::FindGenres(ifstream& file) {
    string temp;
    getline(file, temp);
    set<string> genres;

    while (file && file.peek() != EOF) {
        getline(file, temp, ',');
        string line;
        getline(file, line);

        vector<string> result;
        string current;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); ++i) {
            if (const char c = line[i]; c == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';  // Add the escaped quote to the current string
                    ++i;  // Skip the next character, which is the second quote in `""`
                } else {
                    inQuotes = !inQuotes;  // Toggle the quote state
                }
            } else if (c == ',' && !inQuotes) {
                result.push_back(current);  // Add the current field to the result
                current.clear();  // Clear the current field
            } else {
                current += c;  // Add the character to the current field
            }
        }
        result.push_back(current);
        if (result.size() < 20) continue;
        genres.insert(result[19]);
    }

    return genres;
}

vector<pair<Song*, float>> Graph::Dijsktra(const string& genre, const int k) {
    mt19937 gen(random_device{}());
    uniform_int_distribution<> distr(0, 999);
    int startIndex = distr(gen);

    const auto& similarityMatrix = adjacencyMatrices.at(genre);
    const size_t n = similarityMatrix.size();

    unordered_set<string> seenSongs;
    vector<float> dist(n, numeric_limits<float>::infinity());
    vector<bool> visited(n, false);
    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;
    dist[startIndex] = 0.0f;
    pq.emplace(0.0f, startIndex);

    vector<pair<int, float>> result;
    while (!pq.empty() && result.size() < k) {
        auto [cost, u] = pq.top();
        pq.pop();
        if (visited[u]) continue;
        visited[u] = true;

        if (u != startIndex) {
            const Song* song = &genreMap[genre][u];
            if (string songKey = song->track_name + "|" + song->artists; seenSongs.insert(songKey).second) {
                result.emplace_back(u, cost);
            }
        }

        for (int v = 0; v < n; ++v) {
            if (!visited[v] && similarityMatrix[u][v] > 0.0f) {
                const float sim = similarityMatrix[u][v];
                if (float newCost = dist[u] + 1.0f / sim; newCost < dist[v]) {
                    dist[v] = newCost;
                    pq.emplace(newCost, v);
                }
            }
        }
    }

    ranges::sort(result, [&](auto& a, auto& b) {
        return similarityMatrix[startIndex][a.first] > similarityMatrix[startIndex][b.first];
    });

    vector<pair<Song*, float>> similar;
    for (auto& [index, score] : result) {
        similar.emplace_back(&genreMap[genre][index], score);
    }

    Song* startSong = &genreMap[genre][startIndex];
    if (const string startKey = startSong->track_name + "|" + startSong->artists; seenSongs.insert(startKey).second) {
        similar.emplace_back(startSong, 0.0f);
    }

    return similar;
}

vector<pair<Song*, float>> Graph::RWR(const string& genre, const int k) {
    constexpr int maxIter = 100;

    const auto& similarityMatrix = adjacencyMatrices[genre];
    const size_t n = similarityMatrix.size();

    mt19937 gen(random_device{}());
    uniform_int_distribution<size_t> distr(0, n - 1);
    const size_t startIndex = distr(gen);

    vector<float> prob(n, 0.0f), prevProb(n, 0.0f);
    prob[startIndex] = 1.0f;

    vector<vector<float>> transMatrix(n, vector<float>(n, 0.0f));
    for (int i = 0; i < n; ++i) {
        const float rowSum = accumulate(similarityMatrix[i].begin(), similarityMatrix[i].end(), 0.0f,
            [](const float acc, const float val) { return val != 1e9f ? acc + val : acc; });

        if (rowSum > 0) {
            for (int j = 0; j < n; ++j)
                if (similarityMatrix[i][j] != 1e9f)
                    transMatrix[i][j] = similarityMatrix[i][j] / rowSum;
        } else {
            transMatrix[i][i] = 1.0f;
        }
    }

    for (int iter = 0; iter < maxIter; ++iter) {
        constexpr float restartProb = 0.15f;
        prevProb = prob;
        vector<float> nextProb(n, 0.0f);

        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                nextProb[i] += (1.0f - restartProb) * transMatrix[j][i] * prob[j];

        nextProb[startIndex] += restartProb;

        float diff = 0.0f;
        for (int i = 0; i < n; ++i)
            diff += fabs(nextProb[i] - prevProb[i]);

        prob = nextProb;
        if (constexpr float epsilon = 1e-6f; diff < epsilon) break;
    }

    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;
    for (int i = 0; i < n; ++i) {
        if (i == startIndex) continue;
        pq.emplace(prob[i], i);
        if (pq.size() > k) pq.pop();
    }

    vector<Song>& songs = genreMap[genre];
    unordered_set<string> seenSongs;
    vector<pair<Song*, float>> topK;

    while (!pq.empty()) {
        auto [score, idx] = pq.top();
        pq.pop();

        Song* song = &songs[idx];
        if (string key = song->track_name + "|" + song->artists; seenSongs.insert(key).second) {
            topK.emplace_back(song, score);
        }
    }

    Song* startSong = &songs[startIndex];
    if (const string startKey = startSong->track_name + "|" + startSong->artists; seenSongs.insert(startKey).second) {
        topK.emplace_back(startSong, 0.0f);
    }

    return topK;
}
