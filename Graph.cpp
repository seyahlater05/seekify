//
// Created by sam51 on 4/19/2025.
//

#include "Graph.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>

vector<float> Graph::GetFeatureVector(const Song& song) const {
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

float Graph::EuclideanDistance(const vector<float>& a, const vector<float>& b) const {
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
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
    for (const auto& [key, _] : genreMap) {
        genres.push_back(key);
    }
    return genres;
}

void Graph::LoadFromCSV(ifstream& file) {
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
        transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
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
            float dist = EuclideanDistance(GetFeatureVector(song), GetFeatureVector(songList[i]));
            adjacencyMatrices[genre][i].push_back(dist);
            newRow[i] = dist;
        }

        adjacencyMatrices[genre].push_back(newRow);
        songList.push_back(song);
    }
}

void Graph::LoadGenreFromCSV(std::ifstream& file, const std::string& target_genre) {
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
            char c = rest[i];
            if (c == '"') {
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
        std::transform(genre.begin(), genre.end(), genre.begin(), ::tolower);

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
            std::transform(potty.begin(), potty.end(), potty.begin(), ::tolower);
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
                matrix[i][j] = EuclideanDistance(GetFeatureVector(songs[i]), GetFeatureVector(songs[j]));
            }
        }
    }

    adjacencyMatrices[target_genre] = std::move(matrix);
}
