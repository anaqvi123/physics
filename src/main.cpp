#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>

float vectorMagnitude(sf::Vector2f vec);
sf::Vector2f unitVector(sf::Vector2f vec);
float dotProduct(sf::Vector2f vec1, sf::Vector2f vec2);
float vectorProjection(sf::Vector2f vec1, sf::Vector2f vec2);

struct Circle{
	sf::CircleShape circle;
	float radius = 20;
	int points = 100;
	float restitution = 0.9;
	float mass;
	float x;
	float y;
	float vx;
	float vy;
	float ax;
	float ay;

	Circle(float x, float y, float vx, float vy, float ax, float ay, const float mass, const float radius){
		this->x = x;
		this->y = y;
		this->vx = vx;
		this->vy = vy;
		this->ax = ax;
		this->ay = ay;
		this->mass = mass;
		circle.setRadius(radius);
		circle.setOrigin({radius, radius}); // make the center (0, 0)
		circle.setPointCount(points);
		circle.setFillColor(sf::Color::Blue);
	}

	void update(float dt){
		x = x + vx * dt; 
		y = y + vy * dt;
		vx = vx + ax * dt;
		vy = vy + ay * dt;
		vx *= 0.999f;
		vy *= 0.999f;
	}

	void wallCollision(sf::RenderWindow& window, std::vector<std::vector<sf::Vector2f>> lines){
		for(std::vector<sf::Vector2f>& line : lines){
			sf::Vector2f lineVec = {line[1].x - line[0].x, line[1].y - line[0].y};
			sf::Vector2f relativePos = sf::Vector2f{x, y} - line[0];
			sf::Vector2f lineNormalUnitVec = unitVector({-lineVec.y, lineVec.x});
			
			float closestPoint = vectorProjection(relativePos, lineVec);
			if(closestPoint > 1){closestPoint = 1;}
			if(closestPoint < 0){closestPoint = 0;}
			sf::Vector2f closestPointPos = line[0] + closestPoint * lineVec;

			float distanceFromClosestPoint = vectorMagnitude({x - closestPointPos.x, y - closestPointPos.y});

			if(distanceFromClosestPoint < radius && dotProduct({vx, vy}, lineNormalUnitVec) < 0){ // distance < radius and moving toward line
				sf::Vector2f normalVel = dotProduct({vx, vy}, -lineNormalUnitVec) * -lineNormalUnitVec;
				sf::Vector2f sidewaysVel = sf::Vector2f{vx, vy} - normalVel;
				
				vx = sidewaysVel.x - normalVel.x * restitution;
				vy = sidewaysVel.y - normalVel.y * restitution;

				float overlap = radius - distanceFromClosestPoint;
				x += overlap * lineNormalUnitVec.x;
				y += overlap * lineNormalUnitVec.y;
			}
		}
	}
		
	void draw(sf::RenderWindow& window){
		circle.setPosition({x, y});
		window.draw(circle);
	};
	
};

struct Rope{
	Circle c1;
	Circle c2;
	int ic1;
	int ic2;
	int ωC1 = 0;
	int ωC2 = 0;
	sf::RectangleShape connector;
	sf::Vector2f c1Pos;
	sf::Vector2f c2Pos;
	float c1Radius;
	float c2Radius;
	float length;

	 Rope(sf::Vector2f c1Pos, sf::Vector2f c2Pos, float c1Radius, float c2Radius, float length)
        : c1(c1Pos.x, c1Pos.y, 0, 0, 0, 200, 5, c1Radius),
          c2(c2Pos.x, c2Pos.y, 0, 0, 0, 200, 10, c2Radius),
          c1Pos(c1Pos),
          c2Pos(c2Pos),
          c1Radius(c1Radius),
          c2Radius(c2Radius),
          length(length) {

		connector.setSize(sf::Vector2f(20, length));
		connector.setOrigin({c1Radius * 0.5f, -c1Radius / 2});
		connector.setFillColor(sf::Color::Red);
		connector.setPosition(c1Pos);
		}
		
	void addCircles(std::vector<Circle>& circles){
		ic1 = circles.size();
		circles.push_back(c1);

		ic2 = circles.size();
		circles.push_back(c2);
	}

