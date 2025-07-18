#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

const int NUM_PLAYERS = 4;
const int NUM_ROUNDS = 3;

map<string, vector<string>> CARD_DECK = {
    {"Hearts",   {"2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace"}},
    {"Spades",   {"2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace"}},
    {"Clubs",    {"2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace"}},
    {"Diamonds", {"2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace"}}
};

map<string, int> SUIT_PRIORITY = {
    {"Diamonds", 4},
    {"Hearts",   3},
    {"Spades",   2},
    {"Clubs",    1}
};

set<string> DRAWN_CARDS;
vector<int> SCOREBOARD(NUM_PLAYERS, 0);
const string PIPES[NUM_PLAYERS] = {
    "/tmp/player1_pipe",
    "/tmp/player2_pipe",
    "/tmp/player3_pipe",
    "/tmp/player4_pipe"
};

pair<string, string> draw_card() {
    string suit, card_value, card_key;
    do {
        int suit_index = rand() % 4;
        switch (suit_index) {
            case 0: suit = "Hearts"; break;
            case 1: suit = "Spades"; break;
            case 2: suit = "Clubs"; break;
            case 3: suit = "Diamonds"; break;
        }
        card_value = CARD_DECK[suit][rand() % CARD_DECK[suit].size()];
        card_key = suit + "-" + card_value;
    } while (DRAWN_CARDS.find(card_key) != DRAWN_CARDS.end());
    DRAWN_CARDS.insert(card_key);
    return {suit, card_value};
}

int compare_cards(pair<string, string> card1, pair<string, string> card2) {
    if (SUIT_PRIORITY[card1.first] > SUIT_PRIORITY[card2.first]) return 1;
    if (SUIT_PRIORITY[card1.first] < SUIT_PRIORITY[card2.first]) return 2;
    vector<string> ORDER = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace"};
    int val1 = find(ORDER.begin(), ORDER.end(), card1.second) - ORDER.begin();
    int val2 = find(ORDER.begin(), ORDER.end(), card2.second) - ORDER.begin();
    if (val1 > val2) return 1;
    if (val1 < val2) return 2;
    return 0;
}

void setup_pipes() {
    for (auto& pipe : PIPES)
        mkfifo(pipe.c_str(), 0666);
}

void cleanup_pipes() {
    for (auto& pipe : PIPES)
        unlink(pipe.c_str());
}

void play_game_ipc() {
    setup_pipes();

    for (int round = 1; round <= NUM_ROUNDS; ++round) {
        cout << "\n======================================\n";
        cout << "|              ROUND " << round << "               |\n";
        cout << "======================================\n";

        vector<pair<string, string>> player_cards(NUM_PLAYERS);
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            char buffer[100];
            int fd_read = open(PIPES[i].c_str(), O_RDONLY);
            read(fd_read, buffer, sizeof(buffer));
            close(fd_read);

            string message(buffer);
            if (message == "READY") {
                auto card = draw_card();
                player_cards[i] = card;
                string card_message = card.first + "-" + card.second;
                int fd_write = open(PIPES[i].c_str(), O_WRONLY);
                write(fd_write, card_message.c_str(), card_message.size() + 1);
                close(fd_write);
            }
        }

        cout << "\nResults:\n";
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            cout << "Player " << (i + 1) << " drew: " << player_cards[i].first << " of " << player_cards[i].second << endl;
        }

        int winner_idx = 0;
        for (int i = 1; i < NUM_PLAYERS; ++i) {
            if (compare_cards(player_cards[winner_idx], player_cards[i]) == 2)
                winner_idx = i;
        }
        auto winner_card = player_cards[winner_idx];
        SCOREBOARD[winner_idx]++;
        cout << "\nWinner of Round " << round << ": Player " << (winner_idx + 1)
             << " with " << winner_card.first << " of " << winner_card.second << "\n";
    }

    cout << "\n======================================\n";
    cout << "|             FINAL SCORES           |\n";
    cout << "======================================\n";
    int max_score = *max_element(SCOREBOARD.begin(), SCOREBOARD.end());
    vector<int> winners;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        cout << "Player " << (i + 1) << ": " << SCOREBOARD[i] << " win(s)\n";
        if (SCOREBOARD[i] == max_score)
            winners.push_back(i + 1);
    }

    if (winners.size() == 1)
        cout << "\nOverall Winner: Player " << winners[0] << "!\n";
    else {
        cout << "\nIt's a tie between players: ";
        for (auto w : winners)
            cout << w << " ";
        cout << "!\n";
    }
    cout << "======================================\n";

    cleanup_pipes();
}

int main() {
    srand(time(0));
    cout << "\n======================================\n";
    cout << "|      Welcome to Pusoy Clash!       |\n";
    cout << "======================================\n";
    cout << "4 players. 3 rounds. Highest card wins each round! \n";
    play_game_ipc();
    cout << "\nGame Over. Thanks for playing!\n\n";
    return 0;
}
