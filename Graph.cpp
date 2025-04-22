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

vector<float> Graph::GetFeatureVector(const Song& song) {
    return {
            song.danceability,
            song.energy,
            song.speechiness,
            song.acousticness,
            static_cast<float>(song.instrumentalness),
            song.liveness,
            song.valence,
            song.tempo
    };
}

float Graph::EuclideanDistance(const vector<float>& a, const vector<float>& b, vector<bool> &metrics) {
    float sum = 0.0f;
    bool no_selection = true;
    for(bool b : metrics) {
        if(b) no_selection = false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (metrics[i] or no_selection) {
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

void Graph::LoadFromCSV(ifstream& file, vector<bool> &metrics) {
    string temp;
    getline(file, temp);

    while (file.is_open() && file.peek() != EOF) {
        Song song;
        getline(file, temp, ',');

        bool inQuotes = false;
        string line;
        getline(file, line);

        string current;
        vector<string> result;
        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            if (c == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                result.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        result.push_back(current);

        if (result.size() < 20) continue;

        song.track_id = result[0];
        song.artists = result[1];
        song.album = result[2];
        song.track_name = result[3];
        song.popularity = stoi(result[4]);
        song.track_length = stoi(result[5]);

        temp = result[6];
        ranges::transform(temp, temp.begin(), ::tolower);
        istringstream(temp) >> boolalpha >> song.potty_words;

        song.danceability = stof(result[7]);
        song.energy = stof(result[8]);
        song.key = stoi(result[9]);
        song.loudness = stof(result[10]);
        song.mode = stoi(result[11]);
        song.speechiness = stof(result[12]);
        song.acousticness = stof(result[13]);
        song.instrumentalness = stod(result[14]);
        song.liveness = stof(result[15]);
        song.valence = stof(result[16]);
        song.tempo = stof(result[17]);
        song.time_signature = stoi(result[18]);
        song.genre = result[19];

        string genre = song.genre;
        auto& songList = genreMap[genre];
        size_t n = songList.size();

        vector<float> newRow(n + 1, 0.0f);
        for (size_t i = 0; i < n; ++i) {
            float dist = EuclideanDistance(GetFeatureVector(song), GetFeatureVector(songList[i]), metrics);
            adjacencyMatrices[genre][i].push_back(dist);
            newRow[i] = dist;
        }

        adjacencyMatrices[genre].push_back(newRow);
        songList.push_back(song);
    }
}

void Graph::LoadGenreFromCSV(std::ifstream& file, const std::string& target_genre, vector<bool> &metrics) {
    std::string headerLine;
    std::getline(file, headerLine);

    std::string line;
    while (std::getline(file, line)) {
        size_t firstComma = line.find(',');
        if (firstComma == std::string::npos) continue;
        std::string rest = line.substr(firstComma + 1);

        std::vector<std::string> fields;
        std::string current;
        bool inQuotes = false;
        for (size_t i = 0; i < rest.size(); ++i) {
            if (char c = rest[i]; c == '"') {
                if (inQuotes && i + 1 < rest.size() && rest[i + 1] == '"') {
                    current += '"';
                    ++i;
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

        if (fields.size() < 20) continue;

        std::string genre = fields[19];
        ranges::transform(genre, genre.begin(), ::tolower);

        if (genre != target_genre) continue;

        Song song;
        song.track_id      = fields[0];
        song.artists       = fields[1];
        song.album         = fields[2];
        song.track_name    = fields[3];
        song.popularity    = std::stoi(fields[4]);
        song.track_length  = std::stoi(fields[5]);
        {
            std::string potty = fields[6];
            ranges::transform(potty, potty.begin(), ::tolower);
            std::istringstream(potty) >> std::boolalpha >> song.potty_words;
        }
        song.danceability     = std::stof(fields[7]);
        song.energy           = std::stof(fields[8]);
        song.key              = std::stoi(fields[9]);
        song.loudness         = std::stof(fields[10]);
        song.mode             = std::stoi(fields[11]);
        song.speechiness      = std::stof(fields[12]);
        song.acousticness     = std::stof(fields[13]);
        song.instrumentalness = std::stod(fields[14]);
        song.liveness         = std::stof(fields[15]);
        song.valence          = std::stof(fields[16]);
        song.tempo            = std::stof(fields[17]);
        song.time_signature   = std::stoi(fields[18]);
        song.genre            = genre;

        genreMap[target_genre].push_back(song);
    }

    auto& songs = genreMap[target_genre];
    size_t n = songs.size();

    std::vector<std::vector<float>> matrix(n, std::vector<float>(n, 0.0f));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i == j) {
                matrix[i][j] = 1e9f;
            } else {
                matrix[i][j] = EuclideanDistance(GetFeatureVector(songs[i]), GetFeatureVector(songs[j]), metrics);
            }
        }
    }

    adjacencyMatrices[target_genre] = std::move(matrix);
}

set<string> Graph::FindGenres(ifstream& file) {
    string temp;
    getline(file, temp);
    set<string> genres;

    while (file.is_open() && file.peek() != EOF) {
        getline(file, temp, ',');

        bool inQuotes = false;
        string line;
        getline(file, line);

        string current;
        vector<string> result;
        for (size_t i = 0; i < line.length(); ++i) {
            if (const char c = line[i]; c == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                result.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        result.push_back(current);

        if (result.size() < 20) continue;

        genres.insert(result[19]);
    }

    return genres;
}

vector<pair<Song*, float>> Graph::Dijsktra(string genre, int k){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(0, 999);
    int startIndex = distr(gen);
//    cout << "SONG : " << genreMap[genre][startIndex].track_name << endl;
    vector<vector<float>> similarityMatrix = adjacencyMatrices.at(genre);
    int n = similarityMatrix.size();
    vector<float> dist(n, numeric_limits<float>::infinity());
    vector<bool> visited(n, false);
    // Min heap
    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;
    dist[startIndex] = 0.0f;
    pq.push({0.0f, startIndex});

    vector<pair<int, float>> result;

    while(!pq.empty() && result.size() < k){
        auto [cost, u] = pq.top();
        pq.pop();
        if (visited[u]) continue;
        visited[u] = true;

        if (u != startIndex){
            result.emplace_back(u, cost);
        }

        for (int v = 0; v < n; ++v){
            if (!visited[v] && similarityMatrix[u][v] > 0.0f){
                float sim = similarityMatrix[u][v]; // similarity score
                float newCost = dist[u] + (1.0f / sim); // 1/sim
                if (newCost < dist[v]) {
                    dist[v] = newCost;
                    pq.push({newCost, v});
                }
            }
        }
    }
    sort(result.begin(), result.end(), [&](auto&a, auto& b){
        return similarityMatrix[startIndex][a.first] > similarityMatrix[startIndex][b.first];
    });
    vector<pair<Song*, float>> similar;
    for (int i = 0; i < result.size(); i++){
//        cout << result[i].first << endl;
//        cout << genreMap[genre][result[i].first].track_name << endl;
        similar.push_back(make_pair(&genreMap[genre][result[i].first], result[i].second));
    }
    similar.push_back(make_pair(&genreMap[genre][startIndex], 0));
    return similar;
}

vector<pair<Song*, float>> Graph::RWR(string genre, int k) {
    float restartProb = 0.15f;
    float epsilon = 1e-6f;
    int maxIter = 100;
    vector<vector<float>> similarityMatrix = adjacencyMatrices[genre];
    int n = similarityMatrix.size();

    // Random start index
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(0, n - 1);
    int startIndex = distr(gen);
//    cout << "SONG : " << genreMap[genre][startIndex].track_name << endl;
    vector<float> prob(n, 0.0f);
    vector<float> prevProb(n, 0.0f);
    prob[startIndex] = 1.0f;

    // Build transition matrix
    vector<vector<float>> transMatrix(n, vector<float>(n, 0.0f));
    for (int i = 0; i < n; ++i) {
        float rowSum = 0.0f;
        for (int j = 0; j < n; ++j) {
            if (similarityMatrix[i][j] != 1e9f) {
                rowSum += similarityMatrix[i][j];
            }
        }
        if (rowSum > 0) {
            for (int j = 0; j < n; ++j) {
                if (similarityMatrix[i][j] != 1e9f) {
                    transMatrix[i][j] = similarityMatrix[i][j] / rowSum;
                }
            }
        } else {
            transMatrix[i][i] = 1.0f; // stay in place if no out-links
        }
    }

    // RWR iterations
    for (int iter = 0; iter < maxIter; ++iter) {
        prevProb = prob;
        vector<float> nextProb(n, 0.0f);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                nextProb[i] += (1.0f - restartProb) * transMatrix[j][i] * prob[j];
            }
        }

        // Restart
        nextProb[startIndex] += restartProb;

        // Check for convergence
        float diff = 0.0f;
        for (int i = 0; i < n; ++i) {
            diff += fabs(nextProb[i] - prevProb[i]);
        }

        prob = nextProb;
        if (diff < epsilon) break;
    }

    // Min-heap to find top K songs (excluding start)
    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;
    for (int i = 0; i < n; ++i) {
        if (i == startIndex) continue;
        pq.push({prob[i], i});
        if (pq.size() > k) pq.pop();
    }

    // Extract results
    vector<Song>& songs = genreMap[genre];
    vector<pair<Song*, float>> topK;
    while (!pq.empty()) {
        auto [score, idx] = pq.top();
        pq.pop();
        topK.emplace_back(&songs[idx], score);
    }

    reverse(topK.begin(), topK.end()); // descending order
    topK.push_back(make_pair(&genreMap[genre][startIndex], 0));
    return topK;
}