	void correctLength(std::vector<Circle>& circles){
		sf::Vector2f vecBetweenCircles = {circles[ic2].x - circles[ic1].x, circles[ic2].y - circles[ic1].y};
		sf::Vector2f unitVec = unitVector(vecBetweenCircles);
		float distance = vectorMagnitude(vecBetweenCircles);
		if (distance == 0) return;

		float correction = (length - distance) * 0.5f;
		float totalMass = circles[ic1].mass + circles[ic2].mass;
		float weightCircle1 = circles[ic2].mass / totalMass;
		float weightCircle2 = circles[ic1].mass / totalMass;

		circles[ic1].x -= correction * weightCircle1 * unitVec.x;
		circles[ic1].y -= correction * weightCircle1 * unitVec.y;
		circles[ic2].x += correction * weightCircle2 * unitVec.x;
		circles[ic2].y += correction * weightCircle2 * unitVec.y;

		float stiffness = 0.5f;
		sf::Vector2f relVel = {circles[ic2].vx - circles[ic1].vx, circles[ic2].vy - circles[ic1].vy};
		
		sf::Vector2f velAlongRope = dotProduct(relVel, unitVec) * unitVec * stiffness;
		circles[ic1].vx += velAlongRope.x * weightCircle1;
		circles[ic1].vy += velAlongRope.y * weightCircle1;
		circles[ic2].vx -= velAlongRope.x * weightCircle2;
		circles[ic2].vy -= velAlongRope.y * weightCircle2;


		connector.setPosition({circles[ic1].x, circles[ic1].y});
		connector.setSize(sf::Vector2f(20, distance));
		connector.setRotation(sf::degrees(std::atan2(vecBetweenCircles.y, vecBetweenCircles.x) * (180 / 3.1415926535) - 90));
	}
};


void circleCollision(Circle& circle1, Circle& circle2);

int main()
{
	const int SCREEN_WIDTH = 1920;
	const int SCREEN_HEIGHT = 1080;
	const sf::Color BACKGROUND_COLOR = sf::Color::Black;
	const int framerate = 600;
	sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "Physics Engine");
	window.setFramerateLimit(framerate);
	sf::Clock clock;
	float dt;
	
	// box
	sf::ConvexShape polygon;
	sf::Vector2f point0 = {300, 400};
	sf::Vector2f point1 = {750, 50};
	sf::Vector2f point2 = {1500, 400};
	sf::Vector2f point3 = {1000, 1000};
	polygon.setPointCount(4);
	polygon.setPoint(0, point0);
	polygon.setPoint(1, point1);
	polygon.setPoint(2, point2);
	polygon.setPoint(3, point3);
	polygon.setOutlineColor(sf::Color::Red);
	polygon.setOutlineThickness(1);
	std::vector<std::vector<sf::Vector2f>> lines = {{point0, point1}, {point1, point2}, {point2, point3}, {point3, point0}};

	// random
	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib1(-100, 100);
	std::uniform_int_distribution<> distrib2(1, 10);

	std::vector<Circle> circles;

	/*
	// circle
	for(int i = 0; i < 10; i++){
		Circle circle(900 + distrib1(gen), 500 + distrib1(gen), distrib1(gen) * 5, distrib1(gen) * 5, 0, 0, distrib2(gen), 20);
		if(circle.mass < 5){circle.circle.setFillColor(sf::Color::Green);}
		circles.push_back(circle);
	}
	*/
	
	// rope
	std::vector<Rope> ropes;
	Rope rope1 = Rope({900, 500}, {950, 500}, 20, 20, 100);
	rope1.addCircles(circles);
	ropes.push_back(rope1);

	/*
	Rope rope2 = Rope({800, 300}, {850, 250}, 20, 20, 100);
	rope2.addCircles(circles);
	ropes.push_back(rope2);

	Rope rope3 = Rope({700, 400}, {750, 450}, 20, 20, 150);
	rope3.addCircles(circles);
	ropes.push_back(rope3);
	*/


	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}
		

		dt = clock.restart().asSeconds();
		window.clear(BACKGROUND_COLOR);

		window.draw(polygon);
		
		

		for(int i = 0; i < circles.size(); i++){
			circles[i].update(dt);
			circles[i].wallCollision(window, lines);
			for(int j = i + 1; j < circles.size(); j++){
				circleCollision(circles[i], circles[j]);
			}
		}

		for(int i = 0; i < 10; i++){
			for(Rope& rope : ropes){
				rope.correctLength(circles);
			}
		}

		for(Rope& rope : ropes){
			window.draw(rope.connector);
		}
		
		for(Circle& circle : circles){
			circle.draw(window);
		}
		
		window.display();
	}
}


