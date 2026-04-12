#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include "heap.h"
#include "dijkstra.h"


using namespace std;

const int NUM_NODES = 15;

const string NODE_NAMES[NUM_NODES] = {
    "Aeroporto",      
    "Rodoviaria",     
    "Centro Norte",   
    "Zona Leste",     
    "Bairro Oeste",   
    "Centro",         
    "Zona Industrial",
    "Universidade",   
    "Porto",          
    "Mercado",        
    "Shopping",       
    "Estadio",        
    "Hospital",       
    "Estacao Sul",    
    "Parque",         
};

const sf::Vector2f NODE_POS[NUM_NODES] = {
    { 100,  80},  
    { 300,  60},  
    { 520,  80},  
    { 700, 160},  
    {  80, 260},  
    { 320, 240},  
    { 600, 270},  
    { 160, 380},  
    { 420, 400},  
    { 640, 420},  
    { 200, 520},  
    { 400, 540},  
    { 620, 550},  
    { 300, 650},  
    { 520, 660},  
};

const vector<Edge> EDGES = {
    { 0,  1,  6}, 
    { 0,  4, 12}, 
    { 0,  5, 20}, 
    { 1,  2,  7},  
    { 1,  5,  9},  
    { 2,  3,  5},  
    { 2,  5,  6}, 
    { 2,  6, 11},  
    { 3,  6,  8},  
    { 3,  9, 15},  
    { 4,  5,  7},  
    { 4,  7, 10},  
    { 5,  2,  6},  
    { 5,  6,  9},  
    { 5,  8,  8}, 
    { 5, 11, 13}, 
    { 6,  9,  7},  
    { 6, 12, 10},  
    { 7,  8,  6},  
    { 7, 10,  5},  
    { 7, 13, 14},  
    { 8,  9,  4},  
    { 8, 11,  9},  
    { 9, 12,  6},  
    { 9, 14,  8},  
    {10, 11,  7},  
    {10, 13,  5},  
    {11, 13,  6},  
    {11, 14,  9},  
    {12, 14,  7},  
    {13, 14,  4},  
    {14, 0,  25},
};

bool isEdgeInPath(const vector<int>& path, int from, int to) {
    for (int i = 0; i + 1 < (int)path.size(); i++)
        if (path[i] == from && path[i+1] == to)
            return true;
    return false;
}

bool isNodeInPath(const vector<int>& path, int id) {
    return find(path.begin(), path.end(), id) != path.end();
}

sf::Vector2f midpoint(sf::Vector2f a, sf::Vector2f b, float offset = 0.f) {
    sf::Vector2f mid = (a + b) / 2.f;
    sf::Vector2f dir = b - a;
    float len = sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len > 0) dir /= len;
    return {mid.x - dir.y * offset, mid.y + dir.x * offset};
}

void drawRoad(sf::RenderWindow& window, sf::Vector2f pa, sf::Vector2f pb, float thickness, sf::Color color) {
    sf::Vector2f dir = pb - pa;
    float len = sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len == 0) return;
    sf::Vector2f norm = {-dir.y / len, dir.x / len};
    float hw = thickness / 2.f;

    sf::VertexArray road(sf::Quads, 4);
    road[0].position = pa + norm * hw;
    road[1].position = pa - norm * hw;
    road[2].position = pb - norm * hw;
    road[3].position = pb + norm * hw;
    for (int k = 0; k < 4; k++) road[k].color = color;
    window.draw(road);
}

void drawArrow(sf::RenderWindow& window, sf::Vector2f pa, sf::Vector2f pb, sf::Color color, float nodeRadius = 16.f) {
    sf::Vector2f dir = pb - pa;
    float len = sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len == 0) return;
    sf::Vector2f unit = dir / len;

    sf::Vector2f tip = pb - unit * nodeRadius;

    float arrowLen  = 12.f;
    float arrowWide = 5.f;
    sf::Vector2f perp = {-unit.y, unit.x};

    sf::Vector2f base = tip - unit * arrowLen;
    sf::Vector2f left  = base + perp * arrowWide;
    sf::Vector2f right = base - perp * arrowWide;

    sf::VertexArray triangle(sf::Triangles, 3);
    triangle[0].position = tip;
    triangle[1].position = left;
    triangle[2].position = right;
    for (int k = 0; k < 3; k++) triangle[k].color = color;
    window.draw(triangle);
}


