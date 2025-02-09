#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <iterator>
#include <map>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <cstdio> 
#include <map>
#include <array>
#include "ysglfontdata.h"
#include "yspng.h"
#include "fssimplewindow.h"

void RenderText(int x, int y, const std::string& text, const unsigned char* const font[], int width, int height, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2i(x, y);
    YsGlDrawFontBitmapDirect(text.c_str(), font, width, height);
}

// Balatro =======================================================================================
class Card
{
public:
    int suit;
    int rank;
    bool selected = false;

    Card(int s, int r) : suit(s), rank(r) {}
};

class Deck
{
public:
    std::vector<Card> cards;

    Deck()
    {
        int suits[] = { 0,1,2,3 };// { "Hearts", "Diamonds", "Clubs", "Spades" };
        int ranks[] = { 2,3,4,5,6,7,8,9,10,11,12,13,14 };
        for (const auto& suit : suits)
        {
            for (const auto& rank : ranks)
            {
                cards.emplace_back(suit, rank);
            }
        }
    }

    void shuffle();
};

// Randomize cards in deck
void Deck::shuffle()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
}

class Hand
{
public:
    int maxCardCount = 8;
    std::vector <Card> cards;
};

// Take empty hand spaces and replace with top of deck
void fillHand(Deck& deck, Hand& hand)
{
    // if not enough cards in deck
    int numToAdd = hand.maxCardCount - hand.cards.size();
    if (deck.cards.size() < numToAdd)
    {
        numToAdd = deck.cards.size();
    }
    // Hand has empty spaces, fill with deck, then remove cards from deck
    if (hand.cards.size() < hand.maxCardCount)
    {
        std::vector <Card> cardsToInject = { deck.cards.begin(), deck.cards.begin() + numToAdd }; // Cards from top of deck
        deck.cards.erase(deck.cards.begin(), deck.cards.begin() + numToAdd - 1); // remove from deck
        hand.cards.insert(hand.cards.end(), cardsToInject.begin(), cardsToInject.end()); // add them to hand
    }
    return;
}

// Sort card vector by rank order
void sortByRank(std::vector <Card>& cards)
{
    auto comp = [](Card& lhs, Card& rhs) {return lhs.rank > rhs.rank; };
    std::sort(cards.begin(), cards.end(), comp);
}

// Sort card vector by suit order
void sortBySuit(std::vector <Card>& cards)
{
    auto comp = [](Card& lhs, Card& rhs) {return lhs.suit > rhs.suit; };
    std::sort(cards.begin(), cards.end(), comp);
}

// Return poker hand type inputted
std::string handType(std::vector <Card> play)
{
    sortByRank(play);
    std::map<int, int> suitCount;
    std::map<int, int> rankCount;
    for (const auto card : play)
    {
        suitCount[card.suit]++;
        rankCount[card.rank]++;
    }

    bool isFlush = (suitCount.size() == 1 && play.size() == 5);
    std::vector<int> ranks;
    for (const auto& pair : rankCount)
    {
        ranks.push_back(pair.first);
    }
    std::sort(ranks.begin(), ranks.end());

    bool isStraight = (ranks.size() == 5 && ranks.back() - ranks.front() == 4);

    if (isFlush && isStraight) return "Straight Flush";


    int n = 0; // FIND PAIRS
    for (const auto& pair : rankCount)
    {
        if (pair.second == 2) { n = n + 1; }
        else if (pair.second == 4) { return "Four of a kind"; }
        else if (pair.second == 3) { return "Three of a kind"; }
    }

    if (isFlush) { return "Flush"; }
    if (isStraight) { return "Straight"; }
    else if (n == 2) { return "Two Pair"; }
    else if (n == 1) { return "One Pair"; }

    return "High Card";
}

class Joker
{
private:
    int cost;
    char trigger[256];
    char effect[256];
public:
    Joker(int c, const char t[], const char e[]) : cost(c) {
        strcpy_s(trigger, t);
        strcpy_s(effect, e);
    }
};

class JokerHand
{
private:
    int maxJokerCount;
public:
    std::vector <Joker> jokers;
};

