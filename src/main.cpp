#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

struct Circle{
	sf::CircleShape circle;
	const float radius = 20;
	const int points = 100;
	const float restitution = 1;
	float x;
	float y;
	float vx;
	float vy;
	float ax;
	float ay;

	void update(float dt, const float containerX, const float containerY, const float containerWidth, const float containerHeight){
		x = x + vx * dt; 
		y = y + vy * dt;
		vx = vx + ax * dt;
		vy = vy + ay * dt;

		if(x + radius > containerX + containerWidth) {vx = -std::abs(vx) * restitution; x = containerX + containerWidth - radius;} // right edge
		if(y + radius > containerY + containerHeight) {vy = -std::abs(vy) * restitution; y = containerY + containerHeight - radius;} // bottom edge
		if(x - radius < containerX) {vx = std::abs(vx) * restitution; x = containerX + radius;} // left edge
		if(y - radius < containerY) {vy = std::abs(vy) * restitution; y = containerY + radius;} // top edge
	}
	void draw(sf::RenderWindow& window){
		circle.setPosition({x, y});
		window.draw(circle);
	}
	Circle(float x, float y, float vx, float vy, float ax, float ay){
		this->x = x;
		this->y = y;
		this->vx = vx;
		this->vy = vy;
		this->ax = ax;
		this->ay = ay;
		circle.setRadius(radius);
		circle.setOrigin({radius, radius}); // make the center (0, 0)
		circle.setPointCount(points);
		circle.setFillColor(sf::Color::White);
	}
};



int main()
{
	const int SCREEN_WIDTH = 1920;
	const int SCREEN_HEIGHT = 1080;
	const sf::Color BACKGROUND_COLOR = sf::Color::Black;
	const int framerate = 60;
	sf::RenderWindow window( sf::VideoMode( { SCREEN_WIDTH, SCREEN_HEIGHT } ), "Physics Engine" );
	window.setFramerateLimit(framerate);
	sf::Clock clock;
	float dt;
	
	// box
	const float boxX = 760;
	const float boxY = 340;
	const float boxWidth = 400;
	const float boxHeight = 400;

	sf::VertexArray lines(sf::PrimitiveType::LineStrip, 5);
	lines[0].position = sf::Vector2f(boxX, boxY);
	lines[1].position = sf::Vector2f(boxX + boxWidth, boxY);
	lines[2].position = sf::Vector2f(boxX + boxWidth, boxY + boxHeight);
	lines[3].position = sf::Vector2f(boxX, boxY + boxHeight);
	lines[4].position = sf::Vector2f(boxX, boxY);


	// random
	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(-100, 100);

	// circle
	std::vector<Circle> circles;
	for(int i = 0; i < 5; i++){
		Circle circle(900 + distrib(gen), 500 + distrib(gen), distrib(gen) * 10, distrib(gen) * 10, 0, 0);
		circles.push_back(circle);
	}


	//Circle circle1(900, 500, 500, 1000, 0, 0);

	

	
	

	while ( window.isOpen() )
	{
		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}
		

		dt = clock.restart().asSeconds();

		window.clear(BACKGROUND_COLOR);


		for(Circle& circle : circles){
			circle.update(dt, boxX, boxY, boxWidth, boxHeight);
			circle.draw(window);
		}

		window.draw(lines);

		window.display();
	}
}