sf::View getLetterboxView(sf::View view, int winW, int winH) {
    float windowRatio = (float)winW / (float)winH;
    float viewRatio   = view.getSize().x / view.getSize().y;
    float sizeX = 1.f, sizeY = 1.f;
    float posX  = 0.f, posY  = 0.f;

    if (windowRatio >= viewRatio) {
        sizeX = viewRatio / windowRatio;
        posX  = (1.f - sizeX) / 2.f;
    } else {
        sizeY = windowRatio / viewRatio;
        posY  = (1.f - sizeY) / 2.f;
    }
    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
    return view;
}

int main(){
    const float LOGICAL_W = 820.f;
    const float LOGICAL_H = 740.f;

    sf::RenderWindow window(sf::VideoMode(820, 740), "Patos de Minas - Dijkstra",
                            sf::Style::Default | sf::Style::Resize);
    window.setFramerateLimit(60);

    sf::View gameView(sf::FloatRect(0.f, 0.f, LOGICAL_W, LOGICAL_H));
    gameView = getLetterboxView(gameView, 820, 740);

    sf::View uiView(sf::FloatRect(0.f, 0.f, 820.f, 740.f));

    sf::Font font;
    bool hasFont = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");

    int startNode = -1;
    int endNode   = -1;
    DijkstraResult result;

    int  animStep  = 0;
    bool animDone  = false;
    sf::Clock animClock;
    float timePerStep = 0.45f;
    int clickPhase = 0;

    while(window.isOpen()){

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::Resized) {
                gameView = sf::View(sf::FloatRect(0.f, 0.f, LOGICAL_W, LOGICAL_H));
                gameView = getLetterboxView(gameView, event.size.width, event.size.height);
                uiView   = sf::View(sf::FloatRect(0.f, 0.f,
                                    (float)event.size.width, (float)event.size.height));
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mpos = window.mapPixelToCoords(
                    {event.mouseButton.x, event.mouseButton.y}, gameView);
                for (int i = 0; i < NUM_NODES; i++) {
                    sf::Vector2f delta = mpos - NODE_POS[i];
                    float d = sqrt(delta.x*delta.x + delta.y*delta.y);
                    if (d < 22.f) {
                        if (clickPhase == 0) {
                            startNode  = i;
                            clickPhase = 1;
                        } else {
                            endNode    = i;
                            clickPhase = 0;
                            result     = runDijkstra(startNode, endNode, NUM_NODES, EDGES);
                            animStep   = 0;
                            animDone   = false;
                            animClock.restart();
                        }
                        break;
                    }
                }
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                startNode  = -1;
                endNode    = -1;
                result     = DijkstraResult();
                animStep   = 0;
                animDone   = false;
                clickPhase = 0;
                animClock.restart();
            }
        }


        if (!animDone && !result.path.empty()) {
            if (animClock.getElapsedTime().asSeconds() >= timePerStep) {
                if (animStep < (int)result.path.size() - 1) {
                    animStep++;
                    animClock.restart();
                } else {
                    animDone = true;
                }
            }
        }

        window.clear(sf::Color::Black);
        window.setView(gameView);

        sf::RectangleShape bg(sf::Vector2f(LOGICAL_W, LOGICAL_H));
        bg.setFillColor(sf::Color(0, 0, 0));
        window.draw(bg);

        for (auto& e : EDGES) {
            sf::Vector2f pa = NODE_POS[e.from];
            sf::Vector2f pb = NODE_POS[e.to];
            bool inPath = isEdgeInPath(result.path, e.from, e.to);

            sf::Color roadCol = inPath ? sf::Color(255, 210,   0) : sf::Color( 70,  75,  88);
            sf::Color arrowCol = inPath ? sf::Color(255, 230,  80): sf::Color(110, 115, 135);

            drawRoad(window, pa, pb, inPath ? 6.f : 3.f, roadCol);
            drawArrow(window, pa, pb, arrowCol);

            if (hasFont) {
                sf::Text wt;
                wt.setFont(font);
                wt.setString(to_string(e.weight) + "m");
                wt.setCharacterSize(10);
                wt.setFillColor(inPath ? sf::Color(255, 230, 80): sf::Color(120, 125, 145));
                sf::FloatRect b = wt.getLocalBounds();
                wt.setOrigin(b.width / 2.f, b.height / 2.f);
                wt.setPosition(midpoint(pa, pb, 12.f));
                window.draw(wt);
            }
        }

        for (int i = 0; i < NUM_NODES; i++) {
            sf::Vector2f pos = NODE_POS[i];
            bool isStart = (startNode >= 0 && i == startNode);
            bool isEnd   = (endNode   >= 0 && i == endNode);
            bool animated = false;
            for (int s = 0; s <= animStep && s < (int)result.path.size(); s++)
                if (result.path[s] == i) { animated = true; break; }

            sf::CircleShape halo(22.f);
            halo.setOrigin(22.f, 22.f);
            halo.setPosition(pos);
            if      (isStart)  halo.setFillColor(sf::Color(50, 120, 255, 70));
            else if (isEnd)    halo.setFillColor(sf::Color(255, 100, 30, 70));
            else if (animated) halo.setFillColor(sf::Color(255, 200,  0, 55));
            else               halo.setFillColor(sf::Color(0, 0, 0, 0));
            window.draw(halo);

            sf::CircleShape node(14.f);
            node.setOrigin(14.f, 14.f);
            node.setPosition(pos);
            if      (isStart)  node.setFillColor(sf::Color( 50, 130, 255));
            else if (isEnd)    node.setFillColor(sf::Color(255, 100,  30));
            else if (animated) node.setFillColor(sf::Color(255, 200,   0));
            else if (isNodeInPath(result.path, i))
                               node.setFillColor(sf::Color(200, 160,   0));
            else               node.setFillColor(sf::Color( 70,  75,  90));
            node.setOutlineThickness(2.f);
            node.setOutlineColor(sf::Color(190, 195, 210));
            window.draw(node);

            if (hasFont) {
                sf::Text label;
                label.setFont(font);
                label.setString(NODE_NAMES[i]);
                label.setCharacterSize(11);
                label.setFillColor(sf::Color(215, 220, 232));
                sf::FloatRect b = label.getLocalBounds();
                label.setOrigin(b.width / 2.f, 0.f);
                label.setPosition(pos.x, pos.y + 17.f);
                window.draw(label);
            }
        }

        if (hasFont) {
            window.setView(uiView);

            float panelH = (startNode < 0 || endNode < 0) ? 108.f
                         : result.path.empty()            ? 108.f
                                                          : 118.f;
            sf::RectangleShape panel(sf::Vector2f(280.f, panelH));
            panel.setPosition(10.f, 10.f);
            panel.setFillColor(sf::Color(20, 22, 28, 220));
            panel.setOutlineThickness(1.f);
            panel.setOutlineColor(sf::Color(60, 65, 80));
            window.draw(panel);

            sf::Text info;
            info.setFont(font);
            info.setCharacterSize(12);
            info.setFillColor(sf::Color(200, 205, 215));
            info.setString("De: " + (startNode >= 0 ? NODE_NAMES[startNode] : string("---")));
            info.setPosition(16.f, 16.f);
            window.draw(info);

            info.setString("Para: " + (endNode >= 0 ? NODE_NAMES[endNode] : string("---")));
            info.setPosition(16.f, 34.f);
            window.draw(info);

            if (startNode < 0 || endNode < 0) {
            } else if (result.path.empty()) {
                info.setFillColor(sf::Color(220, 80, 60));
                info.setString("Sem caminho disponivel!");
                info.setPosition(16.f, 54.f);
                window.draw(info);
            } else {
                info.setFillColor(sf::Color(255, 210, 60));
                info.setString("Tempo: " + to_string(result.totalCost) + " min  |  " +
                               to_string((int)result.path.size()) + " nos");
                info.setPosition(16.f, 54.f);
                window.draw(info);

                string pathStr = "";
                for (int i = 0; i < (int)result.path.size(); i++) {
                    if (i > 0) pathStr += " > ";
                    pathStr += to_string(result.path[i]);
                }
                info.setCharacterSize(10);
                info.setFillColor(sf::Color(130, 135, 150));
                info.setString(pathStr);
                info.setPosition(16.f, 74.f);
                window.draw(info);
            }

            info.setCharacterSize(10);
            info.setFillColor(sf::Color(100, 105, 120));
            string phase = clickPhase == 0 ? "Clique: nova ORIGEM" : "Clique: novo DESTINO";
            info.setString(phase + "  |  Espaco: resetar");
            info.setPosition(16.f, 92.f);
            window.draw(info);
        }

        window.display();
    }
}