// independent, inhand, inplay, indeck, handsremain, discardsremain
Joker joker(2, "independent", "+4 mult");
Joker greedyJoker(5, "heart inplay", "+3 mult");
Joker jollyJoker(5, "handtype pair", "+8 mult");
Joker slyJoker(3, "handtype pair", "+50 chips");

void readCards(std::vector <Card> cards)
{
    for (Card card : cards)
    {
        std::string suit;
        switch (card.suit)
        {
        case 0:
            suit = "S";
            break;
        case 1:
            suit = "C";
            break;
        case 2:
            suit = "H";
            break;
        case 3:
            suit = "D";
            break;
        }
        std::cout << card.rank << suit << " ";
    }
}

void updateScore(std::string hand, int& chips, int& mult)
{
    if (hand == "High Card")
    {
        chips = chips + 5;
        mult = mult + 1;
    }
    else if (hand == "One Pair")
    {
        chips = chips + 10;
        mult = mult + 2;
    }
    else if (hand == "Two Pair")
    {
        chips = chips + 20;
        mult = mult + 2;
    }
    else if (hand == "Straight")
    {
        chips = chips + 30;
        mult = mult + 4;
    }
    else if (hand == "Flush")
    {
        chips = chips + 35;
        mult = mult + 4;
    }
    else if (hand == "Four of a kind")
    {
        chips = chips + 60;
        mult = mult + 7;
    }
    else if (hand == "Three of a kind")
    {
        chips = chips + 30;
        mult = mult + 3;
    }

}

void DrawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_TRIANGLE_FAN);
    for (int ii = 0; ii < num_segments; ii++) {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
        float x = r * cosf(theta);//calculate the x component 
        float y = r * sinf(theta);//calculate the y component 
        glVertex2f(x + cx, y + cy);//output vertex 
    }
    glEnd();
}