float vectorMagnitude(sf::Vector2f vec){
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}
float distance(sf::Vector2f pos1, sf::Vector2f pos2){
	return vectorMagnitude({pos2.x - pos1.x, pos2.y - pos1.y});
}
sf::Vector2f unitVector(sf::Vector2f vec){
	if(vectorMagnitude(vec) == 0){return {1, 0};}
	return {vec.x / vectorMagnitude(vec), vec.y / vectorMagnitude(vec)};
}
float dotProduct(sf::Vector2f vec1, sf::Vector2f vec2){
	return vec1.x * vec2.x + vec1.y * vec2.y;
}
float vectorProjection(sf::Vector2f vec1, sf::Vector2f vec2){
	float magnitude = vectorMagnitude(vec2);
	return dotProduct(vec1, vec2) / (magnitude * magnitude); // * vec2;
}
void circleCollision(Circle& circle1, Circle& circle2){
	float dist = distance({circle1.x, circle1.y}, {circle2.x, circle2.y});
	if (dist <= circle1.radius + circle2.radius){ // distance <= sum of radii
		sf::Vector2f collisionNormalUnitVector = unitVector({circle2.x - circle1.x, circle2.y - circle1.y});

		sf::Vector2f circle1V = {circle1.vx, circle1.vy};
		sf::Vector2f circle2V = {circle2.vx, circle2.vy};

		float velAlongNormal = dotProduct(circle2V - circle1V, collisionNormalUnitVector);
		if(velAlongNormal < 0){ // if relative velocity moving along collision normal < 0 (moving towards each other)
			float circle1NormalVel = dotProduct(circle1V, collisionNormalUnitVector);
			sf::Vector2f circle1SidewaysVel = circle1V - circle1NormalVel * collisionNormalUnitVector;

			float circle2NormalVel = dotProduct(circle2V, collisionNormalUnitVector);
			sf::Vector2f circle2SidewaysVel = circle2V - circle2NormalVel * collisionNormalUnitVector;

			// From m1v1 + m2v2 = m1v1' + m2v2' (conservation of momentum) and
			// v1 - v2 = -(v1 - v2) (relative velocity becomes negative)
			float newCircle1NormalVel = (2 * circle2.mass * circle2NormalVel + (circle1.mass - circle2.mass) * circle1NormalVel) / (float) (circle1.mass + circle2.mass);
			float newCircle2NormalVel = (2 * circle1.mass * circle1NormalVel + (circle2.mass - circle1.mass) * circle2NormalVel) / (float) (circle1.mass + circle2.mass);
			sf::Vector2f newCircle1NormalVelVec = newCircle1NormalVel * collisionNormalUnitVector;
			sf::Vector2f newCircle2NormalVelVec = newCircle2NormalVel * collisionNormalUnitVector;

			circle1.vx = circle1SidewaysVel.x + newCircle1NormalVelVec.x * circle1.restitution;
			circle1.vy = circle1SidewaysVel.y + newCircle1NormalVelVec.y * circle1.restitution;

			circle2.vx = circle2SidewaysVel.x + newCircle2NormalVelVec.x * circle2.restitution;
			circle2.vy = circle2SidewaysVel.y + newCircle2NormalVelVec.y * circle2.restitution;
			
			// push apart by overlap
			float overlap = (circle1.radius + circle2.radius) - dist;
			circle1.x -= ((circle2.mass / double (circle1.mass + circle2.mass)) * overlap) * collisionNormalUnitVector.x; 
			circle1.y -= ((circle2.mass / double (circle1.mass + circle2.mass)) * overlap) * collisionNormalUnitVector.y;
			circle2.x += ((circle1.mass / double (circle1.mass + circle2.mass)) * overlap) * collisionNormalUnitVector.x;
			circle2.y += ((circle1.mass / double (circle1.mass + circle2.mass)) * overlap) * collisionNormalUnitVector.y;
		}
	}
}