void drawCard(int handCount, int index, int rank, std::string suit, bool selected, int width, int height)
{
    int padding = 50;
    int cardWidth = 45;
    int cardHeight = cardWidth * 1.5;
    int startingX = (width / handCount) * (index)+cardWidth + 5;
    int startingY = height * (3 / 2);
    if (selected)
    {
        startingY = startingY - 60;
    }

    glLineWidth(2);
    glColor3f(1, 1, 1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(startingX - cardWidth, startingY - cardHeight);
    glVertex2i(startingX + cardWidth, startingY - cardHeight);
    glVertex2i(startingX + cardWidth, startingY + cardHeight);
    glVertex2i(startingX - cardWidth, startingY + cardHeight);
    glEnd();
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(startingX - cardWidth, startingY - cardHeight);
    glVertex2i(startingX + cardWidth, startingY - cardHeight);
    glVertex2i(startingX + cardWidth, startingY + cardHeight);
    glVertex2i(startingX - cardWidth, startingY + cardHeight);
    glEnd();

    RenderText(startingX - 20, startingY - 34, std::to_string(rank) + suit, YsFont16x20, 16, 20, 0.0f, 0.0f, 0.0f);

    startingY = startingY + 10;
    if (suit == "H")
    {
        glLineWidth(20);
        glColor3f(1, 0, 0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(startingX - cardWidth / 5 - 8, startingY - cardHeight / 3);
        glVertex2i(startingX, startingY);
        glVertex2i(startingX + cardWidth / 5 + 8, startingY - cardHeight / 3);
        glEnd();

        DrawCircle(startingX - cardWidth / 5 + 2, startingY - cardHeight / 3, 9, 10);
        DrawCircle(startingX + cardWidth / 5 - 2, startingY - cardHeight / 3, 9, 10);
    }
    else if (suit == "D")
    {
        glLineWidth(4);
        glColor3f(1, 0, 0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(startingX - cardWidth / 4, startingY - cardHeight / 4);
        glVertex2i(startingX, startingY);
        glVertex2i(startingX + cardWidth / 4, startingY - cardHeight / 4);
        glVertex2i(startingX, startingY - cardHeight / 2);
        glEnd();
    }
    else if (suit == "C")
    {
        startingY = startingY - 10;
        glLineWidth(2);
        glColor3f(0, 0, 0);
        DrawCircle(startingX - cardWidth / 4, startingY, 6, 10);
        DrawCircle(startingX + cardWidth / 4, startingY, 6, 10);
        DrawCircle(startingX, startingY - cardHeight / 4, 6, 10);
    }
    else if (suit == "S")
    {
        startingY = startingY - 10;
        glLineWidth(20);
        glColor3f(0, 0, 0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(startingX - cardWidth / 5 - 8, startingY);
        glVertex2i(startingX, startingY - cardHeight / 3);
        glVertex2i(startingX + cardWidth / 5 + 8, startingY);
        glEnd();

        DrawCircle(startingX - cardWidth / 5 + 2, startingY, 9, 10);
        DrawCircle(startingX + cardWidth / 5 - 2, startingY, 9, 10);
    }

}

void drawHand(std::string hand, int& handIter, int wWid, int wHei)
{
    if (handIter > 0)
    {
        RenderText(wWid / 2, wHei / 2, hand + "!", YsFont16x20, 16, 20, 1.0f, 1.0f, 1.0f);
    }
    handIter = handIter - 1;
}

// joker hand, Order: base, at hand, at type, at end
// base modifiers, hands, discards, money, cards in hand, cards in play, cards in deck, jokers in hand
int main()
{
    int discards = 5;
    int hands = 5;

    Deck redDeck; // Initialize deck
    redDeck.shuffle();

    Hand playerHand;
    fillHand(redDeck, playerHand);
    sortBySuit(playerHand.cards);
    readCards(playerHand.cards);

    std::cout << std::endl;

    std::vector<Card> handPlay = { playerHand.cards.begin(), playerHand.cards.end() - 3 };
    std::string  hand = handType(handPlay);
    //std::cout << hand << " ";

    std::vector<int> playIdx;
    handPlay.clear();
    // select cards -> play selected, [jokers], add up points
    // select cards -> discard selected, fill hand
    int score = 0;
    int chips = 0;
    int mult = 0;
    int rounds = 0;
    int lb, mb, rb, mx, my;
    int wWid = 800;
    int wHei = 500;
    int handIter = 0;
    FsOpenWindow(0, 0, 800, 600, 1, "Balatro");
    FsResizeWindow(wWid, 500);
    for (;;)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glColor3ub(53, 153, 66);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(0, 0);
        glVertex2i(0, wHei + 100);
        glVertex2i(wWid, wHei + 100);
        glVertex2i(wWid, 0);
        glEnd();

        glColor3ub(76, 181, 77);
        DrawCircle(wWid / 2, wHei + 300, wWid / 2 + 50, 30);

        FsPollDevice();
        int key = FsInkey();

        if (FSKEY_ESC == key) // Quit game
        {
            break;
        }

        // Mouse click +++
        FsGetMouseState(lb, mb, rb, mx, my);

        for (int i = 0; i < playerHand.cards.size(); ++i) // loop through hand, draw each card
        {
            std::string suit;
            switch (playerHand.cards[i].suit)
            {
            case 0:
                suit = "S";
                break;
            case 1:
                suit = "C";
                break;
            case 2:
                suit = "H";
                break;
            case 3:
                suit = "D";
                break;
            }

            drawCard(playerHand.maxCardCount, i, playerHand.cards[i].rank, suit, playerHand.cards[i].selected, wWid, wHei);
        }

        glColor3ub(30, 30, 30);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(10, wHei / 3 - 50);
        glVertex2i(230, wHei / 3 - 50);
        glVertex2i(230, wHei / 3 + 160);
        glVertex2i(10, wHei / 3 + 160);

        glEnd();
        RenderText(20, wHei / 3 - 20, "Score:    " + std::to_string(score), YsFont16x20, 16, 20, 1.0f, 1.0f, 1.0f);
        RenderText(20, wHei / 3 + 75, "Hands:    " + std::to_string(hands), YsFont16x20, 16, 20, 0.106f, 0.573f, 0.89f);
        RenderText(20, wHei / 3 + 100, "Discards: " + std::to_string(discards), YsFont16x20, 16, 20, 0.89f, 0.271f, 0.106f);
        RenderText(20, wHei / 3 + 150, "Round " + std::to_string(rounds + 1) + "/3", YsFont16x20, 16, 20, 0.969f, 0.831f, 0.106f);

        if (key <= FSKEY_8 && key >= FSKEY_1) // Select cards to play
        {
            key = key - FSKEY_1;
            playerHand.cards[key].selected = !playerHand.cards[key].selected; // toggle selected
            // create vector of selected
            playIdx.clear();
            std::cout << "\n";
            for (int i = 0; i < playerHand.cards.size(); ++i)
            {
                if (playerHand.cards[i].selected)
                {
                    playIdx.insert(playIdx.begin(), i);
                    std::string suit;
                    switch (playerHand.cards[i].suit)
                    {
                    case 0:
                        suit = "S";
                        break;
                    case 1:
                        suit = "C";
                        break;
                    case 2:
                        suit = "H";
                        break;
                    case 3:
                        suit = "D";
                        break;
                    }
                    std::cout << playerHand.cards[i].rank << suit << " | ";
                }
            }
            std::cout << " \n";
            handPlay.clear();
            for (int id : playIdx)
            {
                handPlay.insert(handPlay.begin(), playerHand.cards[id]);
            }
        }


        if (FSKEY_ENTER == key) // Play selected hand
        {
            if (hands > 0 && handPlay.size() > 0)
            {
                hand = handType(handPlay); // get poker type

                // score points
                handIter = 75;

                std::cout << "Hand Played: " << hand << "\n";
                int idx;
                for (int i = 0; i <= playIdx.size() - 1; i++)
                {
                    // add ranks to chips, add poker type chips + multiplier
                    // remove cards in hand, fill from deck
                    idx = playIdx[i];
                    playerHand.cards.erase(playerHand.cards.begin() + idx);
                }
                fillHand(redDeck, playerHand);
                hands = hands - 1;
                readCards(playerHand.cards);

                updateScore(hand, chips, mult);
                score = score + (chips * mult); // update score
                std::cout << "\nCurrent Score: " << score;
                std::cout << "\nHands Left: " << hands;
                std::cout << "\nDiscards Left: " << discards;

                if (rounds == 0 && score >= 40) // Round 1
                {
                    rounds = rounds + 1;
                    // Reset deck and hand, new goal
                    score = 0;
                    hands = 5;
                    discards = 5;
                    redDeck = Deck();
                    redDeck.shuffle();
                    Hand playerHand;
                    fillHand(redDeck, playerHand);
                    std::cout << "\n\nRound Won!\nNew score to beat: 350" << "\n";
                }
                else if (rounds == 1 && score >= 350) // Round 2
                {
                    rounds = rounds + 1;
                    // Reset deck and hand, new goal
                    score = 0;
                    hands = 5;
                    discards = 5;
                    redDeck = Deck();
                    redDeck.shuffle();
                    Hand playerHand;
                    fillHand(redDeck, playerHand);
                    std::cout << "\n\nRound Won!\nNew score to beat: 400" << "\n";
                }
                else if (rounds == 2 && score >= 400) // Round 3 - Final round
                {
                    std::cout << "\n\n\nGame Won!\n\n\n";
                    break;
                }

                if (hands <= 0) // No more hands left...
                {
                    std::cout << "\n\n\nGame Over\n\n\n";
                    break;
                }

                chips = 0;
                mult = 0;
            }
        }
        else if (FSKEY_ALT == key) // discard selected cards
        {
            if (discards > 0 && handPlay.size() > 0)
            {
                int idx;
                for (int i = 0; i <= playIdx.size() - 1; i++)
                {
                    // add ranks to chips, add poker type chips + multiplier
                    // remove cards in hand, fill from deck
                    idx = playIdx[i];
                    playerHand.cards.erase(playerHand.cards.begin() + idx);
                }
                fillHand(redDeck, playerHand);
                readCards(playerHand.cards);
                discards = discards - 1;
            }
        }
        else if (FSKEY_S == key) // Sort hand by suit
        {
            std::cout << "\n\nSorting by Suit... \n";
            sortBySuit(playerHand.cards);
            readCards(playerHand.cards);
        }
        else if (FSKEY_R == key) // Sort hand by rank
        {
            std::cout << "\n\nSorting by Rank... \n";
            sortByRank(playerHand.cards);
            readCards(playerHand.cards);
        }
        drawHand(hand, handIter, wWid, wHei);
        FsSwapBuffers();
        FsSleep(25);
    }
    return 0;
}